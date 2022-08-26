/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file reactor.c
 * @date 21 Jul 2022
 * @brief File contains C implementation of Reactor
 * @see http://www.featuremine.com
 */

#define utarray_oom()                                                          \
  do {                                                                         \
    fmc_error_reset(error, FMC_ERROR_MEMORY, NULL);                            \
    goto cleanup;                                                              \
  } while (0)

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/math.h>
#include <fmc/reactor.h>
#include <fmc/time.h>
#include <stdlib.h> // calloc() free()
#include <uthash/utarray.h>
#include <uthash/utheap.h>
#include <uthash/utlist.h>

#define FMC_REACTOR_HARD_STOP 3

fmc_icd sched_item_icd = {.sz = sizeof(struct sched_item)};

fmc_icd size_t_icd = {.sz = sizeof(size_t)};

void fmc_reactor_init(struct fmc_reactor *reactor) {
  // important: initialize lists to NULL
  reactor->stop_list = NULL;
  memset(reactor, 0, sizeof(*reactor));
  fmc_array_init(&reactor->sched, &sched_item_icd);
  fmc_array_init(&reactor->queued, &size_t_icd);
  fmc_array_init(&reactor->toqueue, &size_t_icd);
  fmc_pool_init(&reactor->pool);
  fmc_error_init_none(&reactor->err);
}

void fmc_reactor_destroy(struct fmc_reactor *reactor) {
  utarray_done(&reactor->sched);
  utarray_done(&reactor->queued);
  utarray_done(&reactor->toqueue);

  struct fmc_reactor_stop_item *head = reactor->stop_list;
  struct fmc_reactor_stop_item *item;
  struct fmc_reactor_stop_item *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    DL_DELETE(head, item);
    free(item);
  }
  fmc_pool_destroy(&reactor->pool);

  for (unsigned int i = 0; reactor->ctxs && i < reactor->size; ++i) {
    fmc_reactor_ctx_del(reactor->ctxs[i]);
  }
  fmc_error_destroy(&reactor->err);
  free(reactor->ctxs);
  memset(reactor, 0, sizeof(*reactor));
}

static void utarr_del(void *elt) {
  UT_array *_elt = (UT_array *)elt;
  utarray_done(_elt);
}
static void utarr_init(void *elt) {
  UT_array *_elt = (UT_array *)elt;
  UT_icd deps;
  deps.sz = sizeof(struct fmc_reactor_ctx_dep);
  deps.dtor = NULL;
  deps.copy = NULL;
  deps.init = NULL;
  fmc_array_init(_elt, &deps);
}

struct fmc_reactor_ctx *fmc_reactor_ctx_new(struct fmc_reactor *reactor,
                                            fmc_error_t **error) {
  struct fmc_reactor_ctx *ctx =
      (struct fmc_reactor_ctx *)calloc(1, sizeof(*ctx));
  if (!ctx)
    goto cleanup;
  ctx->reactor = reactor;
  ctx->idx = reactor->size;
  fmc_icd deps;
  deps.sz = sizeof(UT_array);
  deps.dtor = utarr_del;
  deps.copy = NULL;
  deps.init = utarr_init;
  fmc_array_init(&ctx->deps, &deps);
  fmc_error_init_none(&ctx->err);
  return ctx;
cleanup:
  fmc_error_set2(error, FMC_ERROR_MEMORY);
  return NULL;
}

void fmc_reactor_ctx_take(struct fmc_reactor_ctx *ctx,
                          struct fmc_component_input *inps,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor *r = ctx->reactor;
  struct fmc_reactor_ctx **ctxstmp = (struct fmc_reactor_ctx **)realloc(
      r->ctxs, sizeof(*r->ctxs) * (r->size + 1));
  if (!ctxstmp)
    goto cleanup;
  r->ctxs = ctxstmp;
  r->ctxs[r->size] = ctx;
  ++r->size;
  return;
cleanup:
  fmc_error_set2(error, FMC_ERROR_MEMORY);
}

void fmc_reactor_ctx_del(struct fmc_reactor_ctx *ctx) {
  if (!ctx)
    return;
  struct fmc_reactor_ctx_out *phead = ctx->out_tps;
  struct fmc_reactor_ctx_out *el = NULL;
  struct fmc_reactor_ctx_out *ptmp = NULL;
  DL_FOREACH_SAFE(phead, el, ptmp) {
    DL_DELETE(phead, el);
    if (el->name)
      free(el->name);
    if (el->type)
      free(el->type);
    free(el);
  }
  ctx->out_tps = NULL;
  utarray_done(&ctx->deps);
  fmc_error_destroy(&ctx->err);
  free(ctx);
}

fmc_time64_t fmc_reactor_sched(struct fmc_reactor *reactor) {
  struct sched_item *item =
      (struct sched_item *)utarray_front(&(reactor->sched));
  return item ? item->t : fmc_time64_end();
}

bool fmc_reactor_run_once(struct fmc_reactor *reactor, fmc_time64_t now,
                          fmc_error_t **usr_error) {
  fmc_error_t *error = &reactor->err;

  if (fmc_error_has(&reactor->err))
    goto cleanup;

  // NOTE: code handling stop in a thread safe way
  int stop_prev = reactor->stop;
  reactor->stop = __atomic_load_n(&reactor->stop_signal, __ATOMIC_SEQ_CST);
  if (reactor->stop && !stop_prev) {
    struct fmc_reactor_stop_item *item = NULL;
    struct fmc_reactor_stop_item *tmp = NULL;
    DL_FOREACH_SAFE(reactor->stop_list, item, tmp) {
      struct fmc_reactor_ctx *ctx = reactor->ctxs[item->idx];
      if (!ctx->finishing && ctx->shutdown) {
        ++reactor->finishing;
        ctx->finishing = true;
        ctx->shutdown(ctx->comp, ctx);
      }
    }
  }

  bool busy = utarray_len(&reactor->toqueue) ||
              utarray_len(&(reactor->sched)) || utarray_len(&reactor->queued);
  bool stopped = reactor->stop && !reactor->finishing;
  bool hardstop = reactor->stop >= FMC_REACTOR_HARD_STOP;
  if (!busy || stopped || hardstop)
    return false;

  ut_swap(&reactor->queued, &reactor->toqueue, sizeof(reactor->queued));
  // NOTE: queue expried componenents
  do {
    struct sched_item *item =
        (struct sched_item *)utarray_front(&reactor->sched);
    if (!item || fmc_time64_greater(item->t, now))
      break;
    utheap_push(&reactor->queued, &item->idx, FMC_SIZE_T_PTR_LESS);
    utheap_pop(&reactor->sched, FMC_INT64_T_PTR_LESS);
  } while (true);

  size_t last = SIZE_MAX;
  while (!fmc_error_has(&reactor->err)) {
    size_t *item = (size_t *)utarray_front(&reactor->queued);
    if (!item)
      break;
    size_t ctxidx = *item;
    utheap_pop(&reactor->queued, FMC_SIZE_T_PTR_LESS);
    struct fmc_reactor_ctx *ctx = reactor->ctxs[ctxidx];
    if (*item != last && !fmc_error_has(&ctx->err) && ctx->exec) {
      ctx->exec(ctx->comp, ctx, now);
      if (fmc_error_has(&ctx->err)) {
        if (*usr_error) {
          fmc_error_set(usr_error,
                        "%s\nalso, failed to run component %s with error: %s",
                        fmc_error_msg(*usr_error), ctx->comp->_vt->tp_name,
                        fmc_error_msg(&ctx->err));
        } else {
          fmc_error_set(usr_error, "failed to run component %s with error: %s",
                        ctx->comp->_vt->tp_name, fmc_error_msg(&ctx->err));
        }
      }
    }
    last = ctxidx;
  };
cleanup:
  if (fmc_error_has(&reactor->err)) {
    fmc_error_set(usr_error, "failed to run reactor once with error: %s",
                  fmc_error_msg(&reactor->err));
    return false;
  }
  if (*usr_error && !reactor->stop) {
    fmc_reactor_stop(reactor);
  }
  return true;
}

void fmc_reactor_run(struct fmc_reactor *reactor, bool live,
                     fmc_error_t **error) {
  fmc_error_clear(error);
  do {
    fmc_time64_t now = live ? fmc_time64_from_nanos(fmc_cur_time_ns())
                            : fmc_reactor_sched(reactor);
    if (!fmc_reactor_run_once(reactor, now, error))
      break;
  } while (true);
}

void fmc_reactor_stop(struct fmc_reactor *reactor) {
  __atomic_fetch_add(&reactor->stop_signal, 1, __ATOMIC_SEQ_CST);
}

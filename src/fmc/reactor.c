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

UT_icd sched_item_icd = {.sz = sizeof(struct sched_item)};

UT_icd size_t_icd = {.sz = sizeof(size_t)};

void fmc_reactor_init(struct fmc_reactor *reactor) {
  // important: initialize lists to NULL
  reactor->stop_list = NULL;
  memset(reactor, 0, sizeof(*reactor));
  utarray_init(&reactor->sched, &sched_item_icd);
  utarray_init(&reactor->queued, &size_t_icd);
  utarray_init(&reactor->toqueue, &size_t_icd);
  fmc_pool_init(&reactor->pool);
}

void fmc_reactor_destroy(struct fmc_reactor *reactor) {
  utarray_done(&reactor->sched);
  utarray_done(&reactor->queued);
  utarray_done(&reactor->toqueue);
  if (reactor->stop_list) {
    struct fmc_reactor_stop_item *phead = reactor->stop_list;
    struct fmc_reactor_stop_item *item = NULL;
    struct fmc_reactor_stop_item *tmp = NULL;
    DL_FOREACH_SAFE(phead, item, tmp) {
      DL_DELETE(phead, item);
      free(item);
    }
  }
  fmc_pool_destroy(&reactor->pool);
  for (unsigned int i = 0; reactor->ctxs && i < reactor->size; ++i) {
    if (reactor->ctxs[i]->out_tps) {
      struct fmc_reactor_ctx_out *phead = reactor->ctxs[i]->out_tps;
      struct fmc_reactor_ctx_out *el = NULL;
      struct fmc_reactor_ctx_out *ptmp = NULL;
      DL_FOREACH_SAFE(phead, el, ptmp) {
        DL_DELETE(phead, el);
        if (el->name) free(el->name);
        if (el->type) free(el->type);
        free(el);
      }
    }
    // Free dependency related objects
    utarray_done(&reactor->ctxs[i]->deps);
    free(reactor->ctxs[i]);
  }
  free(reactor->ctxs);
  memset(reactor, 0, sizeof(*reactor));
}

static void utarr_del(void *elt) {
  UT_array * _elt = (UT_array *)elt;
  utarray_done(_elt);
}
static void utarr_init(void *elt) {
  UT_array * _elt = (UT_array *)elt;
  UT_icd deps;
  deps.sz = sizeof(struct fmc_reactor_ctx_dep);
  deps.dtor = NULL;
  deps.copy = NULL;
  deps.init = NULL;
  utarray_init(_elt, &deps);
}

void fmc_reactor_ctx_init(struct fmc_reactor *reactor,
                          struct fmc_reactor_ctx *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->reactor = reactor;
  ctx->idx = reactor->size;
  UT_icd deps;
  deps.sz = sizeof(UT_array);
  deps.dtor = utarr_del;
  deps.copy = NULL;
  deps.init = utarr_init;
  utarray_init(&ctx->deps, &deps);
  fmc_error_init_none(&ctx->err);
}

void fmc_reactor_ctx_push(struct fmc_reactor_ctx *ctx,
                          struct fmc_component_input *inps,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor *r = ctx->reactor;
  struct fmc_reactor_ctx *ctxtmp = NULL;
  ctxtmp = (struct fmc_reactor_ctx *)calloc(1, sizeof(*ctxtmp));
  if (!ctxtmp)
    goto cleanup;
  struct fmc_reactor_ctx **ctxstmp = (struct fmc_reactor_ctx **)realloc(
      r->ctxs, sizeof(*r->ctxs) * (r->size + 1));
  if (!ctxstmp)
    goto cleanup;
  r->ctxs = ctxstmp;
  r->ctxs[r->size] = ctxtmp;
  memcpy(r->ctxs[r->size], ctx, sizeof(*ctx));
  ++r->size;
  return;
cleanup:
  fmc_error_set2(error, FMC_ERROR_MEMORY);
  if (ctxtmp)
    free(ctxtmp);
}

fmc_time64_t fmc_reactor_sched(struct fmc_reactor *reactor) {
  struct sched_item *item =
      (struct sched_item *)utarray_front(&(reactor->sched));
  return item ? item->t : fmc_time64_end();
}

size_t fmc_reactor_run_once(struct fmc_reactor *reactor, fmc_time64_t now,
                            fmc_error_t **error) {
  fmc_error_clear(error);
  bool stop_prev = reactor->stop;
  reactor->stop = __atomic_load_n(&reactor->stop_cl, __ATOMIC_SEQ_CST);
  if (!stop_prev && reactor->stop) {
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
  size_t completed = 0;
  ut_swap(&reactor->queued, &reactor->toqueue, sizeof(reactor->queued));
  do {
    struct sched_item *item =
        (struct sched_item *)utarray_front(&reactor->sched);
    if (!item || fmc_time64_greater(item->t, now))
      break;
    utheap_push(&reactor->queued, &item->idx, FMC_SIZE_T_PTR_LESS);
    utheap_pop(&reactor->sched, FMC_SIZE_T_PTR_LESS);
  } while (true);

  do {
    size_t *item = (size_t *)utarray_front(&reactor->queued);
    if (!item)
      break;
    struct fmc_reactor_ctx *ctx = reactor->ctxs[*item];
    if (!fmc_error_has(&ctx->err) && ctx->exec) {
      ctx->exec(ctx->comp, ctx, now);
      if (fmc_error_has(&ctx->err)) {
        fmc_error_set(error, "failed to run component %s with error: %s",
                      ctx->comp->_vt->tp_name, fmc_error_msg(&ctx->err));
        return completed;
      }
      ++completed;
    }
    utheap_pop(&reactor->queued, FMC_SIZE_T_PTR_LESS);
  } while (true);
  return completed;
}

void fmc_reactor_run(struct fmc_reactor *reactor, bool live,
                     fmc_error_t **error) {
  fmc_error_clear(error);
  static bool stopped = false;
  do {
    fmc_time64_t next = fmc_reactor_sched(reactor);
    fmc_time64_t now = live ? fmc_time64_from_nanos(fmc_cur_time_ns()) : next;
    if ((!utarray_len(&reactor->toqueue) && fmc_time64_is_end(next) && !utarray_len(&reactor->queued)) ||
        (reactor->stop && !reactor->finishing) ||
        reactor->stop >= FMC_REACTOR_HARD_STOP) {
      break;
    }
    fmc_reactor_run_once(reactor, now, error);
    if (*error && !stopped) {
      fmc_reactor_stop(reactor);
      stopped = true;
    }
  } while (true);
}

void fmc_reactor_stop(struct fmc_reactor *reactor) {
  __atomic_fetch_add(&reactor->stop_cl, 1, __ATOMIC_SEQ_CST);
}

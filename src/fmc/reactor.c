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
#include <fmc/signals.h>
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
  memset(reactor, 0, sizeof(*reactor));
  utarray_init(&reactor->sched, &sched_item_icd);
  utarray_init(&reactor->queued, &size_t_icd);
  utarray_init(&reactor->toqueue, &size_t_icd);
}

void fmc_reactor_destroy(struct fmc_reactor *reactor) {
  utarray_done(&reactor->sched);
  utarray_done(&reactor->queued);
  utarray_done(&reactor->toqueue);
  for (unsigned int i = 0; reactor->ctxs && i < reactor->size; ++i) {
    free(reactor->ctxs[i]);
  }
  free(reactor->ctxs);
  memset(reactor, 0, sizeof(*reactor));
}

void fmc_reactor_ctx_init(struct fmc_reactor *reactor,
                          struct fmc_reactor_ctx *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->reactor = reactor;
  ctx->idx = reactor->size;
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
  size_t completed = 0;

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
    if (ctx->exec) {
      ctx->exec(ctx->comp, ctx, now, 0, NULL); // TODO add argc and schmem
      if (fmc_error_has(&ctx->err)) {
        fmc_error_set(error, "failed to run component %s with error: %s",
                      ctx->comp->_vt->tp_name, fmc_error_msg(&ctx->err));
        return completed;
      }
    }
    ++completed;
    utheap_pop(&reactor->queued, FMC_SIZE_T_PTR_LESS);
  } while (true);
  return completed;
}

void fmc_reactor_run(struct fmc_reactor *reactor, bool live,
                     fmc_error_t **error) {
  fmc_error_clear(error);
  do {
    ut_swap(&reactor->queued, &reactor->toqueue, sizeof(reactor->queued));
    fmc_time64_t next = fmc_reactor_sched(reactor);
    fmc_time64_t now = live ? fmc_time64_from_nanos(fmc_cur_time_ns()) : next;
    if ((!utarray_len(&reactor->queued) && fmc_time64_is_end(next)) ||
        (reactor->stop && !reactor->finishing) ||
        reactor->stop >= FMC_REACTOR_HARD_STOP) {
      break;
    }
    // TODO: handle component error
    fmc_reactor_run_once(reactor, now, error);
  } while (true);
}

void fmc_reactor_stop(struct fmc_reactor *reactor) {
  // TODO: make increment atomic
  if (!reactor->stop++) {
    struct fmc_reactor_stop_item *item = NULL;
    struct fmc_reactor_stop_item *tmp = NULL;
    DL_FOREACH_SAFE(reactor->stop_list, item, tmp) {
      if(!item->ctx->finishing) {
        ++reactor->finishing;
        item->ctx->finishing = true;
        if (item->ctx->shutdown) {
          item->ctx->shutdown(item->ctx->comp, item->ctx);
        }
      }
    }
  }
}

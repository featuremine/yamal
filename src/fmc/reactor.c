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
#include <fmc/reactor.h>
#include <fmc/signals.h>
#include <fmc/time.h>
#include <fmc/math.h>
#include <stdlib.h> // calloc() free()
#include <uthash/utlist.h>
#include <uthash/utarray.h>
#include <uthash/utheap.h>

static volatile bool stop_signal = false;
static void sig_handler(int s) { stop_signal = true; }

UT_icd sched_item_icd = {
  .sz = sizeof(struct sched_item)
};

UT_icd size_t_icd = {
  .sz = sizeof(size_t)
};

void fmc_reactor_init(struct fmc_reactor *reactor) {
  // important: initialize lists to NULL
  reactor->comps = NULL;
  reactor->stop = false;
  reactor->done = false;
  reactor->ctxs = NULL;
  reactor->count = 0;
  utarray_init(&reactor->sched, &sched_item_icd);
  utarray_init(&reactor->queued, &size_t_icd);
  utarray_init(&reactor->toqueue, &size_t_icd);
  fmc_set_signal_handler(sig_handler);
}

void fmc_reactor_destroy(struct fmc_reactor *reactor) {
  struct fmc_reactor_component_list *head = reactor->comps;
  struct fmc_reactor_component_list *item;
  struct fmc_reactor_component_list *tmp;
  DL_FOREACH_SAFE(head, item, tmp) {
    DL_DELETE(head, item);
    free(item);
    // Do not delete the component itself.
    // It is not owned by the reactor
  }
  utarray_done(&reactor->sched);
  utarray_done(&reactor->queued);
  utarray_done(&reactor->toqueue);
}

void fmc_reactor_ctx_init(struct fmc_reactor *reactor, 
                          struct fmc_reactor_ctx *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->reactor = reactor;
  ctx->idx = reactor->count;
}

void fmc_reactor_ctx_push(struct fmc_reactor_ctx *ctx,
                          struct fmc_component_input *inps,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor *r = ctx->reactor;
  struct fmc_reactor_ctx *ctxtmp = NULL;
  struct fmc_reactor_component_list *add =
      (struct fmc_reactor_component_list *)calloc(1, sizeof(*add));
  if (!add) goto cleanup;
  ctxtmp = (struct fmc_reactor_ctx *)calloc(1, sizeof(*ctxtmp));
  if(!ctxtmp) goto cleanup;
  struct fmc_reactor_ctx **ctxstmp = (struct fmc_reactor_ctx **)realloc(r->ctxs,
                                               sizeof(*r->ctxs)*(r->count+1));
  if(!ctxstmp) goto cleanup;
  add->comp = ctx->comp;
  add->sched = fmc_time64_end();
  r->ctxs = ctxstmp;
  r->ctxs[r->count] = ctxtmp;
  memcpy(r->ctxs[r->count], ctx, sizeof(*ctx));
  DL_APPEND(r->comps, add);
  ++r->count;
  return;
cleanup:
  fmc_error_set2(error, FMC_ERROR_MEMORY);
  if(add) free(add);
  if(ctxtmp) free(ctxtmp);
}

fmc_time64_t fmc_reactor_sched(struct fmc_reactor *reactor) {
  struct sched_item *item = (struct sched_item *)utarray_front(&(reactor->sched));
  return item ? item->t : fmc_time64_end();
}


size_t fmc_reactor_run_once(struct fmc_reactor *reactor, fmc_time64_t now,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  size_t completed = 0;

  do {
    struct sched_item *item = (struct sched_item *)utarray_front(&reactor->sched);
    if (!item || fmc_time64_greater(item->t, now)) break;
    utheap_push(&reactor->queued, &item->idx, FMC_SIZE_T_PTR_LESS);
    utheap_pop(&reactor->sched, FMC_SIZE_T_PTR_LESS);
  } while (true);
  
  do {
    size_t *item = (size_t *)utarray_front(&reactor->queued);
    if (!item) break;
    struct fmc_reactor_ctx *ctx = reactor->ctxs[*item];
    if(ctx->exec) {
      ctx->exec(ctx->comp, now, ctx);
      if (fmc_error_has(&ctx->comp->_err)) {
        fmc_error_set(error, "failed to run component %s with error: %s", ctx->comp->_vt->tp_name,
                      fmc_error_msg(&ctx->comp->_err));
        return completed;
      }
    }
    ++completed;
    utheap_pop(&reactor->queued, FMC_SIZE_T_PTR_LESS);
  } while (true);

  ut_swap(&reactor->queued, &reactor->toqueue, sizeof(reactor->queued));
  return completed;
}

void fmc_reactor_run_sched(struct fmc_reactor *reactor, fmc_error_t **error) {
  fmc_error_clear(error);
  do {
    fmc_time64_t now = fmc_reactor_sched(reactor);
    if (reactor->done || fmc_time64_is_end(now)) break;
    fmc_reactor_run_once(reactor, now, error);
  } while(true);
  reactor->done = true;
}

void fmc_reactor_run_live(struct fmc_reactor *reactor, fmc_error_t **error) {
  fmc_error_clear(error);
  do {
    fmc_time64_t now = fmc_time64_from_nanos(fmc_cur_time_ns());
    if (reactor->done) break;
    fmc_reactor_run_once(reactor, now, error);
  } while(true);
  reactor->done = true;
}

void fmc_reactor_stop(struct fmc_reactor *reactor) { reactor->stop = true; }

bool fmc_reactor_done(struct fmc_reactor *reactor) { return reactor->done; }

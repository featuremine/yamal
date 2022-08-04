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
#include <stdlib.h> // calloc() free()
#include <uthash/utlist.h>

static volatile bool stop_signal = false;
static void sig_handler(int s) { stop_signal = true; }

void fmc_reactor_init(struct fmc_reactor *reactor) {
  // important: initialize lists to NULL
  reactor->comps = NULL;
  reactor->stop = false;
  reactor->done = true;
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
  fmc_reactor_init(reactor);
}

void fmc_reactor_component_add(struct fmc_reactor *reactor,
                               struct fmc_component *comp, int priority,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor_component_list *add =
      (struct fmc_reactor_component_list *)calloc(1, sizeof(*add));
  if (add) {
    add->comp = comp;
    add->priority = priority;
    add->sched = fmc_time64_end();
    struct fmc_reactor_component_list *head = reactor->comps;
    struct fmc_reactor_component_list *item;
    DL_FOREACH(head, item) {
      if (priority > item->priority) {
        DL_PREPEND_ELEM(reactor->comps, item, add);
        return;
      }
    }
    DL_APPEND(reactor->comps, add);
  } else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
  }
}

fmc_time64_t fmc_reactor_sched(struct fmc_reactor *reactor) {
  fmc_time64_t ret = fmc_time64_end();
  struct fmc_reactor_component_list *head = reactor->comps;
  struct fmc_reactor_component_list *item;
  bool realtime = false;
  DL_FOREACH(head, item) {
    if (!item->comp->_vt->tp_sched) {
      realtime = true;
      continue;
    }
    if (fmc_time64_is_end(item->sched)) {
      item->sched = item->comp->_vt->tp_sched(item->comp);
    }
    ret = fmc_time64_min(item->sched, ret);
  }
  return realtime ? fmc_time64_from_nanos(fmc_cur_time_ns()) : ret;
}

bool fmc_reactor_run_once(struct fmc_reactor *reactor, fmc_time64_t now,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor_component_list **it = &reactor->comps;
  bool complete = false;
  bool done = true;
  while (*it) {
    struct fmc_component *comp = (*it)->comp;
    bool stop = reactor->stop || stop_signal;
    if (!fmc_error_has(&comp->_err)) {
      if (!comp->_vt->tp_sched || fmc_time64_less_or_equal((*it)->sched, now)) {
        if (comp->_vt->tp_proc(comp, now, &stop)) {
          complete = true;
          (*it)->sched = fmc_time64_end();
          it = &reactor->comps;
          continue;
        } else {
          reactor->stop = reactor->stop || fmc_error_has(&comp->_err);
        }
      }
    }
    it = &(*it)->next;
    done = done && stop;
  }
  reactor->done = done;
  return complete;
}

void fmc_reactor_run(struct fmc_reactor *reactor, fmc_error_t **error) {
  fmc_error_clear(error);
  reactor->done = false;
  fmc_time64_t now = fmc_reactor_sched(reactor);
  while (!reactor->done && !fmc_time64_is_end(now)) {
    fmc_reactor_run_once(reactor, now, error);
    now = fmc_time64_max(now, fmc_reactor_sched(reactor));
  }
  reactor->done = true;
}

void fmc_reactor_stop(struct fmc_reactor *reactor) { reactor->stop = true; }

bool fmc_reactor_done(struct fmc_reactor *reactor) { return reactor->done; }

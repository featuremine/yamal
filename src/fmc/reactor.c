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
#include <fmc/time.h>
#include <stdlib.h> // calloc() free()
#include <uthash/utlist.h>

void fmc_reactor_init(struct fmc_reactor *reactor) {
  // important: initialize lists to NULL
  reactor->comps = NULL;
  reactor->stop = false;
  reactor->done = true;
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
                               struct fmc_component *comp,
                               int priority,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_reactor_component_list *add =
      (struct fmc_reactor_component_list *)calloc(1, sizeof(*add));
  if (add) {
    add->comp = comp;
    add->priority = priority;
    add->sched = comp->_vt->tp_sched(comp);
    struct fmc_reactor_component_list *head = reactor->comps;
    struct fmc_reactor_component_list *item;
    DL_FOREACH(head, item) {
      if(priority > item->priority) {
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
  DL_FOREACH(head, item) {
    item->sched = item->comp->_vt->tp_sched ?
                  item->comp->_vt->tp_sched(item->comp) : 
                  fmc_time64_from_nanos(fmc_cur_time_ns());
    ret = fmc_time64_min(item->sched, ret);
  }
  return ret;
}

bool fmc_reactor_run_once(struct fmc_reactor *reactor,
                          fmc_time64_t now,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  bool ret = false;
  bool retcomp = false;
  struct fmc_reactor_component_list *item = reactor->comps;
  while(item != NULL)
  {
    bool stop = reactor->stop;
    if (!item->comp->_vt->tp_sched ||
        fmc_time64_equal(now, item->sched)) {
      // TODO: What to do with high priority RT components
      // Will they block any other component??
      retcomp = item->comp->_vt->tp_proc(item->comp, now, &stop);
      // TODO: How does the stop work here? How do I handle it?
    }
    if(fmc_error_has(&item->comp->_err)) {
      fmc_error_set(error, "failed to run component %s with error: %s", item->comp->_vt->tp_name,
                    fmc_error_msg(&item->comp->_err));
      reactor->stop = true;
      return false;
    } else if (reactor->stop && !stop) {
      // no-op
    } else if ( (item->comp->_vt->tp_sched && retcomp)) {
      item = reactor->comps; // head
      // TODO: Why do we go to head? Don't we need to start from this item?
      // The first high priority items run already.
    } else {
      item = item->next;
    }
    ret |= retcomp;
    retcomp = false;
  }
  if(reactor->stop) {
    reactor->done = true;
  }
  return ret;
}

void fmc_reactor_run(struct fmc_reactor *reactor,
                     fmc_error_t **error) {
  fmc_error_clear(error);
  reactor->done = false;
  fmc_time64_t now = fmc_reactor_sched(reactor);
  while (!reactor->done && !fmc_time64_is_end(now)) {
    fmc_reactor_run_once(reactor, now, error);
    now = fmc_reactor_sched(reactor);
  }
  reactor->stop = false;
  reactor->done = true;
}

void fmc_reactor_stop(struct fmc_reactor *reactor) { reactor->stop = true; }

bool fmc_reactor_done(struct fmc_reactor *reactor) { return reactor->done; }

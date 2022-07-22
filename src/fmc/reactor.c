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

#include <fmc/reactor.h>
#include <fmc/component.h>
#include <fmc/time.h>
#include <fmc/error.h>
#include <uthash/utlist.h>
#include <stdlib.h> // calloc() free()

void fmc_reactor_init(struct fmc_reactor *reactor) {
  // important: initialize lists to NULL
  reactor->comps = NULL;
  reactor->stop = false;
  reactor->done = true;
}

void fmc_reactor_destroy(struct fmc_reactor *reactor) {
  struct fmc_component_list *head = reactor->comps;
  struct fmc_component_list *item;
  struct fmc_component_list *tmp;
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
                               fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_component_list *item =
      (struct fmc_component_list *)calloc(1, sizeof(*item));
  if (item) {
    item->comp = comp;
    DL_APPEND(reactor->comps, item);
  } else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
  }
}

static fm_time64_t next_sched_time(struct fmc_component_list *head) {
  fm_time64_t ret = fm_time64_end();
  struct fmc_component_list *item;
  DL_FOREACH(head, item) {
    if(item->comp->_vt->tp_sched) {
      fm_time64_t comptime = item->comp->_vt->tp_sched(item->comp);
      ret = fm_time64_min(comptime, ret);
    } else {
      // TODO: return wallclock
      return fm_time64_from_nanos(fmc_cur_time_ns()); // return wallclock if any sched is null
    }
  }
  return ret;
}

void fmc_reactor_run(struct fmc_reactor *reactor) {
  reactor->done = false;
  bool proc;
  struct fmc_component_list *head = reactor->comps;
  struct fmc_component_list *item;
  while(!reactor->stop) {
    fm_time64_t now = next_sched_time(reactor->comps);
    if (fm_time64_is_end(now)) {
      break;
    }
    DL_FOREACH(head, item) {
      if (!item->comp->_vt->tp_sched ||
          fm_time64_equal(now, item->comp->_vt->tp_sched(item->comp))) {
        proc = item->comp->_vt->tp_proc(item->comp, now);
        // TODO: what to do with tp_proc return value?
        // TODO: handle error? If 1 component fails, others should fail?
      }
    }
  }
  reactor->stop = false;
  reactor->done = true;
}

void fmc_reactor_stop(struct fmc_reactor *reactor) {
  reactor->stop = true;
}

bool fmc_reactor_is_done(struct fmc_reactor *reactor) {
  return reactor->done;
}
/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc++/lazy_rem_vector.hpp>

#include <ytp/cursor.h>
#include <ytp/glob.h>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "cursor.h"

// TODO: properly handle exceptions

struct ytp_glob {
  ytp_cursor_t *cursor;
  std::unordered_map<std::string, fmc::lazy_rem_vector<ytp_cursor_data_cb_cl_t>>
      prfx_cb;
  std::multimap<std::string_view, ytp_mmnode_offs> name_to_streamid;
};

template <typename F>
static void prfx_for_each(ytp_glob_t *glob, std::string_view namestr,
                          const F &f) {
  if (namestr == "/") {
    for (auto &it : glob->name_to_streamid) {
      if (!f(it.second)) {
        return;
      }
    }
  } else if (*namestr.rbegin() != '/') {
    for (auto [it, end] = glob->name_to_streamid.equal_range(namestr);
         it != end; ++it) {
      if (!f(it->second)) {
        return;
      }
    }
  } else {
    auto it = glob->name_to_streamid.lower_bound(namestr);
    for (; it != glob->name_to_streamid.end() &&
           it->first.substr(0, namestr.size()) == namestr;
         ++it) {
      if (!f(it->second)) {
        return;
      }
    }
  }
}

static void ann_cb(void *closure, uint64_t seqno, ytp_mmnode_offs stream,
                   size_t peer_sz, const char *peer_name, size_t ch_sz,
                   const char *ch_name, size_t encoding_sz,
                   const char *encoding_data, bool subscribed) {
  ytp_glob_t *glob = (ytp_glob_t *)closure;

  glob->name_to_streamid.emplace(std::string(peer_name, ch_sz), stream);

  std::string current_name_key(ch_name, ch_sz);
  if (auto it = glob->prfx_cb.find("/"); it != glob->prfx_cb.end()) {
    for (auto &e : it->second) {
      fmc_error_t *error;
      ytp_cursor_data_cb(glob->cursor, stream, e.cb, e.cl, &error);
    }
  }
  do {
    if (auto it = glob->prfx_cb.find(current_name_key);
        it != glob->prfx_cb.end()) {
      for (auto &e : it->second) {
        fmc_error_t *error;
        ytp_cursor_data_cb(glob->cursor, stream, e.cb, e.cl, &error);
      }
    }
    if (current_name_key.size() < 2) {
      break;
    }
    auto pos = current_name_key.rfind('/', current_name_key.size() - 2);
    if (pos == std::string::npos) {
      break;
    }
    current_name_key.resize(pos + 1);
  } while (true);
}

ytp_glob_t *ytp_glob_new(ytp_cursor_t *cursor, fmc_error_t **error) {
  auto glob = std::make_unique<ytp_glob_t>();

  ytp_cursor_ann_cb(cursor, ann_cb, glob.get(), error);
  if (*error) {
    return nullptr;
  }

  glob->cursor = cursor;
  return glob.release();
}

FMMODFUNC void ytp_glob_del(ytp_glob_t *glob, fmc_error_t **error) {
  fmc_error_clear(error);
  std::default_delete<ytp_glob_t>()(glob);
}

void ytp_glob_prefix_cb(ytp_glob_t *glob, size_t sz, const char *prfx,
                        ytp_cursor_data_cb_t cb, void *closure,
                        fmc_error_t **error) {
  fmc_error_clear(error);
  std::string namestr(prfx, sz);

  ytp_cursor_data_cb_cl_t c{
      .cb = cb,
      .cl = closure,
  };

  glob->prfx_cb[namestr].emplace_back(c);

  prfx_for_each(glob, namestr,
                [glob, cb, closure, error](ytp_mmnode_offs stream) {
                  ytp_cursor_data_cb(glob->cursor, stream, cb, closure, error);
                  if (*error) {
                    return false;
                  }

                  return true;
                });
}

void ytp_glob_prefix_cb_rm(ytp_glob_t *glob, size_t sz, const char *prfx,
                           ytp_cursor_data_cb_t cb, void *closure,
                           fmc_error_t **error) {
  fmc_error_clear(error);
  std::string namestr(prfx, sz);

  ytp_cursor_data_cb_cl_t c{
      .cb = cb,
      .cl = closure,
  };

  std::erase_if(
      glob->prfx_cb[namestr], [&c](const ytp_cursor_data_cb_cl_t &item) {
        return memcmp(&c, &item, sizeof(ytp_cursor_data_cb_cl_t)) == 0;
      });

  prfx_for_each(
      glob, namestr, [glob, cb, closure, error](ytp_mmnode_offs stream) {
        ytp_cursor_data_cb_rm(glob->cursor, stream, cb, closure, error);
        if (*error) {
          return false;
        }

        return true;
      });
}

void ytp_glob_consume(ytp_glob_t *dest, ytp_glob_t *src, fmc_error_t **error) {
  fmc_error_clear(error);

  for (auto &[k, s] : src->prfx_cb) {
    auto &d = dest->prfx_cb[k];
    for (auto &cb : s) {
      d.emplace_back(cb);
    }
    s.clear();
  }
}

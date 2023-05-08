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

#include "stream.hpp"

#include <fmc++/error.hpp>
#include <fmc++/lazy_rem_vector.hpp>

ytp_cursor_t *ytp_cursor_new(ytp_yamal_t *yamal, fmc_error_t **error) {
  auto *cursor = static_cast<ytp_cursor_t *>(aligned_alloc(alignof(ytp_cursor_t), sizeof(ytp_cursor_t)));
  if (!cursor) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return {};
  }

  ytp_cursor_init(cursor, yamal, error);
  if (*error) {
    free(cursor);
    return {};
  }

  return cursor;
}

void ytp_cursor_del(ytp_cursor_t *cursor, fmc_error_t **error) {
  ytp_cursor_destroy(cursor, error);
  if (error) {
    return;
  }

  free(cursor);
}

void ytp_cursor_init(ytp_cursor_t *cursor, ytp_yamal_t *yamal, fmc_error_t **error) {
  try {
    new (cursor) ytp_cursor_t(yamal);
  }
  catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
  }
}

void ytp_cursor_destroy(ytp_cursor_t *cursor, fmc_error_t **error) {
  fmc_error_clear(error);
  cursor->~ytp_cursor();
}

ytp_cursor::ytp_cursor(ytp_yamal_t *yamal) : yamal(yamal), ann_processed(0), data_processed(0) {
  fmc_error_t *error;
  it_data = ytp_yamal_begin(yamal, YTP_STREAM_LIST_DATA, &error);
  if (error) {
    throw fmc::error(*error);
  }
  it_ann = ytp_yamal_begin(yamal, YTP_STREAM_LIST_ANN, &error);
  if (error) {
    throw fmc::error(*error);
  }
}

void ytp_cursor_ann_cb(ytp_cursor_t *cursor,
                       ytp_cursor_ann_cb_t cb, void *closure,
                       fmc_error_t **error) {
  fmc_error_clear(error);
  cursor->cb_ann.emplace_back(ytp_cursor_ann_cb_cl_t{cb, closure});
}

void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor,
                          ytp_cursor_ann_cb_t cb, void *closure,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  auto &l = cursor->cb_ann;
  auto c = ytp_cursor_ann_cb_cl_t(cb, closure);
  for(auto it = l.begin(); it != l.end(); ++it) {
    if (*it == c) {
      l.erase(it);
    }
  }
}

void ytp_cursor_data_cb(ytp_cursor_t *cursor,
                        ytp_stream_t stream,
                        ytp_cursor_data_cb_t cb, void *closure,
                        fmc_error_t **error) {
  fmc_error_clear(error);
  auto &l = cursor->cb_data[stream];
  l.emplace_back(ytp_cursor_data_cb_cl_t{cb, closure});
}

void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor,
                                       ytp_stream_t stream,
                                       ytp_cursor_data_cb_t cb, void *closure,
                                       fmc_error_t **error) {
  fmc_error_clear(error);
  if(auto where = cursor->cb_data.find(stream); where != cursor->cb_data.end()) {
    auto &l = where->second;
    auto c = ytp_cursor_data_cb_cl_t(cb, closure);
    for(auto it = l.begin(); it != l.end(); ++it) {
      if (*it == c) {
        l.erase(it);
      }
    }
  }
}

bool ytp_cursor_term(ytp_cursor_t *cursor) {
  return ytp_yamal_term(cursor->it_data) && ytp_yamal_term(cursor->it_ann);
}

uint64_t ytp_cursor_count(ytp_cursor_t *cursor) {
  return cursor->data_processed;
}

static std::tuple<std::string_view, std::string_view, std::string_view, const std::atomic<uint64_t> *> parse_ann_payload(const char *data, std::size_t sz, fmc_error_t **error) {
  if (sz < sizeof(ann_msg_t)) {
    fmc_error_set(error, "invalid stream announcement");
    return {};
  }

  auto &hdr = *reinterpret_cast<const ann_msg_t *>(data);
  std::size_t peer_name_sz = (uint16_t)ye16toh(hdr.peer_name_sz);
  std::size_t channel_name_sz = (uint16_t)ye16toh(hdr.channel_name_sz);

  if (sz < peer_name_sz + channel_name_sz) {
    fmc_error_set(error, "invalid stream announcement");
    return {};
  }

  auto peername = std::string_view(hdr.payload, peer_name_sz);
  auto chname = std::string_view(hdr.payload + peer_name_sz, channel_name_sz);
  auto encoding = std::string_view(hdr.payload + peer_name_sz + channel_name_sz, sz - channel_name_sz - peer_name_sz - sizeof(ann_msg_t));

  return {peername, chname, encoding, &hdr.subscription};
}

std::tuple<ytp_stream_t, std::string_view> parse_data_payload(const char *data, std::size_t sz, fmc_error_t **error) {
  if (sz < sizeof(data_msg_t)) {
    fmc_error_set(error, "invalid stream announcement");
    return {};
  }

  auto &hdr = *reinterpret_cast<const data_msg_t *>(data);
  auto stream = (uint64_t)ye64toh(hdr.stream);
  return {stream, std::string_view(hdr.payload, sz - sizeof(data_msg_t))};
}

bool ytp_cursor_poll_one_ann(ytp_cursor_t *cursor, fmc_error_t **error) {
  size_t seqno;
  size_t sz;
  const char *dataptr;

  ytp_yamal_read(cursor->yamal, cursor->it_ann, &seqno, &sz, &dataptr, error);
  if (*error) {
    return false;
  }

  ytp_stream_t stream = ytp_yamal_tell(cursor->yamal, cursor->it_ann, error);
  if (*error) {
    return false;
  }

  auto next_it = ytp_yamal_next(cursor->yamal, cursor->it_ann, error);
  if (*error) {
    return false;
  }

  auto [peername, chname, encoding, sub] = parse_ann_payload(dataptr, sz, error);
  if (*error) {
    return false;
  }

  cursor->it_ann = next_it;
  cursor->ann_processed = seqno;

  for (auto &cb : cursor->cb_ann) {
    cb.first(cb.second, stream, seqno, peername.size(), peername.data(), chname.size(), chname.data(), encoding.size(), encoding.data());
  }
  return true;
}

extern bool ytp_cursor_poll_one_data(ytp_cursor_t *cursor, fmc_error_t **error) {
  size_t seqno;
  size_t sz;
  const char *dataptr;
  uint64_t msgtime;

  ytp_time_read(cursor->yamal, cursor->it_data, &seqno, &msgtime, &sz, &dataptr, error);
  if (*error) {
    return false;
  }

  auto [stream, data] = parse_data_payload(dataptr, sz, error);
  if (*error) {
    return false;
  }

  auto stream_it = ytp_yamal_seek(cursor->yamal, stream, error);
  if (*error) {
    return false;
  }

  size_t stream_seqno;
  size_t stream_sz;
  const char *stream_dataptr;
  ytp_yamal_read(cursor->yamal, stream_it, &stream_seqno, &stream_sz, &stream_dataptr, error);
  if (*error) {
    return false;
  }

  if (cursor->ann_processed < stream_seqno) {
    if (ytp_yamal_term(cursor->it_ann)) {
      fmc_error_set(error, "unknown stream reference in data seqno=%zu", seqno);
      return false;
    }
    else {
      return ytp_cursor_poll_one_ann(cursor, error);
    }
  }

  auto next_it = ytp_yamal_next(cursor->yamal, cursor->it_data, error);
  if (*error) {
    return false;
  }

  cursor->it_data = next_it;
  cursor->data_processed = seqno;

  if (auto it = cursor->cb_data.find(stream); it != cursor->cb_data.end()) {
    for (auto &cb : it->second) {
      cb.first(cb.second, seqno, msgtime, stream, data.size(), data.data());
    }
  }
  return true;
}

bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error) {
  if (!ytp_yamal_term(cursor->it_ann)) {
    return ytp_cursor_poll_one_ann(cursor, error);
  }
  if (!ytp_yamal_term(cursor->it_data)) {
    return ytp_cursor_poll_one_data(cursor, error);
  }
  fmc_error_clear(error);
  return false;
}

bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src) {
  if (src->it_data != dest->it_data || src->it_ann != dest->it_ann) {
    return false;
  }

  if (dest->cb_data.empty()) {
    dest->cb_data = std::move(src->cb_data);
  }
  else {
    for (auto &&[k, v] : src->cb_data) {
      auto &d = dest->cb_data[k];
      if (d.empty()) {
        d = std::move(v);
      }
      else {
        d.splice(d.end(), v);
        v.clear();
      }
    }
  }

  if (dest->cb_ann.empty()) {
    dest->cb_ann = std::move(src->cb_ann);
  }
  else {
    dest->cb_ann.splice(dest->cb_ann.end(), src->cb_ann);
    src->cb_ann.clear();
  }

  return true;
}

void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor) {
  cursor->cb_data.clear();
  cursor->cb_ann.clear();
}

void ytp_cursor_seek(ytp_cursor_t *cursor, size_t off, fmc_error_t **error) {
  auto new_it = ytp_yamal_seek(cursor->yamal, off, error);
  if (*error) {
    return;
  }

  cursor->it_data = new_it;
}

size_t ytp_cursor_tell(ytp_cursor_t *cursor, fmc_error_t **error) {
  return ytp_yamal_tell(cursor->yamal, cursor->it_data, error);
}

ytp_anns_t *ytp_anns_new(ytp_yamal_t *yamal, fmc_error_t **error) {
  auto *anns = static_cast<ytp_anns_t *>(aligned_alloc(alignof(ytp_anns_t), sizeof(ytp_anns_t)));
  if (!anns) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return {};
  }

  ytp_anns_init(anns, yamal, error);
  if (*error) {
    free(anns);
    return {};
  }

  return anns;
}

void ytp_anns_del(ytp_anns_t *anns, fmc_error_t **error) {
  ytp_anns_destroy(anns, error);
  if (error) {
    return;
  }

  free(anns);
}

void ytp_anns_init(ytp_anns_t *anns, ytp_yamal_t *yamal, fmc_error_t **error) {
  try {
    new (anns) ytp_anns_t(yamal);
  }
  catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
  }
}

void ytp_anns_destroy(ytp_anns_t *anns, fmc_error_t **error) {
  fmc_error_clear(error);
  anns->~ytp_anns();
}

ytp_anns::ytp_anns(ytp_yamal_t *yamal) : yamal(yamal) {
  fmc_error_t *error;
  it_ann = ytp_yamal_begin(yamal, YTP_STREAM_LIST_ANN, &error);
  if (error) {
    throw fmc::error(*error);
  }
}

template<typename F>
static bool ytp_anns_find_one(ytp_anns_t *anns, fmc_error_t **error, const F &should_stop) {
  do {
    size_t seqno;
    size_t sz;
    const char *dataptr;

    ytp_yamal_read(anns->yamal, anns->it_ann, &seqno, &sz, &dataptr, error);
    if (*error) {
      return false;
    }

    ytp_stream_t stream = ytp_yamal_tell(anns->yamal, anns->it_ann, error);
    if (*error) {
      return false;
    }

    auto next_it = ytp_yamal_next(anns->yamal, anns->it_ann, error);
    if (*error) {
      return false;
    }

    auto [peername, chname, encoding, sub] = parse_ann_payload(dataptr, sz, error);
    if (*error) {
      return false;
    }

    anns->it_ann = next_it;
    anns->ann_processed = seqno;

    using key_t = typename decltype(anns->reverse_map)::key_type;
    auto it = anns->reverse_map.emplace(key_t{peername, chname}, stream);

    if (should_stop(it.first->second, peername, chname, encoding)) {
      break;
    }
  } while (true);
  return true;
}

ytp_stream_t ytp_anns_stream(ytp_anns_t *anns, size_t pz, const char *pn, size_t cz, const char *cn, size_t ez, const char *en, fmc_error_t **error) {
  fmc_error_clear(error);

  std::string_view arg_peer(pn, pz);
  std::string_view arg_ch(cn, cz);
  std::string_view arg_encoding(en, ez);

  if (auto it = anns->reverse_map.find({arg_peer, arg_ch}); it != anns->reverse_map.end()) {
    return it->second;
  }

  ytp_stream_write_ann(anns->yamal, pz, pn, cz, cn, ez, en, error);
  if (*error) {
    return {};
  }

  auto ret = ytp_stream_t{};
  ytp_anns_find_one(anns, error, [&] (ytp_stream_t stream, std::string_view peer, std::string_view ch, std::string_view encoding) {
    if (arg_peer == peer && arg_ch == ch) {
      if (arg_encoding != encoding) {
        fmc_error_set(error, "stream encoding redefinition is not allowed");
        return true;
      }

      ret = stream;
      return true;
    }

    return false;
  });

  return ret;
}

template<typename Handler>
static bool ytp_any_next(ytp_yamal_t *yamal, ytp_iterator_t &it, size_t *sz, const char **data, fmc_error_t **error, const Handler &handler) {
  fmc_error_clear(error);
  if (ytp_yamal_term(it)) {
    return false;
  }

  size_t seqno;
  ytp_yamal_read(yamal, it, &seqno, sz, data, error);
  if (*error) {
    return false;
  }

  handler(error);
  if (*error) {
    return false;
  }

  auto next_it = ytp_yamal_next(yamal, it, error);
  if (*error) {
    return false;
  }

  it = next_it;
  return !ytp_yamal_term(it);
}

void ytp_inds_init(ytp_inds_t *inds, ytp_yamal_t *yamal, fmc_error_t **error) {
  inds->yamal = yamal;
  inds->it_idx = ytp_yamal_begin(yamal, YTP_STREAM_LIST_IDX, error);
}

bool ytp_inds_next(ytp_inds_t *inds, ytp_stream_t *id, uint64_t *offset, size_t *sz, const char **data, fmc_error_t **error) {
  return ytp_any_next(inds->yamal, inds->it_idx, sz, data, error, [&] (fmc_error_t **error) {
    if (*sz < sizeof(idx_msg_t)) {
      fmc_error_set(error, "invalid index message");
      return;
    }

    auto &idx = *reinterpret_cast<idx_msg_t *>(data);
    *id = ye64toh(idx.stream);
    *offset = ye64toh(idx.offset);
    *sz -= sizeof(idx_msg_t);
    *data += sizeof(idx_msg_t);
  });
}

void ytp_subs_init(ytp_subs_t *subs, ytp_yamal_t *yamal, fmc_error_t **error) {
  if (yamal->readonly_) {
    fmc_error_set(error, "unable to initialize using a readonly file descriptor");
    return;
  }

  subs->yamal = yamal;
  subs->it_subs = ytp_yamal_begin(yamal, YTP_STREAM_LIST_SUB, error);
}

bool ytp_subs_next(ytp_subs_t *subs, ytp_stream_t *id, fmc_error_t **error) {
  size_t *sz;
  const char **data;
  bool skip = false;
  do {
    auto ret = ytp_any_next(subs->yamal, subs->it_subs, sz, data, error, [&] (fmc_error_t **error) {
      if (*sz < sizeof(sub_msg_t)) {
        fmc_error_set(error, "invalid index message");
        return;
      }

      auto &idx = *reinterpret_cast<sub_msg_t *>(data);
      *id = ye64toh(idx.stream);

      auto off_sub = ytp_yamal_tell(subs->yamal, subs->it_subs, error);
      if (*error) {
        return;
      }

      auto it_stream = ytp_yamal_seek(subs->yamal, *id, error);
      if (*error) {
        return;
      }

      size_t stream_seqno;
      size_t stream_sz;
      const char *stream_dataptr;
      ytp_yamal_read(subs->yamal, it_stream, &stream_seqno, &stream_sz, &stream_dataptr, error);
      if (*error) {
        return;
      }

      auto [peername, chname, encoding, const_sub] = parse_ann_payload(stream_dataptr, stream_sz, error);
      if (*error) {
        return;
      }

      uint64_t prev_val = 0;
      auto &sub = const_cast<std::atomic<uint64_t> &>(*const_sub);
      skip = !sub.compare_exchange_weak(prev_val, off_sub);
    });
    if (error || !skip) {
      return ret;
    }
  } while(true);
}

char *ytp_stream_reserve(ytp_yamal_t *yamal, size_t size, fmc_error_t **error) {
  fmc_error_clear(error);
  auto *data_msg = (data_msg_t *)ytp_time_reserve(yamal, size + sizeof(data_msg_t), error);
  if (*error) {
    return nullptr;
  }

  return data_msg->payload;
}

ytp_iterator_t ytp_stream_commit(ytp_yamal_t *yamal, uint64_t msgtime, ytp_stream_t id, void *data, fmc_error_t **error) {
  auto *stream_msg = (data_msg_t *)((char *)data - sizeof(data_msg_t));
  stream_msg->stream = htoye64(id);
  return ytp_time_commit(yamal, msgtime, stream_msg, YTP_STREAM_LIST_DATA, error);
}

void ytp_stream_read_ann(ytp_yamal_t *yamal, ytp_stream_t id, uint64_t *seqno, size_t *pz, const char **pn, size_t *cz, const char **cn, size_t *ez, const char **en, uint64_t *sub, fmc_error_t **error) {
  auto it_stream = ytp_yamal_seek(yamal, id, error);
  if (*error) {
    return;
  }

  size_t stream_sz;
  const char *stream_dataptr;
  ytp_yamal_read(yamal, it_stream, seqno, &stream_sz, &stream_dataptr, error);
  if (*error) {
    return;
  }

  auto [peername, chname, encoding, const_sub] = parse_ann_payload(stream_dataptr, stream_sz, error);
  if (*error) {
    return;
  }

  *pz = peername.size();
  *pn = peername.data();
  *cz = chname.size();
  *cn = chname.data();
  *ez = encoding.size();
  *en = encoding.data();
  *sub = const_sub->load();
}

void ytp_stream_write_idx(ytp_yamal_t *yamal, ytp_stream_t id, uint64_t offset, size_t sz, char *data, fmc_error_t **error) {
  auto *ptr = (idx_msg_t *)ytp_yamal_reserve(yamal, sz + sizeof(idx_msg_t), error);
  if (*error) {
    return;
  }
  ptr->stream = id;
  ptr->offset = offset;
  std::memcpy(ptr->payload, data, sz);
  ytp_yamal_commit(yamal, ptr, YTP_STREAM_LIST_IDX, error);
}

ytp_iterator_t ytp_stream_write_ann(ytp_yamal_t *yamal, size_t pz, const char *pn, size_t cz, const char *cn, size_t ez, const char *en, fmc_error_t **error) {
  ann_msg_t *ptr;
  constexpr auto max_pz = (size_t)std::numeric_limits<decltype(ptr->peer_name_sz)>::max();
  constexpr auto max_cz = (size_t)std::numeric_limits<decltype(ptr->channel_name_sz)>::max();
  if (pz > max_pz) {
    fmc_error_set(error, "channel name too long");
    return {};
  }
  if (cz > max_cz) {
    fmc_error_set(error, "channel name too long");
    return {};
  }

  ptr = (ann_msg_t *)ytp_yamal_reserve(yamal, pz + cz + ez + sizeof(ann_msg_t), error);
  if (*error) {
    return {};
  }
  ptr->peer_name_sz = pz;
  ptr->channel_name_sz = cz;
  ptr->subscription = 0;
  std::memcpy(ptr->payload, pn, pz);
  std::memcpy(ptr->payload + pz, cn, cz);
  std::memcpy(ptr->payload + pz + cz, en, ez);
  return ytp_yamal_commit(yamal, ptr, YTP_STREAM_LIST_ANN, error);
}

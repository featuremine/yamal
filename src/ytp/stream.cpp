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

std::tuple<std::string_view, std::string_view, std::string_view, const std::atomic<uint64_t> *> parse_ann_payload(const char *data, std::size_t sz, fmc_error_t **error) {
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
  cursor->cb_ann.emplace_back(cb, closure);
}

void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor,
                          ytp_cursor_ann_cb_t cb, void *closure,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  auto &l = cursor->cb_ann;
  auto c = ytp_cursor_ann_cb_cl_t(cb, closure);
  std::erase_if(l, [&] (const ytp_cursor_ann_cb_cl_t &item) {
    return c == item;
  });
}

void ytp_cursor_data_cb(ytp_cursor_t *cursor,
                        ytp_stream_t stream,
                        ytp_cursor_data_cb_t cb, void *closure,
                        fmc_error_t **error) {
  fmc_error_clear(error);

  if (!cursor->yamal->readonly_) {
    auto stream_it = ytp_yamal_seek(cursor->yamal, stream, error);
    if (*error) {
      return;
    }

    uint64_t stream_seqno;
    size_t stream_sz;
    const char *stream_dataptr;
    ytp_yamal_read(cursor->yamal, stream_it, &stream_seqno, &stream_sz, &stream_dataptr, error);
    if (*error) {
      return;
    }

    auto &const_sub = *std::get<3>(parse_ann_payload(stream_dataptr, stream_sz, error));
    uint64_t unset = SUBSCRIPTION_STATE::NO_SUBSCRIPTION;
    if (const_sub.load() == unset) {
      auto it_sub = ytp_stream_write_sub(cursor->yamal, stream, error);
      if (*error) {
        return;
      }

      auto off_sub = ytp_yamal_tell(cursor->yamal, it_sub, error);
      if (*error) {
        return;
      }

      auto &sub = const_cast<std::atomic<uint64_t> &>(const_sub);
      sub.compare_exchange_weak(unset, off_sub);
    }
  }

  auto &l = cursor->cb_data[stream];
  l.emplace_back(cb, closure);
}

void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor,
                                       ytp_stream_t stream,
                                       ytp_cursor_data_cb_t cb, void *closure,
                                       fmc_error_t **error) {
  fmc_error_clear(error);
  if(auto where = cursor->cb_data.find(stream); where != cursor->cb_data.end()) {
    auto &l = where->second;
    auto c = ytp_cursor_data_cb_cl_t(cb, closure);
    std::erase_if(l, [&] (const ytp_cursor_data_cb_cl_t &item) {
      return c == item;
    });
  }
}

bool ytp_cursor_term_ann(ytp_cursor_t *cursor) {
  if (ytp_yamal_term(cursor->it_ann)) {
    return true;
  }

  fmc_error_t *error;
  uint64_t stream_seqno;
  size_t stream_sz;
  const char *stream_dataptr;
  ytp_yamal_read(cursor->yamal, cursor->it_ann, &stream_seqno, &stream_sz, &stream_dataptr, &error);
  if (error) {
    return true;
  }

  auto sub = std::get<3>(parse_ann_payload(stream_dataptr, stream_sz, &error))->load();
  return sub < SUBSCRIPTION_STATE::NO_SUBSCRIPTION;
}

bool ytp_cursor_term(ytp_cursor_t *cursor) {
  return ytp_yamal_term(cursor->it_data) && ytp_cursor_term_ann(cursor);
}

uint64_t ytp_cursor_count(ytp_cursor_t *cursor) {
  return cursor->data_processed;
}

static std::tuple<ytp_stream_t, std::string_view> parse_data_payload(const char *data, std::size_t sz, fmc_error_t **error) {
  if (sz < sizeof(data_msg_t)) {
    fmc_error_set(error, "invalid stream announcement");
    return {};
  }

  auto &hdr = *reinterpret_cast<const data_msg_t *>(data);
  auto stream = (uint64_t)ye64toh(hdr.stream);
  return {stream, std::string_view(hdr.payload, sz - sizeof(data_msg_t))};
}

bool ytp_cursor_poll_ann(ytp_cursor_t *cursor, fmc_error_t **error) {
  if (ytp_yamal_term(cursor->it_ann)) {
    return false;
  }

  uint64_t seqno;
  size_t sz;
  const char *dataptr;

  ytp_yamal_read(cursor->yamal, cursor->it_ann, &seqno, &sz, &dataptr, error);
  if (*error) {
    return false;
  }

  auto [peername, chname, encoding, const_sub] = parse_ann_payload(dataptr, sz, error);
  if (*error) {
    return false;
  }

  auto sub = const_sub->load();
  if (sub == SUBSCRIPTION_STATE::UNKNOWN) {
    return false;
  }

  auto next_it = ytp_yamal_next(cursor->yamal, cursor->it_ann, error);
  if (*error) {
    return false;
  }

  ytp_stream_t stream = ytp_yamal_tell(cursor->yamal, cursor->it_ann, error);
  if (*error) {
    return false;
  }

  cursor->it_ann = next_it;
  cursor->ann_processed = seqno;
  if (sub == SUBSCRIPTION_STATE::DUPLICATED) {
    return true;
  }

  cursor->cb_ann.lock();
  for (auto it = cursor->cb_ann.begin(); it != cursor->cb_ann.end(); ++it) {
    if (it.was_removed()) {
      continue;
    }
    auto &cb = *it;
    cb.first(cb.second, stream, seqno, peername.size(), peername.data(), chname.size(), chname.data(), encoding.size(), encoding.data());
  }
  cursor->cb_ann.release();
  return true;
}

extern bool ytp_cursor_poll_data(ytp_cursor_t *cursor, fmc_error_t **error) {
  if (ytp_yamal_term(cursor->it_data)) {
    return false;
  }

  uint64_t seqno;
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

  uint64_t stream_seqno;
  size_t stream_sz;
  const char *stream_dataptr;
  ytp_yamal_read(cursor->yamal, stream_it, &stream_seqno, &stream_sz, &stream_dataptr, error);
  if (*error) {
    return false;
  }

  if (cursor->ann_processed < stream_seqno) {
    if (!ytp_cursor_poll_ann(cursor, error)) {
      fmc_error_set(error, "unknown stream reference in data seqno=%zu", seqno);
      return false;
    }
    return true;
  }

  auto next_it = ytp_yamal_next(cursor->yamal, cursor->it_data, error);
  if (*error) {
    return false;
  }

  cursor->it_data = next_it;
  cursor->data_processed = seqno;

  if (auto it2 = cursor->cb_data.find(stream); it2 != cursor->cb_data.end()) {
    auto &d = it2->second;
    d.lock();
    for (auto it = d.begin(); it != d.end(); ++it) {
      if (it.was_removed()) {
        continue;
      }
      auto &cb = *it;
      cb.first(cb.second, seqno, msgtime, stream, data.size(), data.data());
    }
    d.release();
  }
  return true;
}

bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error) {
  bool polled = ytp_cursor_poll_ann(cursor, error);
  if (polled || *error) {
    return polled;
  }
  return ytp_cursor_poll_data(cursor, error);
}

template<typename T>
static void concat_callbacks(fmc::lazy_rem_vector<T> &d, fmc::lazy_rem_vector<T> &s) {
  for (auto it = s.begin(); it != s.end(); ++it) {
    if (it.was_removed()) {
      continue;
    }
    d.emplace_back(*it);
  }
  s.clear();
}

bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src) {
  if (src->it_data != dest->it_data || src->it_ann != dest->it_ann) {
    return false;
  }

  for (auto &&[k, s] : src->cb_data) {
    concat_callbacks(dest->cb_data[k], s);
  }

  concat_callbacks(dest->cb_ann, src->cb_ann);
  return true;
}

void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor) {
  for (auto &&[k, s] : cursor->cb_data) {
    s.clear();
  }

  cursor->cb_ann.clear();
}

void ytp_cursor_seek(ytp_cursor_t *cursor, uint64_t offset, fmc_error_t **error) {
  auto new_it = ytp_yamal_seek(cursor->yamal, offset, error);
  if (*error) {
    return;
  }

  cursor->it_data = new_it;
}

uint64_t ytp_cursor_tell(ytp_cursor_t *cursor, fmc_error_t **error) {
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

ytp_stream_t ytp_anns_stream(ytp_anns_t *anns, size_t pz, const char *pn, size_t cz, const char *cn, size_t ez, const char *en, fmc_error_t **error) {
  fmc_error_clear(error);

  std::string_view arg_peer(pn, pz);
  std::string_view arg_ch(cn, cz);
  std::string_view arg_encoding(en, ez);

  if (auto it = anns->reverse_map.find({arg_peer, arg_ch}); it != anns->reverse_map.end()) {
    auto stream = it->second;
    auto it_stream = ytp_yamal_seek(anns->yamal, stream, error);
    if (*error) {
      return {};
    }

    uint64_t stream_seqno;
    size_t stream_sz;
    const char *stream_dataptr;
    ytp_yamal_read(anns->yamal, it_stream, &stream_seqno, &stream_sz, &stream_dataptr, error);
    if (*error) {
      return {};
    }

    auto [peername, chname, encoding, const_sub] = parse_ann_payload(stream_dataptr, stream_sz, error);
    if (*error) {
      return {};
    }

    if (arg_encoding != encoding) {
      fmc_error_set(error, "stream encoding redefinition is not allowed");
      return {};
    }

    return stream;
  }

  ytp_stream_write_ann(anns->yamal, pz, pn, cz, cn, ez, en, error);
  if (*error) {
    return {};
  }

  auto ret = ytp_stream_t{};
  ytp_anns_lookup_one(anns, error, [&] (ytp_stream_t stream, std::string_view peer, std::string_view ch, std::string_view encoding) {
    if (arg_peer != peer || arg_ch != ch) {
      return false;
    }

    if (arg_encoding != encoding) {
      fmc_error_set(error, "stream encoding redefinition is not allowed");
      return true;
    }

    ret = stream;
    return true;
  });

  return ret;
}

template<typename Handler>
static void ytp_any_next(ytp_yamal_t *yamal, ytp_iterator_t &it, size_t *sz, const char **data, fmc_error_t **error, const Handler &handler) {
  fmc_error_clear(error);

  uint64_t seqno;
  ytp_yamal_read(yamal, it, &seqno, sz, data, error);
  if (*error) {
    return;
  }

  auto next_it = ytp_yamal_next(yamal, it, error);
  if (*error) {
    return;
  }

  handler(error);
  if (*error) {
    return;
  }

  it = next_it;
}

void ytp_inds_init(ytp_inds_t *inds, ytp_yamal_t *yamal, fmc_error_t **error) {
  inds->yamal = yamal;
  inds->it_idx = ytp_yamal_begin(yamal, YTP_STREAM_LIST_IDX, error);
}

bool ytp_inds_next(ytp_inds_t *inds, ytp_stream_t *id, uint64_t *offset, size_t *sz, const char **data, fmc_error_t **error) {
  fmc_error_clear(error);

  if (!ytp_yamal_term(inds->it_idx)) {
    return false;
  }

  ytp_any_next(inds->yamal, inds->it_idx, sz, data, error, [&] (fmc_error_t **error) {
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

  return !*error;
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
  fmc_error_clear(error);

  size_t sz;
  const char *data;
  bool ret = false;
  while(!ytp_yamal_term(subs->it_subs)) {
    ytp_any_next(subs->yamal, subs->it_subs, &sz, &data, error, [&] (fmc_error_t **error) {
      if (sz < sizeof(sub_msg_t)) {
        fmc_error_set(error, "invalid index message");
        return;
      }

      auto &submsg = *reinterpret_cast<const sub_msg_t *>(data);
      *id = ye64toh(submsg.stream);

      auto off_sub = ytp_yamal_tell(subs->yamal, subs->it_subs, error);
      if (*error) {
        return;
      }

      auto it_stream = ytp_yamal_seek(subs->yamal, *id, error);
      if (*error) {
        return;
      }

      uint64_t stream_seqno;
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

      uint64_t prev_val = SUBSCRIPTION_STATE::NO_SUBSCRIPTION;
      auto &sub = const_cast<std::atomic<uint64_t> &>(*const_sub);
      ret = sub.compare_exchange_weak(prev_val, off_sub) || prev_val == off_sub;
    });
    if (*error || ret) {
      return !*error;
    }
  }

  return false;
}

char *ytp_stream_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error) {
  fmc_error_clear(error);
  auto *data_msg = (data_msg_t *)ytp_time_reserve(yamal, sz + sizeof(data_msg_t), error);
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

ytp_iterator_t ytp_stream_write_idx(ytp_yamal_t *yamal, ytp_stream_t id, uint64_t offset, size_t sz, const char *data, fmc_error_t **error) {
  auto *ptr = (idx_msg_t *)ytp_yamal_reserve(yamal, sz + sizeof(idx_msg_t), error);
  if (*error) {
    return {};
  }
  ptr->stream = id;
  ptr->offset = offset;
  std::memcpy(ptr->payload, data, sz);
  return ytp_yamal_commit(yamal, ptr, YTP_STREAM_LIST_IDX, error);
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
  ptr->subscription = SUBSCRIPTION_STATE::UNKNOWN;
  std::memcpy(ptr->payload, pn, pz);
  std::memcpy(ptr->payload + pz, cn, cz);
  std::memcpy(ptr->payload + pz + cz, en, ez);
  return ytp_yamal_commit(yamal, ptr, YTP_STREAM_LIST_ANN, error);
}

ytp_iterator_t ytp_stream_write_sub(ytp_yamal_t *yamal, ytp_stream_t id, fmc_error_t **error) {
  auto *ptr = (sub_msg_t *)ytp_yamal_reserve(yamal, sizeof(sub_msg_t), error);
  if (*error) {
    return {};
  }
  ptr->stream = htoye64(id);
  return ytp_yamal_commit(yamal, ptr, YTP_STREAM_LIST_SUB, error);
}

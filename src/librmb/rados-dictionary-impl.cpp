// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Copyright (c) 2017 Tallence AG and the authors
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 */

#include "rados-dictionary-impl.h"

#include <limits.h>

#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <cstdint>

#include <rados/librados.hpp>


using std::string;
using std::stringstream;
using std::map;
using std::pair;
using std::set;

using librmb::RadosDictionaryImpl;

#define DICT_USERNAME_SEPARATOR '/'
#define DICT_PATH_PRIVATE "priv/"
#define DICT_PATH_SHARED "shared/"

RadosDictionaryImpl::RadosDictionaryImpl(RadosCluster *_cluster, const string &_poolname, const string &_username,
                                         const string &_oid)
    : cluster(_cluster), poolname(_poolname), username(_username), oid(_oid) {
  shared_oid = oid + DICT_USERNAME_SEPARATOR + "shared";
  private_oid = oid + DICT_USERNAME_SEPARATOR + username;
  shared_io_ctx_created = false;
  private_io_ctx_created = false;
}

RadosDictionaryImpl::~RadosDictionaryImpl() {}

const string RadosDictionaryImpl::get_shared_oid() { return shared_oid; }

const string RadosDictionaryImpl::get_private_oid() { return private_oid; }

const string RadosDictionaryImpl::get_full_oid(const std::string &key) {
  if (!key.compare(0, strlen(DICT_PATH_PRIVATE), DICT_PATH_PRIVATE)) {
    return get_private_oid();
  } else if (!key.compare(0, strlen(DICT_PATH_SHARED), DICT_PATH_SHARED)) {
    return get_shared_oid();
  } else {
    // TODO(peter) i_unreached();
  }
  return "";
}

librados::IoCtx &RadosDictionaryImpl::get_shared_io_ctx() {
  if (!shared_io_ctx_created) {
    shared_io_ctx_created = cluster->io_ctx_create(poolname, &shared_io_ctx) == 0;
  }
  return shared_io_ctx;
}

librados::IoCtx &RadosDictionaryImpl::get_private_io_ctx() {
  if (!private_io_ctx_created) {
    if (cluster->io_ctx_create(poolname, &private_io_ctx) == 0) {
      private_io_ctx_created = true;
      private_io_ctx.set_namespace(username);
    }
  }
  return private_io_ctx;
}

librados::IoCtx &RadosDictionaryImpl::get_io_ctx(const std::string &key) {
  if (!key.compare(0, strlen(DICT_PATH_PRIVATE), DICT_PATH_PRIVATE)) {
    return get_private_io_ctx();
  } else if (!key.compare(0, strlen(DICT_PATH_SHARED), DICT_PATH_SHARED)) {
    return get_shared_io_ctx();
  }
  assert(false);
}

int RadosDictionaryImpl::get(const string &key, string *value_r) {
  int r_val = -1;

  set<string> keys;
  keys.insert(key);

  map<std::string, librados::bufferlist> map;
  librados::ObjectReadOperation oro;
  oro.omap_get_vals_by_keys(keys, &map, &r_val);

  librados::bufferlist bl;
  int err = get_io_ctx(key).operate(get_full_oid(key), &oro, &bl);

  if (err == 0) {
    if (r_val == 0) {
      auto it = map.find(key);  // map.begin();
      if (it != map.end()) {
        *value_r = it->second.to_str();
        return 0;
      }
      return -ENOENT;
    } else {
      err = r_val;
    }
  }

  return err;
}

void RadosDictionaryImpl::remove_completion(librados::AioCompletion *c) {
  completions_mutex.lock();
  completions.remove(c);
  completions_mutex.unlock();
}
void RadosDictionaryImpl::push_back_completion(librados::AioCompletion *c) {
  completions_mutex.lock();
  completions.push_back(c);
  completions_mutex.unlock();
}

void RadosDictionaryImpl::wait_for_completions() {
  while (!completions.empty()) {
    auto c = completions.front();
    c->wait_for_complete_and_cb();
    remove_completion(c);
    c->release();
  }
}
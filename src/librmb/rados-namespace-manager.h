// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Copyright (c) 2017-2018 Tallence AG and the authors
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 */
#ifndef SRC_LIBRMB_RADOS_NAMESPACE_MANAGER_H_
#define SRC_LIBRMB_RADOS_NAMESPACE_MANAGER_H_

#include <map>
#include <string>
#include "rados-storage.h"
#include "rados-dovecot-ceph-cfg.h"
#include "rados-guid-generator.h"
namespace librmb {

class RadosNamespaceManager {
 public:
  RadosNamespaceManager(RadosDovecotCephCfg *config_) {
    this->oid_suffix = "_namespace";
    this->config = config_;
  }
  virtual ~RadosNamespaceManager();
  void set_config(RadosDovecotCephCfg *config_) { config = config_; }
  RadosDovecotCephCfg *get_config() { return config; }

  void set_namespace_oid(std::string &namespace_oid_) { this->oid_suffix = oid_suffix; }
  bool lookup_key(const std::string &uid, std::string *value);
  bool add_namespace_entry(const std::string &uid, std::string *value, RadosGuidGenerator *guid_generator_);

 private:
  std::map<std::string, std::string> cache;
  std::string oid_suffix;
  RadosDovecotCephCfg *config;
};

} /* namespace librmb */

#endif /* SRC_LIBRMB_RADOS_NAMESPACE_MANAGER_H_ */

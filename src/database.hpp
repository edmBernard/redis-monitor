//
//  tools.hpp
//  redis-monitor
//
//  Created by Erwan BERNARD on 17/02/2018.
//  Copyright © 2018 Erwan BERNARD. All rights reserved.
//

#pragma once

#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include <iostream>
#include <map>
#include <mutex>
#include <string>

#include "nlohmann/json.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/slice_transform.h"

namespace rm {

// =================================================================================================
class Database {
public:
  virtual void hset(const std::string &prefix, const std::string &key, const std::string &value) = 0;

  virtual const std::map<std::string, std::string> &hgetall(std::string prefix) = 0;

  std::string buildPrefix(char letter, int i);

  int prefixLength = 4;
};

// =================================================================================================
class RocksdbDatabase : public Database {
public:
  RocksdbDatabase(std::string path = "/tmp/redis-monitor", bool clean = false);

  ~RocksdbDatabase() { delete db; }

  void hset(const std::string &prefix, const std::string &key, const std::string &value);

  const std::map<std::string, std::string> &hgetall(std::string prefix);

private:
  std::string path;
  rocksdb::DB *db;
  std::map<std::string, std::string> tmpData;
};

// =================================================================================================
class StlDatabase : public Database {
public:
  StlDatabase() {}

  void hset(const std::string &prefix, const std::string &key, const std::string &value);

  const std::map<std::string, std::string> &hgetall(std::string prefix) { return this->data[prefix]; }

private:
  std::mutex mx;
  std::map<std::string, std::map<std::string, std::string>> data;
};

// =================================================================================================
inline std::string Database::buildPrefix(char letter, int i) {
  std::ostringstream ss;
  ss << letter << std::setw(this->prefixLength - 1) << std::setfill('0') << i;
  return ss.str();
}

// =================================================================================================
inline RocksdbDatabase::RocksdbDatabase(std::string path, bool clean) : path(path) {
  // Configure RocksDB
  rocksdb::Options DBOptions;
  DBOptions.IncreaseParallelism();
  DBOptions.compression = rocksdb::kNoCompression;
  DBOptions.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(this->prefixLength));
  DBOptions.create_if_missing = true;

  if (clean) {
    rocksdb::DestroyDB(this->path, DBOptions);
  }

  rocksdb::Status s = rocksdb::DB::Open(DBOptions, this->path, &this->db);
  if (!s.ok()) {
    std::cout << s.ToString() << std::endl;
  }
}

// =================================================================================================
inline void RocksdbDatabase::hset(const std::string &prefix, const std::string &key, const std::string &value) {
  this->db->Put(rocksdb::WriteOptions(), prefix + key, value);
}

// =================================================================================================
inline const std::map<std::string, std::string> &RocksdbDatabase::hgetall(std::string prefix) {
  auto iter = db->NewIterator(rocksdb::ReadOptions());
  rocksdb::Slice rdb_prefix = prefix;

  this->tmpData.clear();

  for (iter->Seek(rdb_prefix); iter->Valid() && iter->key().starts_with(rdb_prefix); iter->Next()) {
    std::string tmp = iter->key().ToString();
    (this->tmpData)[tmp.substr(this->prefixLength, tmp.size())] = iter->value().ToString();
  }

  return this->tmpData;
}

// =================================================================================================
inline void StlDatabase::hset(const std::string &prefix, const std::string &key, const std::string &value) {
  mx.lock();
  this->data[prefix][key] = value;
  mx.unlock();
}

}; // namespace rm

#endif // !DATABASE_HPP_

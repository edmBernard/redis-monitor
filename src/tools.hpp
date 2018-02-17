//
//  tools.hpp
//  redis-monitor
//
//  Created by Erwan BERNARD on 17/02/2018.
//  Copyright © 2018 Erwan BERNARD. All rights reserved.
//

#pragma once

#ifndef TOOLS_HPP_
#define TOOLS_HPP_

#include <map>
#include <string>
#include <mutex>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace eb {

class Database {
public:
  virtual void hset(const std::string &prefix, const std::string &key, const float &value) = 0;
  virtual const std::map<std::string, float> &hgetall(std::string prefix) = 0;
};

class RocksdbDatabase : public Database {
public:
  RocksdbDatabase(std::string path = "/tmp/redis-monitor") : path(path) {}
  void hset(const std::string &prefix, const std::string &key, const float &value) {
  }
  const std::map<std::string, float> &hgetall(std::string prefix) {
    std::map<std::string, float> tmp;
    tmp["azer"] = 123.123;
    return tmp;
  }

private:
  std::string path;
};

class StlDatabase : public Database {
public:

  StlDatabase() {}
  void hset(const std::string &prefix, const std::string &key, const float &value) {
    mx.lock();
    this->data[prefix][key] = value;
    mx.unlock();
  }
  const std::map<std::string, float> &hgetall(std::string prefix) { return this->data[prefix]; }

private:
  std::mutex mx;
  std::map<std::string, std::map<std::string, float>> data;
};

class Tic {
public:
  Tic(std::string name = "default") : name(name) {}
  void incr() { ++this->count; }
  void reset() { this->count = 0; }
  const int get() const { return this->count; }

private:
  int count = 0;
  std::string name;
};

class Monitor {
public:
  Monitor(Database &database, std::string prefix) : database(database), prefix(prefix) {}
  // virtual void add(std::string data, int value) = 0;  // Differe in function of update policy
  const std::map<std::string, float> &get() { return database.hgetall(this->prefix); }
  const json &to_json() { return json(database.hgetall(this->prefix)); }
  std::pair<std::string, float> lastData;

protected:
  Database &database;
  std::string prefix;
};

class MonitorLength : public Monitor {
public:
  MonitorLength(Database &database, std::string prefix) : Monitor(database, prefix) {}

  void add(std::string date, int keyLength) {
    this->lastData = {date, keyLength};
    database.hset(this->prefix, date, keyLength);
  };
};

class MonitorFrequency : public Monitor {
public:
  MonitorFrequency(Database &database, std::string prefix) : Monitor(database, prefix), counter(prefix) {}

  void incr(std::string date, float value) { this->counter.incr(); };
  void add(std::string date) {
    this->lastData = {date, this->counter.get()};
    database.hset(this->prefix, date, this->counter.get());
    this->counter.reset();
  };

private:
  Tic counter;
};

}; // namespace eb

#endif // !TOOLS_HPP_

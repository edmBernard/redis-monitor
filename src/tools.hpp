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

namespace eb {

class Database {
public:
  virtual void hset(const std::string prefix, const std::string key, const float value) const = 0;
  virtual std::map<std::string, float> const &hgetall(std::string prefix) = 0;
};

class RocksdbDatabase : public Database {
public:
  RocksdbDatabase(std::string path) : path(path) {}
  std::map<std::string, float> &hgetall() {
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
  std::map<std::string, float> &hgetall(std::string prefix) { return this->data[prefix]; }
  void hset(const std::string prefix, const std::string key, const float value) {
    this->data[prefix][key] = value;
  }

private:
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
  std::map<std::string, float> get() { return database.hgetall(this->prefix); }

protected:
  Database &database;
  std::string prefix;
  std::pair<std::string, float> lastData;
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

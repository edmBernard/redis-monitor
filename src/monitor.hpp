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

#include <iostream>
#include <map>
#include <mutex>
#include <string>

#include "database.hpp"
#include "nlohmann/json.hpp"

namespace rm {

// =================================================================================================
class Tic {
public:
  explicit Tic(std::string name = "default") : name(name) {}
  void incr() { ++this->count; }
  void reset() { this->count = 0; }
  const int get() const { return this->count; }

private:
  int count = 0;
  std::string name;
};

// =================================================================================================
class Monitor {
public:
  Monitor(Database &database, int prefix, char letter = 'a')
      : letter(letter), database(database), prefix(this->database.buildPrefix(letter, prefix)) {}

  // virtual void add(std::string data, int value) = 0;  // Differe in function of update policy

  void addSeparator(std::string date) { this->database.hset(this->prefix, date, "null"); }

  const std::map<std::string, std::string> &get() { return database.hgetall(this->prefix); }

  nlohmann::json get_json(std::string id);

  std::pair<std::string, std::string> lastData;

protected:
  char letter;
  Database &database;
  std::string prefix;
};

// =================================================================================================
class MonitorLength : public Monitor {
public:
  MonitorLength(Database &database, int prefix) : Monitor(database, prefix, 'm') {}

  void add(std::string date, std::string keyLength);
};

class MonitorFrequency : public Monitor {
public:
  MonitorFrequency(Database &database, int prefix)
      : Monitor(database, prefix, 'f'), counter(this->database.buildPrefix('f', prefix)) {}

  void incr() { this->counter.incr(); }

  void add(std::string date);

private:
  Tic counter;
};

// =================================================================================================
inline nlohmann::json Monitor::get_json(std::string id) {
  nlohmann::json data;
  nlohmann::json abscisse = nlohmann::json::array();
  nlohmann::json ordinate = nlohmann::json::array();

  auto tmp = this->get();
  for (auto &&it = tmp.begin(); it != tmp.end(); it++) {
    abscisse.push_back(it->first);
    ordinate.push_back(it->second);
  }

  data["id"] = id;
  data["abscisse"] = abscisse;
  data["ordinate"] = ordinate;

  return data;
}

// =================================================================================================
inline void MonitorLength::add(std::string date, std::string keyLength) {
  this->lastData = {date, keyLength};
  this->database.hset(this->prefix, date, keyLength);
}

// =================================================================================================
inline void MonitorFrequency::add(std::string date) {
  this->lastData = {date, std::to_string(this->counter.get())};
  database.hset(this->prefix, date, std::to_string(this->counter.get()));
  this->counter.reset();
}

}; // namespace rm

#endif // !TOOLS_HPP_

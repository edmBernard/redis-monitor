//
//  spawned_loop.hpp
//  redis-monitor
//
//  Created by Erwan BERNARD on 03/03/2018.
//  Copyright © 2018 Erwan BERNARD. All rights reserved.
//

#pragma once
#ifndef SPAWNED_LOOP_HPP_
#define SPAWNED_LOOP_HPP_

#include <chrono> // std::chrono::seconds
#include <string>
#include <vector>

// clang-format off
#include "nlohmann/json.hpp"
#include "database.hpp"
#include "monitor.hpp"
#include "timeUtils.hpp"
#include <cpp_redis/cpp_redis>
#include <uWS.h>
// clang-format on

inline void checkRedisKeyLength(cpp_redis::client &client, std::vector<std::string> keys,
                                std::vector<std::string> patterns, std::vector<rm::MonitorLength> &lengthMonitors,
                                std::vector<rm::MonitorFrequency> &frequencyMonitors, uWS::Hub *h, int rate) {

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(rate));
    std::time_t t = std::time(nullptr);
    nlohmann::json new_data;
    new_data["date"] = timeUtils::convertTimeToStr(t);

    // Process key length
    for (unsigned int i = 0; i < keys.size(); ++i) {
      client.llen(keys[i], [i, keys, &lengthMonitors, t](cpp_redis::reply &reply) {
        lengthMonitors[i].add(timeUtils::convertTimeToStr(t), std::to_string(reply.as_integer()));
      });
    }

    client.sync_commit();

    // Process pattern publish
    for (unsigned int i = 0; i < patterns.size(); ++i) {
      frequencyMonitors[i].add(timeUtils::convertTimeToStr(t));
    }

    for (unsigned int i = 0; i < keys.size(); ++i) {
      nlohmann::json tmp;
      tmp["id"] = keys[i];
      tmp["value"] = lengthMonitors[i].lastData.second;
      new_data["keys"].push_back(tmp);
    }

    for (unsigned int i = 0; i < patterns.size(); ++i) {
      nlohmann::json tmp;
      tmp["id"] = patterns[i];
      tmp["value"] = frequencyMonitors[i].lastData.second;
      new_data["patterns"].push_back(tmp);
    }

    std::string data_string = new_data.dump();
    h->getDefaultGroup<uWS::SERVER>().broadcast(data_string.data(), data_string.length(), uWS::TEXT);
  }
}

#endif // !SPAWNED_LOOP_HPP_

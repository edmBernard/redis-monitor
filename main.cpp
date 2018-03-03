//
//  main.cpp
//  redis-monitor
//
//  Created by Erwan BERNARD on 16/12/2017.
//  Copyright © 2017, 2018 Erwan BERNARD. All rights reserved.
//

#include <chrono> // std::chrono::seconds
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// clang-format off
#include "nlohmann/json.hpp"
#include "inja.hpp"
#include "database.hpp"
#include "monitor.hpp"
#include "timeUtils.hpp"
#include <cpp_redis/cpp_redis>
#include <cxxopts.hpp>
#include <uWS.h>
// clang-format on

using json = nlohmann::json;

void checkRedisKeyLength(cpp_redis::client &client, std::vector<std::string> keys, std::vector<std::string> patterns,
                         std::vector<rm::MonitorLength> &lengthMonitors,
                         std::vector<rm::MonitorFrequency> &frequencyMonitors, uWS::Hub *h, int rate) {

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(rate));
    std::time_t t = std::time(nullptr);
    json new_data;
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
      json tmp;
      tmp["id"] = keys[i];
      tmp["value"] = lengthMonitors[i].lastData.second;
      new_data["keys"].push_back(tmp);
    }

    for (unsigned int i = 0; i < patterns.size(); ++i) {
      json tmp;
      tmp["id"] = patterns[i];
      tmp["value"] = frequencyMonitors[i].lastData.second;
      new_data["patterns"].push_back(tmp);
    }

    std::string data_string = new_data.dump();
    h->getDefaultGroup<uWS::SERVER>().broadcast(data_string.data(), data_string.length(), uWS::TEXT);
  }
}

int main(int argc, char *argv[]) {
  try {
    // =================================================================================================
    // Parse command line options
    cxxopts::Options options(argv[0], "A simple monitoring for redis database");
    options.positional_help("[optional args]").show_positional_help();
    // clang-format off
    options.add_options()
      ("help", "Print help")
      ("h, host", "redis server hostname",
        cxxopts::value<std::string>()->default_value("localhost"), "HOST")
      ("p, port", "redis server port",
        cxxopts::value<int>()->default_value("6379"), "PORT")
      ("a, auth", "redis server authentification",
        cxxopts::value<std::string>()->default_value(""), "AUTH")
      ("rocksdb-path", "path for rocksdb",
        cxxopts::value<std::string>()->default_value("/tmp/redis_monitor"), "PATH")
      ("update-rate", "update rate in second",
        cxxopts::value<int>()->default_value("1"), "RATE")
      ("psubscribe", "pattern used to redis pattern subscription",
        cxxopts::value<std::vector<std::string>>(), "PATTERN")
      ("k, key", "Keys to monitor",
        cxxopts::value<std::vector<std::string>>(), "KEYS");
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }

    std::cout << "host = " << result["host"].as<std::string>() << std::endl;
    std::cout << "port = " << result["port"].as<int>() << std::endl;
    std::cout << "auth = " << result["auth"].as<std::string>() << std::endl;
    std::cout << "update-rate = " << result["update-rate"].as<int>() << std::endl;
    std::cout << "rocksdb-path = " << result["rocksdb-path"].as<std::string>() << std::endl;

    std::vector<std::string> keys;

    if (result.count("key")) {
      std::cout << "keys = [";
      keys = result["key"].as<std::vector<std::string>>();
      for (const auto &k : keys) {
        std::cout << k << ", ";
      }
      std::cout << "]" << std::endl;
    }

    std::vector<std::string> patterns;

    if (result.count("psubscribe")) {
      std::cout << "patterns = [";
      patterns = result["psubscribe"].as<std::vector<std::string>>();
      for (const auto &p : patterns) {
        std::cout << p << ", ";
      }
      std::cout << "]" << std::endl;
    }

    // =================================================================================================
    // Configure Database
    // rm::StlDatabase database;
    rm::RocksdbDatabase database(result["rocksdb-path"].as<std::string>());

    std::vector<rm::MonitorLength> lengthMonitors;
    std::vector<rm::MonitorFrequency> frequencyMonitors;

    // =================================================================================================
    // Create Monitor object for length and frequency
    std::time_t t = std::time(nullptr);

    for (unsigned int i = 0; i < keys.size(); ++i) {
      lengthMonitors.push_back(rm::MonitorLength(database, i));
      // add null data in graph to separate data from previous session
      lengthMonitors[i].addSeparator(timeUtils::convertTimeToStr(t));
    }

    for (unsigned int i = 0; i < patterns.size(); ++i) {
      frequencyMonitors.push_back(rm::MonitorFrequency(database, i));
      // add null data in graph to separate data from previous session
      frequencyMonitors[i].addSeparator(timeUtils::convertTimeToStr(t));
    }

    // =================================================================================================
    // Redis client
    cpp_redis::client client;

    client.connect(result["host"].as<std::string>(), result["port"].as<int>(),
                   [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status) {
                     if (status == cpp_redis::client::connect_state::dropped) {
                       std::cout << "client disconnected from " << host << ":" << port << std::endl;
                     }
                   });

    client.auth(result["auth"].as<std::string>());

    // =================================================================================================
    // Redis subscriber
    cpp_redis::subscriber sub;

    sub.connect(result["host"].as<std::string>(), result["port"].as<int>(),
                [](const std::string &host, std::size_t port, cpp_redis::subscriber::connect_state status) {
                  if (status == cpp_redis::subscriber::connect_state::dropped) {
                    std::cout << "subscriber disconnected from " << host << ":" << port << std::endl;
                  }
                });

    sub.auth(result["auth"].as<std::string>());

    for (unsigned int i = 0; i < patterns.size(); ++i) {
      // redis pattern subsciption on pubsub we increment counter of specific pattern
      sub.psubscribe(patterns[i], [i, &frequencyMonitors](const std::string &chan, const std::string &msg) {
        frequencyMonitors[i].incr();
      });
    }

    sub.commit();

    // =================================================================================================
    // Inja Template
    inja::Environment env = inja::Environment("../templates/");
    env.setElementNotation(inja::ElementNotation::Dot);
    json parameters;
    parameters["keys"] = keys;
    parameters["patterns"] = patterns;
    inja::Template temp = env.parse_template("./index.html.tpl");
    std::string rendered = temp.render(parameters);

    // =================================================================================================
    // Web Server
    uWS::Hub h;
    h.onHttpRequest([rendered, keys, patterns, &lengthMonitors,
                     &frequencyMonitors](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
      // Temp string because regex_match don't allow versatile string get by toString
      std::string url_temp = req.getUrl().toString();

      // Define routing regex
      std::regex route_static_file("/(static/.*)");
      std::regex route_home("/");
      std::regex route_update_keys("/updatekeys");
      std::regex route_update_patterns("/updatepatterns");

      std::smatch pieces_match;

      // =================================================================================================
      // Routing static file
      if (std::regex_match(url_temp, pieces_match, route_static_file)) {

        std::ifstream in("../" + pieces_match[1].str(), std::ios::in | std::ios::binary);
        if (in) {
          std::ostringstream contents;
          contents << in.rdbuf();
          in.close();
          res->end(contents.str().data(), contents.str().length());
        }

        // =================================================================================================
        // Routing home
      } else if (std::regex_match(url_temp, pieces_match, route_home)) {

        res->end(rendered.data(), rendered.length());

        // =================================================================================================
        // Routing update keys
      } else if (std::regex_match(url_temp, pieces_match, route_update_keys)) {

        json data = json::array();

        for (unsigned int i = 0; i < keys.size(); ++i) {
          auto tmp = lengthMonitors[i].get_json(keys[i]);
          data.push_back(tmp);
        }

        std::string data_string = data.dump();
        res->end(data_string.data(), data_string.length());

        // =================================================================================================
        // Routing update patterns
      } else if (std::regex_match(url_temp, pieces_match, route_update_patterns)) {

        json data = json::array();

        for (unsigned int i = 0; i < patterns.size(); ++i) {
          auto tmp = frequencyMonitors[i].get_json(patterns[i]);
          data.push_back(tmp);
        }

        std::string data_string = data.dump();
        res->end(data_string.data(), data_string.length());

        // =================================================================================================
        // Routing Nothing
      } else {

        res->end(nullptr, 0);
      }
    });

    // =================================================================================================
    // Websocker Server
    h.onConnection([](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
      std::cout << "one client connected" << std::endl;
    });

    h.onDisconnection([](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
      std::cout << "one client disconnected" << std::endl;
    });

    // Spawn thread to listen redis periodically, update publish speed and send websocket to client
    std::thread checkingKey(checkRedisKeyLength, std::ref(client), keys, patterns, std::ref(lengthMonitors),
                            std::ref(frequencyMonitors), &h, result["update-rate"].as<int>());

    h.getDefaultGroup<uWS::SERVER>().startAutoPing(30000);
    if (h.listen(3000)) {
      std::cout << "Listening to port 3000" << std::endl;
    } else {
      std::cerr << "Failed to listen to port" << std::endl;
      return -1;
    }

    h.run();

  } catch (const cxxopts::OptionException &e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}

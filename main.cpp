//
//  main.cpp
//  redis-monitor
//
//  Created by Erwan BERNARD on 16/12/2017.
//  Copyright © 2017 Erwan BERNARD. All rights reserved.
//

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <chrono>       // std::chrono::seconds
#include <mutex>

#include <cxxopts.hpp>
#include <cpp_redis/cpp_redis>
#include <uWS.h>
#include "json.hpp"
#include "inja.hpp"
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"
#include "rocksdb/slice_transform.h"


using json = nlohmann::json;

std::stringstream indexHtml;
inja::Environment env = inja::Environment("templates/");
std::vector<int> g_data;
std::mutex g_data_mutex;
std::string kDBPath = "/tmp/redis_monitor";

std::time_t convertStrToTime(std::string stime) {
    std::tm tm;
    std::istringstream iss(stime);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // or just %T in this case
    return timegm(&tm);
}

std::string convertTimeToStr(std::time_t time) {
    std::tm tm = *std::gmtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void checkRedisKeyLength(cpp_redis::client* client, std::vector<std::string> keys, rocksdb::DB* db) {

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (unsigned int i = 0; i < keys.size(); ++i) {
            client->llen(keys[i], [i, db](cpp_redis::reply& reply) {
                g_data_mutex.lock();
                g_data.push_back(reply);

                std::ostringstream ss;
                ss << std::setw(3) << std::setfill('0') << i;
                std::time_t t = std::time(nullptr);
                db->Put(rocksdb::WriteOptions(), ss.str() + convertTimeToStr(t), std::to_string(reply.as_integer()));
                g_data_mutex.unlock();
            });
        }
        client->sync_commit();
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // =================================================================================================
        // Parse command line options
        cxxopts::Options options(argv[0], "A simple monitoring for redis database");
        options
            .positional_help("[optional args]")
            .show_positional_help();
        
        options.add_options()
            ("help", "Print help")
            ("h, host", "redis server hostname", 
                cxxopts::value<std::string>()->default_value("localhost"), "HOST")
            ("p, port", "redis server port", 
                cxxopts::value<int>()->default_value("6379"), "PORT")
            ("a, auth", "redis server authentification", 
                cxxopts::value<std::string>()->default_value(""), "AUTH")
            ("k, key", "Keys to monitor", cxxopts::value<std::vector<std::string>>(), "KEYS")
        ;

        // options.parse_positional({"key"});

        auto result = options.parse(argc, argv);

        std::cout << "Host = " << result["host"].as<std::string>() << std::endl;
        std::cout << "Port = " << result["port"].as<int>() << std::endl;
        std::cout << "Auth = " << result["auth"].as<std::string>() << std::endl;
        
        std::vector<std::string> keys;
        if (result.count("key"))
        {
            std::cout << "Key = [";
            keys = result["key"].as<std::vector<std::string> >();
            for (const auto& k : keys) {
                std::cout << k << ", ";
            }
            std::cout << "]" << std::endl;
        }

        // =================================================================================================
        // Configure RocksDB
        rocksdb::DB *db;
        rocksdb::Options DBOptions;
        DBOptions.IncreaseParallelism();
        
        DBOptions.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(3));
        // create the DB if it's not already present
        DBOptions.create_if_missing = true;
        rocksdb::Status s = rocksdb::DB::Open(DBOptions, kDBPath, &db);
        if (!s.ok()) std::cerr << s.ToString() << std::endl;

        // =================================================================================================
        // Listen Redis keys
        cpp_redis::client *client = new cpp_redis::client();

        client->connect(result["host"].as<std::string>(), result["port"].as<int>(), [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
            if (status == cpp_redis::client::connect_state::dropped) {
                std::cout << "client disconnected from " << host << ":" << port << std::endl;
            }
        });

        client->auth(result["auth"].as<std::string>());

        // Spawn thread to listen redis periodically
        std::thread checkingKey(checkRedisKeyLength, client, keys, db);

        // =================================================================================================
        // Inja Template
        env.setElementNotation(inja::ElementNotation::Dot);
        json parameters;
        parameters["keys"] = keys;
        inja::Template temp = env.parse_template("./index.html.tpl");
        std::string rendered = temp.render(parameters);

        // =================================================================================================
        // Web Server
        uWS::Hub h;
        h.onHttpRequest([rendered, keys, db](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
            std::cout << "req.getUrl() :" << req.getUrl().toString() << std::endl;

            // Temp string because regex_match don't allow versatile string get by toString
            std::string url_temp = req.getUrl().toString();
            // Routing
            std::regex route_static_file("/(static/.*)");
            std::regex route_home("/");
            std::regex route_update("/update");
            std::smatch pieces_match;

            // Routing static file
            if (std::regex_match(url_temp, pieces_match, route_static_file)) {
                std::ifstream in(pieces_match[1].str(), std::ios::in | std::ios::binary);
                if (in) {
                    std::ostringstream contents;
                    contents << in.rdbuf();
                    in.close();
                    res->end(contents.str().data(), contents.str().length());
                }

            // Routing home
            } else if (std::regex_match(url_temp, pieces_match, route_home)) {
                res->end(rendered.data(), rendered.length());

            // Routing update
            } else if (std::regex_match(url_temp, pieces_match, route_update)) {

                json data = json::array();

                auto iter = db->NewIterator(rocksdb::ReadOptions());

                for (unsigned int i = 0; i < keys.size(); ++i) {
                    json temp;
                    json abscisse = json::array();
                    json ordinate = json::array();
                    std::ostringstream ss;
                    ss << std::setw(3) << std::setfill('0') << i;
                    rocksdb::Slice prefix = ss.str();

                    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix); iter->Next()) {
                        std::string tmp = iter->key().ToString();
                        abscisse.push_back(tmp.substr(3, tmp.size()));
                        ordinate.push_back(iter->value().ToString());
                    }

                    temp["id"] = ss.str();
                    temp["abscisse"] = abscisse;
                    temp["ordinate"] = ordinate;
                    data.push_back(temp);
                }

                std::string data_string = data.dump();
                res->end(data_string.data(), data_string.length());

            // Routing Nothing
            } else {
                // i guess this should be done more gracefully?
                res->end(nullptr, 0);
            }
        });

        h.getDefaultGroup<uWS::SERVER>().startAutoPing(30000);
        if (h.listen(3000)) {
            std::cout << "Listening to port 3000" << std::endl;
        } else {
            std::cerr << "Failed to listen to port" << std::endl;
            return -1;
        }

        h.run();

        delete db;
        delete client;

    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}

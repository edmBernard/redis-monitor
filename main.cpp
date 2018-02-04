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
inja::Environment env = inja::Environment("../templates/");
std::mutex g_data_mutex;

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

void checkRedisKeyLength(cpp_redis::client* client, std::vector<std::string> keys, std::vector<std::string> patterns, rocksdb::DB* db, int rate) {

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(rate));
        std::time_t t = std::time(nullptr);

        // Process key length
        for (unsigned int i = 0; i < keys.size(); ++i) {
            client->llen(keys[i], [i, db, t](cpp_redis::reply& reply) {
                std::ostringstream ss;
                ss << "k" << std::setw(3) << std::setfill('0') << i;

                g_data_mutex.lock();
                db->Put(rocksdb::WriteOptions(), ss.str() + convertTimeToStr(t), std::to_string(reply.as_integer()));
                g_data_mutex.unlock();
            });
        }

        client->sync_commit();

        // Process pattern publish
        for (unsigned int i = 0; i < patterns.size(); ++i) {
            std::ostringstream ssc;
            ssc << "c" << std::setw(3) << std::setfill('0') << i;
            std::ostringstream ssp;
            ssp << "p" << std::setw(3) << std::setfill('0') << i;
            std::string value;

            g_data_mutex.lock();
            db->Get(rocksdb::ReadOptions(), ssc.str(), &value);
            db->Put(rocksdb::WriteOptions(), ssp.str() + convertTimeToStr(t), value);
            db->Put(rocksdb::WriteOptions(), ssc.str(), "0");
            g_data_mutex.unlock();
        }

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
            ("rocksdb-path", "path for rocksdb", 
                cxxopts::value<std::string>()->default_value("/tmp/redis_monitor"), "PATH")
            ("update-rate", "update rate in second", 
                cxxopts::value<int>()->default_value("1"), "RATE")
            ("psubscribe", "pattern used to redis pattern subscription", 
                cxxopts::value<std::vector<std::string>>(), "PATTERN")
            ("k, key", "Keys to monitor", 
                cxxopts::value<std::vector<std::string>>(), "KEYS")
        ;
        
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
        if (result.count("key"))
        {
            std::cout << "keys = [";
            keys = result["key"].as<std::vector<std::string> >();
            for (const auto& k : keys) {
                std::cout << k << ", ";
            }
            std::cout << "]" << std::endl;
        }

        std::vector<std::string> patterns;
        if (result.count("psubscribe"))
        {
            std::cout << "patterns = [";
            patterns = result["psubscribe"].as<std::vector<std::string> >();
            for (const auto& p : patterns) {
                std::cout << p << ", ";
            }
            std::cout << "]" << std::endl;
        }

        // =================================================================================================
        // Configure RocksDB
        rocksdb::DB *db;
        rocksdb::Options DBOptions;
        DBOptions.IncreaseParallelism();
        
        DBOptions.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(4));
        // create the DB if it's not already present
        DBOptions.create_if_missing = true;
        std::string kDBPath = result["rocksdb-path"].as<std::string>();
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
        std::thread checkingKey(checkRedisKeyLength, client, keys, patterns, db, result["update-rate"].as<int>());

        // =================================================================================================
        // Redis subscriber
        cpp_redis::subscriber *sub = new cpp_redis::subscriber();

        sub->connect(result["host"].as<std::string>(), result["port"].as<int>(), [](const std::string& host, std::size_t port, cpp_redis::subscriber::connect_state status) {
            if (status == cpp_redis::subscriber::connect_state::dropped) {
                std::cout << "subscriber disconnected from " << host << ":" << port << std::endl;
            }
        });

        sub->auth(result["auth"].as<std::string>());

        for (unsigned int i = 0; i < patterns.size(); ++i) {
            // Counter initialisation at start to remove old counter value
            std::ostringstream ss;
            ss << "c" << std::setw(3) << std::setfill('0') << i;
            g_data_mutex.lock();
            db->Put(rocksdb::WriteOptions(), ss.str(), "0");
            g_data_mutex.unlock();

            // redis pattern subsciption on pubsub we increment counter of specific pattern
            sub->psubscribe(patterns[i], [i, db](const std::string& chan, const std::string& msg) {
                std::ostringstream ss;
                ss << "c" << std::setw(3) << std::setfill('0') << i;
                std::string value;

                g_data_mutex.lock();
                db->Get(rocksdb::ReadOptions(), ss.str(), &value);
                db->Put(rocksdb::WriteOptions(), ss.str(), std::to_string( std::stoi(value) + 1));
                g_data_mutex.unlock();
            });
        }

        sub->commit();

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
        h.onHttpRequest([rendered, keys, patterns, db](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
            std::cout << "req.getUrl() :" << req.getUrl().toString() << std::endl;

            // Temp string because regex_match don't allow versatile string get by toString
            std::string url_temp = req.getUrl().toString();
            // Routing
            std::regex route_static_file("/(static/.*)");
            std::regex route_home("/");
            std::regex route_update_keys("/updatekeys");
            std::regex route_update_patterns("/updatepatterns");
            std::smatch pieces_match;

            // Routing static file
            if (std::regex_match(url_temp, pieces_match, route_static_file)) {
                std::ifstream in("../" + pieces_match[1].str(), std::ios::in | std::ios::binary);
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
            } else if (std::regex_match(url_temp, pieces_match, route_update_keys)) {

                json data = json::array();

                auto iter = db->NewIterator(rocksdb::ReadOptions());

                for (unsigned int i = 0; i < keys.size(); ++i) {
                    json temp;
                    json abscisse = json::array();
                    json ordinate = json::array();
                    std::ostringstream ss;
                    ss << "k" << std::setw(3) << std::setfill('0') << i;
                    rocksdb::Slice prefix = ss.str();

                    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix); iter->Next()) {
                        std::string tmp = iter->key().ToString();
                        abscisse.push_back(tmp.substr(4, tmp.size()));
                        ordinate.push_back(iter->value().ToString());
                    }

                    temp["id"] = keys[i];
                    temp["abscisse"] = abscisse;
                    temp["ordinate"] = ordinate;
                    data.push_back(temp);
                }

                std::string data_string = data.dump();
                res->end(data_string.data(), data_string.length());

            } else if (std::regex_match(url_temp, pieces_match, route_update_patterns)) {

                json data = json::array();

                auto iter = db->NewIterator(rocksdb::ReadOptions());

                for (unsigned int i = 0; i < keys.size(); ++i) {
                    json temp;
                    json abscisse = json::array();
                    json ordinate = json::array();
                    std::ostringstream ss;
                    ss << "p" << std::setw(3) << std::setfill('0') << i;
                    rocksdb::Slice prefix = ss.str();

                    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix); iter->Next()) {
                        std::string tmp = iter->key().ToString();
                        abscisse.push_back(tmp.substr(4, tmp.size()));
                        ordinate.push_back(iter->value().ToString());
                    }

                    temp["id"] = patterns[i];
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
        delete sub;

    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}

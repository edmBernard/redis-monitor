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

#include <cxxopts.hpp>
#include <cpp_redis/cpp_redis>
#include <uWS.h>
#include "json.hpp"
#include "inja.hpp"

using json = nlohmann::json;

std::stringstream indexHtml;
inja::Environment env = inja::Environment("templates/");

void checkRedisKeyLength(std::string redis_host, int redis_port, std::string redis_auth, std::vector<std::string> keys) {
    cpp_redis::client client;

    client.connect(redis_host, redis_port, [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
        if (status == cpp_redis::client::connect_state::dropped) {
            std::cout << "client disconnected from " << host << ":" << port << std::endl;
        }
    });

    client.auth(redis_host);

    while (true) {
        sleep(1);
        for (const auto& key: keys) {
            client.llen(key, [](cpp_redis::reply& reply) {
                std::cout << "client.llen(key) :" << reply << std::endl;
            });
        }
        client.sync_commit();
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

        options.parse_positional({"key"});

        auto result = options.parse(argc, argv);

        std::cout << "Host = " << result["host"].as<std::string>() << std::endl;
        std::cout << "Port = " << result["port"].as<int>() << std::endl;
        std::cout << "Auth = " << result["auth"].as<std::string>() << std::endl;
        
        std::vector<std::string> keys_list;
        if (result.count("key"))
        {
            std::cout << "Key = [";
            keys_list = result["key"].as<std::vector<std::string> >();
            for (const auto& k : keys_list) {
                std::cout << k << ", ";
            }
            std::cout << "]" << std::endl;
        }

        // =================================================================================================
        // Listen Redis keys
        // Spawn thread to listen redis periodically
        std::thread checkingKey(checkRedisKeyLength, result["host"].as<std::string>(), result["port"].as<int>(), result["auth"].as<std::string>(), keys_list);

        // =================================================================================================
        // Inja Template
        env.setElementNotation(inja::ElementNotation::Dot);
        json parameters;
        parameters["keys"] = keys_list;
        inja::Template temp = env.parse_template("./index.html.tpl");
        std::string rendered = temp.render(parameters);

        // =================================================================================================
        // Web Server
        uWS::Hub h;
        h.onHttpRequest([rendered](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
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
                std::string rendered = "Hello world";
                res->end(rendered.data(), rendered.length());

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

    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}

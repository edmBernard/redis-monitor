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

#include <cxxopts.hpp>
#include <cpp_redis/cpp_redis>
#include <uWS.h>
#include "json.hpp"
#include "inja.hpp"

using json = nlohmann::json;

std::stringstream indexHtml;
inja::Environment env = inja::Environment("templates/");

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

        if (result.count("host"))
        {
            std::cout << "Host = " << result["host"].as<std::string>() << std::endl;
        }

        if (result.count("port"))
        {
            std::cout << "Port = " << result["port"].as<int>() << std::endl;
        }

        if (result.count("auth"))
        {
            std::cout << "Auth = " << result["auth"].as<std::string>() << std::endl;
        }

        if (result.count("key"))
        {
            std::cout << "Key = {";
            auto& v = result["key"].as<std::vector<std::string>>();
            for (const auto& s : v) {
                std::cout << s << ", ";
            }
            std::cout << "}" << std::endl;
        }

        // =================================================================================================
        // Redis connection 
        cpp_redis::client client;

        client.connect(result["host"].as<std::string>(), result["port"].as<int>(), [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
            if (status == cpp_redis::client::connect_state::dropped) {
            std::cout << "client disconnected from " << host << ":" << port << std::endl;
            }
        });
        if (result.count("auth"))
        {
            client.auth(result["host"].as<std::string>());
        }

        client.set("hello", "42");

        client.sync_commit();

        uWS::Hub h;

        // =================================================================================================
        // Inja Template
        env.setElementNotation(inja::ElementNotation::Dot); // e.g. time.start

        h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
            std::cout << "req.getUrl() :" << req.getUrl().toString() << std::endl;
            if (req.getUrl().toString() == "/") {
                json parameters;
                parameters["name"] = "world";
                inja::Template temp = env.parse_template("./index.html.tpl");
                std::string rendered = temp.render(parameters); // "Hello World!"
                res->end(rendered.data(), rendered.length());
            } else if (req.getUrl().toString() == "/update") {
                std::string rendered = "Hello world";
                res->end(rendered.data(), rendered.length());
            } else if (req.getUrl().valueLength == 1) {
                json parameters;
                parameters["name"] = "world";
                inja::Template temp = env.parse_template("./index.html.tpl");
                std::string rendered = temp.render(parameters); // "Hello World!"
                res->end(rendered.data(), rendered.length());
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

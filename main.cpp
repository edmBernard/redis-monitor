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

#include <cxxopts.hpp>
#include <cpp_redis/cpp_redis>

int main(int argc, char *argv[])
{
    try
    {
        // Parse command line options
        cxxopts::Options options(argv[0], "A simple monitoring for redis database");
        options
            .positional_help("[optional args]")
            .show_positional_help();
        
        options.add_options()
            ("help", "Print help")
            ("h, host", "redis server hostname including port number", 
                cxxopts::value<std::string>()->default_value("localhost:6379"), "HOST")
            ("k, key", "Keys to monitor", cxxopts::value<std::vector<std::string>>(), "KEYS")
        ;

        options.parse_positional({"key"});

        auto result = options.parse(argc, argv);

        if (result.count("host"))
        {
            std::cout << "Host = " << result["host"].as<std::string>() << std::endl;
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

    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}

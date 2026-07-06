#include "manager.hpp"
#include "json_parser.hpp"
#include "utils.hpp"
#include <iostream>
#include <unordered_map>

Config cfg;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: nypkg [operation] [package/file]" << std::endl;
        return 1;
    }

    std::unordered_map<std::string, bool*> flag_map {
        {"install", &cfg.flags.install}, {"-S", &cfg.flags.install},
        {"update", &cfg.flags.update},   {"-Su", &cfg.flags.update},
        {"remove", &cfg.flags.remove},   {"-R", &cfg.flags.remove},
        {"list", &cfg.flags.list},       {"-l", &cfg.flags.list}
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (flag_map.count(arg)) *flag_map[arg] = true;
        else if (arg[0] != '-') cfg.pkg_name = arg;
    }

    Manager manager;

    manager.run(argc, argv);

    return 0;
}

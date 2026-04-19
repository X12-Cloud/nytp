#include "utils.hpp"
#include "manager.hpp"
#include "package.hpp"
#include "json_parser.hpp"
#include <filesystem>
#include <iostream>
#include <unordered_map>

Config cfg;

struct Operations {
    bool install = false;
    bool remove = false;
    bool url = false;
    bool global = false;
    bool list = false;
} flags;

std::unordered_map<std::string, bool*> flag_map {
    {"install", &flags.install}, {"-S", &flags.install},
    {"remove", &flags.remove},   {"-R", &flags.remove},
    {"-U", &flags.url},
    {"global", &flags.global},   {"-g", &flags.global},
    {"list", &flags.list},       {"-l", &flags.list}, {"-Ql", &flags.list},
};

int main(int argc, char* argv[]) {
    // Check for first 2 arguments
    if (argc < 2) {
        std::cerr << "No operation specified. Usage: nypkg [operation] [package]" << std::endl;
        return 1;
    } //else cfg.operation = argv[1];

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (flag_map.count(arg)) {
            *flag_map[arg] = true;
        } else if (arg[0] != '-') {
            cfg.pkg_name = arg;
        }
    }

    if (flags.list) {
        Manager manager;
        manager.list();
        return 0;
    }

    if (cfg.pkg_name.empty()) {
        std::cerr << "Error: No package name specified." << std::endl;
        return 1;
    }

    Manager manager;

    if (flags.remove || flags.url) {
        if (flags.url) cfg.operation = "-U";
        if (flags.remove) cfg.operation = "-R";

        manager.run();
    }

    if (flags.install) {
        cfg.operation = "-S";

        std::string target_json = cfg.pkg_name;
        if (target_json.find(".json") == std::string::npos) target_json += ".json";

        if (std::filesystem::exists(target_json)) {
            extern Package pkg;
            pkg = JsonParser::parse(target_json);

            manager.run();
        } else {
            std::cerr << "Could not find " << target_json << " to install." << std::endl;
        }
    }

    if (cfg.operation == "test-json") {
        Package pkg = JsonParser::parse(cfg.pkg_name);

        std::cout << "--- Parser Test ---" << std::endl;
        std::cout << "Name:    " << pkg.name << std::endl;
        std::cout << "Version: " << pkg.version << std::endl;
        std::cout << "Repo:    " << pkg.repo_url << std::endl;
        std::cout << "Build:   " << pkg.build_cmd << std::endl;

        if (!pkg.dependencies.empty()) std::cout << "Deps:    " << pkg.dependencies[0] << std::endl;
    }

    return 0;
}

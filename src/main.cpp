#include "utils.hpp"
#include "manager.hpp"
#include "package.hpp"
#include "json_parser.hpp"
#include <filesystem>
#include <iostream>

Config cfg;

int main(int argc, char* argv[]) {
    // Check for first 2 arguments
    if (argc < 2) {
        std::cerr << "No operation specified." << std::endl;
        return 1;
    } else cfg.operation = argv[1];

    if (cfg.operation == "list" || cfg.operation == "-l" || cfg.operation == "Ql") {
        Manager manager;
        manager.list();
        return 0;
    }

    if (cfg.operation == "install" || cfg.operation == "-S" || 
        cfg.operation == "remove" || cfg.operation == "-R") {
        std::string target_json = cfg.pkg_name;
        if (target_json.find(".json") == std::string::npos) target_json += ".json";

        if (argc >= 3) {
            cfg.pkg_name = argv[2];
        } else {
            std::cerr << "No package name specified." << std::endl;
            return 1;
        }

        if (cfg.operation == "remove" || cfg.operation == "R") {
            Manager manager;
            manager.run();
        }

        if (std::filesystem::exists(target_json)) {
            extern Package pkg; 
            pkg = JsonParser::parse(target_json);

            Manager manager;
            manager.run();
        } else {
            std::cerr << "Could not find " << target_json << " to install." << std::endl;
        }
    }

    if (cfg.operation == "test-json") {
        Package pkg = JsonParser::parse("test_pkg.json");

        std::cout << "--- Parser Test ---" << std::endl;
        std::cout << "Name:    " << pkg.name << std::endl;
        std::cout << "Version: " << pkg.version << std::endl;
        std::cout << "Repo:    " << pkg.repo_url << std::endl;
        std::cout << "Build:   " << pkg.build_cmd << std::endl;

        if (!pkg.dependencies.empty()) std::cout << "Deps:    " << pkg.dependencies[0] << std::endl;
    }
}

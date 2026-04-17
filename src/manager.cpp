#include "manager.hpp"
#include "package.hpp"
#include "json_parser.hpp"
#include <iostream>
#include <filesystem>

Package pkg;

const char* home = std::getenv("HOME");

void Manager::fetch(std::string url, std::string dest) {
    std::string registry_dir = std::string(home) + "/.nytrogen/registry/" + pkg.name + ".json";
    std::string sources_dir = std::string(home) + "/.nytrogen/src/" + pkg.name + "/";

    url = pkg.repo_url;
    dest = sources_dir;
    std::string cmd = "git clone " + url + " " + dest;

    if (!std::filesystem::exists(sources_dir)) {
        std::cout << "Making directory: " << sources_dir << std::endl;
        std::filesystem::create_directories(sources_dir);
    }
    if (pkg.isValid()) {
        std::cout << "Cloning repo: " << url << " into " << dest << std::endl;
    } else {
        std::cerr << "Error: no package was found.";
    }

    int result = system(cmd.c_str());
    if (result == 0) {
        std::cout << "Registering package metadata..." << std::endl;
        std::cout << "Registering path: " << registry_dir << std::endl;
        std::filesystem::create_directories(std::filesystem::path(registry_dir).parent_path());
        JsonParser::write(registry_dir, pkg);
    }
}

void Manager::run() {
    std::string registry_dir = std::string(home) + "/.nytrogen/registry/" + pkg.name + ".json";
    std::string sources_dir = std::string(home) + "/.nytrogen/src/" + pkg.name + "/";

    if (cfg.operation == "install" || cfg.operation == "-S") {
        if (std::filesystem::exists(registry_dir)) {
            std::cerr << "Package already exists." << std::endl;
        }
        else Manager::fetch(pkg.repo_url, cfg.pkg_name);

        std::cout << "Installing package: " << cfg.pkg_name << std::endl;

        if (std::filesystem::exists(sources_dir)) {
            std::filesystem::current_path(sources_dir);
        }

        if (!pkg.build_cmd.empty()) {
            std::system(pkg.build_cmd.c_str());
        } else if (std::filesystem::exists("install.sh")) {
            std::system("chmod +x install.sh && ./install.sh");
        }
    }

   if (cfg.operation == "remove" || cfg.operation == "-R") {
        if (!std::filesystem::exists(registry_dir)) {
            std::cerr << "Error: Package '" << cfg.pkg_name << "' is not installed." << std::endl;
            return;
        }
        std::cout << "Removing package: " << cfg.pkg_name << std::endl;

        if (std::filesystem::exists(sources_dir)) {
            std::cout << "Deleting sources: " << sources_dir << std::endl;
            std::filesystem::remove_all(sources_dir);
        }
        std::cout << "Deleting registry entry: " << registry_dir << std::endl;
        std::filesystem::remove(registry_dir);

        std::cout << "Package successfully removed." << std::endl;
        return;
    }

    if (cfg.operation == "-U") {
        fetchRemote(cfg.pkg_name);
    }
}

void Manager::fetchRemote(std::string url) {
    std::string tmp_file = "/tmp/nytro_remote.json";
    std::string cmd = "curl -sL " + url + " -o " + tmp_file;

    if (std::system(cmd.c_str()) == 0) {
        pkg = JsonParser::parse(tmp_file);
        if (std::system(cmd.c_str()) == 0) {
            pkg = JsonParser::parse(tmp_file);
            std::cout << "Downloaded metadata for: " << pkg.name << std::endl;
            std::cout << "Target Repo: " << pkg.repo_url << std::endl;
        }
        std::filesystem::remove(tmp_file);

        fetch(pkg.repo_url, pkg.name); 
        std::cout << "Remote package " << pkg.name << " registered successfully." << std::endl;
    } else {
        std::cerr << "Failed to download package metadata from " << url << std::endl;
    }
}

void Manager::list() {
    const char* home = std::getenv("HOME");
    std::string registry_path = std::string(home) + "/.nytrogen/registry/";

    if (!std::filesystem::exists(registry_path) || std::filesystem::is_empty(registry_path)) {
        std::cout << "No packages installed." << std::endl;
        return;
    }

    std::cout << "Installed Packages:" << std::endl;
    std::cout << "-------------------" << std::endl;

    for (const auto& entry : std::filesystem::directory_iterator(registry_path)) {
        if (entry.path().extension() == ".json") {
            // std::cout << " - " << entry.path().stem().string() << std::endl;

            try {
                Package p = JsonParser::parse(entry.path());
                std::cout << "  " << p.name << " [" << p.version << "]" << std::endl;
            } catch (...) {
                std::cout << "  " << entry.path().stem().string() << " (metadata corrupt)" << std::endl;
            }
        }
    }
}

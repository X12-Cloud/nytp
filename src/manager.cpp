#include "manager.hpp"
#include "package.hpp"
#include "json_parser.hpp"
#include <iostream>
#include <filesystem>

Package pkg;

const char* home = std::getenv("HOME");

void Manager::fetch(std::string url, std::string dest) {
    std::string base_dir = cfg.flags.global ? "/.nypkg/" : "/.nytrogen/";

    std::string registry_dir = std::string(home) + base_dir + "registry/" + pkg.name + ".json";
    std::string sources_dir = std::string(home) + base_dir + "src/" + pkg.name + "/";

    std::string final_url = url.empty() ? pkg.repo_url : url;

    if (final_url.empty()) {
        std::cerr << "Error: No repository URL provided." << std::endl;
        return;
    }
    else std::cout << "Fetching URL: " << url << " for package: " << pkg.name << std::endl;

    if (std::filesystem::exists(sources_dir)) {
        std::cout << "Sources already exist. Skipping clone..." << std::endl;
    } else {
        std::cout << "Cloning repo: " << final_url << " into " << sources_dir << std::endl;
        std::string clone_cmd = "git clone " + final_url + " " + sources_dir;

        if (std::system(clone_cmd.c_str()) != 0) {
            std::cerr << "Error: Git clone failed." << std::endl;
            return; 
        }
    }

    std::cout << "Registering package metadata..." << std::endl;
    std::cout << "Registering path: " << registry_dir << std::endl;
    std::filesystem::create_directories(std::filesystem::path(registry_dir).parent_path());
    JsonParser::write(registry_dir, pkg);
}

void Manager::run() {
    if (cfg.flags.init) {
        initializePath();
        return;
    }

    if (cfg.pkg_name.find(".json") != std::string::npos) {
        if (std::filesystem::exists(cfg.pkg_name)) {
            std::cout << "Loading local package metadata: " << cfg.pkg_name << std::endl;
            pkg = JsonParser::parse(cfg.pkg_name);
        } else {
            std::cerr << "Error: Local file " << cfg.pkg_name << " not found." << std::endl;
            return;
        }
    }

    // Fetch online package from repo
    if (cfg.flags.url) {
        fetchRemote(cfg.pkg_name);
        return;
    }

    std::string active_name = pkg.name.empty() ? cfg.pkg_name : pkg.name;
    std::string base_dir = cfg.flags.global ? "/.nypkg/" : "/.nytrogen/";

    std::string registry_dir = std::string(home) + base_dir + "registry/" + active_name + ".json";
    std::string sources_dir = std::string(home) + base_dir + "src/" + active_name + "/";

    // Install package
    if (cfg.flags.install) {
        if (std::filesystem::exists(registry_dir)) {
            std::cerr << "Package already exists." << std::endl;
        }
        else Manager::fetch(pkg.repo_url, active_name);

        std::cout << "Installing package: " << active_name << std::endl;

        if (std::filesystem::exists(sources_dir)) {
            std::filesystem::current_path(sources_dir);
        }

        if (!pkg.build_cmd.empty()) {
            std::system(pkg.build_cmd.c_str());
        } else if (std::filesystem::exists("install.sh")) {
            std::system("chmod +x install.sh && ./install.sh");
        } else std::cerr << "Error: No build script or install.sh file found." << std::endl;

        if (!pkg.package.empty()) fetchPackageInstall();
    }

    // Remove package
    if (cfg.flags.remove) {
        if (!std::filesystem::exists(registry_dir)) {
            std::cerr << "Error: Package '" << active_name << "' is not installed." << std::endl;
            return;
        }
        std::cout << "Removing package: " << active_name << std::endl;

        if (std::filesystem::exists(sources_dir)) {
            std::cout << "Deleting sources: " << sources_dir << std::endl;
            std::filesystem::remove_all(sources_dir);
        }
        std::cout << "Deleting registry entry: " << registry_dir << std::endl;
        std::filesystem::remove(registry_dir);

        std::cout << "Package successfully removed." << std::endl;
        return;
    }
}

void Manager::fetchRemote(std::string pkg_name) {
    std::string base_url = "https://raw.githubusercontent.com/X12-Cloud/nytp-r/main/";
    std::string full_url = base_url + pkg_name + ".json";

    std::string tmp_file = "/tmp/nytro_remote.json";
    std::string cmd = "curl -sL " + full_url + " -o " + tmp_file;

    if (std::system(cmd.c_str()) == 0) {
        pkg = JsonParser::parse(tmp_file);

        std::cout << "Downloaded metadata for: " << pkg.name << std::endl;
        std::cout << "Target Repo: " << pkg.repo_url << std::endl;

        std::filesystem::remove(tmp_file);

        fetch(pkg.repo_url, pkg.name); 
        std::cout << "Remote package " << pkg.name << " registered successfully." << std::endl;
    } else {
        std::cerr << "Failed to download package metadata from " << full_url << std::endl;
    }
}

void Manager::fetchPackageInstall() {
    std::string binary_path = pkg.package;
    std::string bins_dir = std::string(home) + (cfg.flags.global ?  "/.nypkg/bins/" : "/.nytrogen/bins/");

    if (!std::filesystem::exists(bins_dir)) {
        std::filesystem::create_directories(bins_dir);
    }

    std::string source_dir = binary_path.substr(0, binary_path.find_last_of("/\\"));
    if (!std::filesystem::exists(source_dir)) {
        std::cerr << "Error: Build output directory " << source_dir << " not found. Did the build fail?" << std::endl;
        return; // Exit gracefully instead of crashing
    }

    if (binary_path.find("*") != std::string::npos) {
        std::string source_dir = binary_path.substr(0, binary_path.find_last_of("/\\"));

        std::filesystem::copy(source_dir, bins_dir, 
            std::filesystem::copy_options::recursive | 
            std::filesystem::copy_options::overwrite_existing);

        std::cout << "Successfully deployed package assets to " << bins_dir << std::endl;
    } else {
        std::filesystem::copy_file(binary_path, bins_dir + pkg.name, std::filesystem::copy_options::overwrite_existing);
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

void Manager::initializePath() {
    std::string home_dir = std::getenv("HOME");
    std::string shell_path = std::getenv("SHELL") ? std::getenv("SHELL") : "";
    
    std::string local_bin = home_dir + "/.nytrogen/bins";
    std::string global_bin = home_dir + "/.nypkg/bins";

    if (shell_path.find("fish") != std::string::npos) {
        // For fish
        std::string fish_cmd = "fish -c 'fish_add_path " + local_bin + " " + global_bin + "'";
        std::system(fish_cmd.c_str());
        std::cout << "Detected fish shell. Used fish_add_path for persistence." << std::endl;
    } else {
        // For bash/zsh/sh
        std::string bashrc = home_dir + "/.bashrc";
        std::string local_export = "export PATH=\"" + local_bin + ":$PATH\"";
        std::string global_export = "export PATH=\"" + global_bin + ":$PATH\"";

        std::string cmd_local = "grep -qF '" + local_export + "' " + bashrc + " || echo '" + local_export + "' >> " + bashrc;
        std::string cmd_global = "grep -qF '" + global_export + "' " + bashrc + " || echo '" + global_export + "' >> " + bashrc;

        std::system(cmd_local.c_str());
        std::system(cmd_global.c_str());

        std::cout << "Successfully updated " << bashrc << std::endl;
        std::cout << "Added paths to your $PATH. Restart your terminal to apply." << std::endl;
    }
}

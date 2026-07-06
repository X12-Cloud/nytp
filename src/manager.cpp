#include "manager.hpp"
#include "utils.hpp"
#include "json_parser.hpp"
#include <iostream>

fs::path Manager::getBasePath() const {
    return fs::path(std::getenv("HOME")) / (cfg.flags.global ? ".nypkg" : ".nytrogen");
}

bool Manager::executeSafe(const std::string& cmd) {
    std::cout << ">> " << cmd << std::endl;
    return std::system(cmd.c_str()) == 0;
}

void Manager::run(int argc, char* argv[]) {
    if (cfg.flags.init) { initializeShellPath(); return; }
    if (cfg.flags.install) { install(cfg.pkg_name); return; }
    if (cfg.flags.update) { update(cfg.pkg_name); return; }
    if (cfg.flags.remove) { remove(cfg.pkg_name); return; }
    list();
}

void Manager::install(const std::string& name) {
    if (name.find(".json") != std::string::npos && fs::exists(name)) {
        pkg = JsonParser::parse(name);
    } else {
        fetchRemoteMetadata(name);
    }

    if (pkg.name.empty()) {
        throw std::runtime_error("Package metadata failed to load. Check JSON structure.");
    }

    fs::path lib_dest = getBasePath() / "libs" / pkg.name; 
    fs::create_directories(lib_dest);

    if (pkg.repo_url.find(".tar") != std::string::npos) {
        fs::path temp_tar = fs::path("/tmp") / (pkg.name + ".tar.gz");
        pullArchive(pkg.repo_url, temp_tar);
        extractArchive(temp_tar, lib_dest);
        fs::remove(temp_tar);
    } else {
        cloneRepository(pkg.repo_url, lib_dest);
    }

    runBuild(pkg, lib_dest);

    fs::path registry_dir = getBasePath() / "registry" / (pkg.name + ".json");
    JsonParser::write(registry_dir, pkg);
}

void Manager::cloneRepository(const std::string& url, const fs::path& dest) {
    if (!executeSafe("git clone " + url + " " + dest.string())) {
        throw std::runtime_error("Clone failed");
    }
}

void Manager::pullArchive(const std::string& url, const fs::path& dest) {
    if (!executeSafe("wget " + url + " " + dest.string())) {
        throw std::runtime_error("Pulling archive from: " + url + " failed");
    }
}

void Manager::extractArchive(const fs::path& src, const fs::path& dest) {
    if (!executeSafe("tar -xvf " + src.string() + " -C " + dest.string() + " --strip-components=1")) {
        if (!executeSafe("tar -xvzf " + src.string() + " -C " + dest.string() + " --strip-components=1")) {
            throw std::runtime_error("Extracting file: " + src.string() + " failed (No support for bzip or xz yet)");
        }
    }
}

void Manager::runBuild(const Package& pkg, const fs::path& source_dir) {
    fs::path old_path = fs::current_path();
    fs::current_path(source_dir);
    if (!pkg.build_cmd.empty()) {
        if (!executeSafe(pkg.build_cmd)) throw std::runtime_error("Build failed");
    } else if (fs::exists("install.sh")) {
        executeSafe("chmod +x install.sh && ./install.sh");
    } else {
        std::cout << "(Skipping Build) No build command found." << std::endl;
    }
    fs::current_path(old_path);
}

void Manager::deployBinaries(const Package& pkg, const fs::path& source_dir) {
    fs::path bins = getBasePath() / "bins";
    fs::create_directories(bins);
    fs::copy_file(pkg.package, bins / pkg.name, fs::copy_options::overwrite_existing);
}

void Manager::remove(const std::string& name) {
    fs::path src = getBasePath() / "src" / name;
    fs::path reg = getBasePath() / "registry" / (name + ".json");
    if (fs::exists(src)) fs::remove_all(src);
    if (fs::exists(reg)) fs::remove(reg);
}

void Manager::update(const std::string& name) {
    std::cout << "Updating " << name << "..." << std::endl;
    remove(name);
    install(name);
}

void Manager::list() {
    fs::path registry_path = getBasePath() / "registry";
    if (!fs::exists(registry_path)) return;
    for (const auto& entry : fs::directory_iterator(registry_path)) {
        std::cout << "  " << entry.path().stem().string() << std::endl;
    }
}

void Manager::fetchRemoteMetadata(const std::string& pkg_name) {
    std::string url = "https://raw.githubusercontent.com/X12-Cloud/nytp-r/main/" + pkg_name + ".json";
    std::string tmp = "/tmp/remote_meta.json";
    if (executeSafe("curl -sL " + url + " -o " + tmp)) {
        pkg = JsonParser::parse(tmp);
        fs::remove(tmp);
    } else {
        throw std::runtime_error("Failed to fetch metadata for " + pkg_name);
    }
}

void Manager::initializeShellPath() {
    std::string home_dir = std::getenv("HOME");
    std::string local_bin = home_dir + "/.nytrogen/bins";
    // edit shell config
    std::cout << "Shell path initialized." << std::endl;
}

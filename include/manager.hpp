#ifndef MANAGER_HPP
#define MANAGER_HPP

#include "package.hpp"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Manager {
public:
    void run(int argc, char* argv[]);
    void list();

private:
    // Core Workflow
    void install(const std::string& name);
    void update(const std::string& name);
    void remove(const std::string& name);

    // Internal Helpers
    void fetchRemoteMetadata(const std::string& pkg_name);
    void cloneRepository(const std::string& url, const fs::path& dest);
    void pullArchive(const std::string& url, const fs::path& dest);
    void extractArchive(const fs::path& src, const fs::path& dest);
    void runBuild(const Package& pkg, const fs::path& source_dir);
    void deployBinaries(const Package& pkg, const fs::path& source_dir);
    void initializeShellPath();

    fs::path getBasePath() const;
    bool executeSafe(const std::string& cmd);

    Package pkg;
};

#endif

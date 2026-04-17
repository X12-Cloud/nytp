#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#include <string>
#include <vector>

struct Package {
    std::string name;
    std::string version;
    std::string repo_url;
    std::string build_cmd;
    std::vector<std::string> dependencies;

    // check if the repo is available
    bool isValid() const {
        return !name.empty() && !repo_url.empty();
    }
};

#endif


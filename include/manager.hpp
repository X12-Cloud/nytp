#ifndef MANAGER_HPP
#define MANAGER_HPP

#include "utils.hpp"

class Manager {
public:
    static void run();
    static void fetch(std::string url, std::string dest);
    static void fetchRemote(std::string url);
    static void uninstall();
    static void fetchPackageInstall();
    static void list();
};

#endif


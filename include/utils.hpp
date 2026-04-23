#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

struct Config {
    std::string operation;
    std::string pkg_name;

    struct {
        bool install = false;
        bool remove = false;
        bool url = false;
        bool global = false;
        bool list = false;
        bool init = false;
} flags;

};
extern Config cfg;

struct Color {
    std::string RESET = "\033[0m";
    std::string BOLD = "\033[1m";
    std::string MOCHA_BLUE = "\033[38;2;137;180;250m";
    std::string MOCHA_GREEN = "\033[38;2;166;227;161m";
};
extern Color color;

#endif

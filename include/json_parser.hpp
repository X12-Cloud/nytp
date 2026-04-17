#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include "package.hpp"
#include <filesystem>

class JsonParser {
public:
    static Package parse(const std::filesystem::path& path);
    static void write(const std::filesystem::path& path, const Package& pkg);
};

#endif


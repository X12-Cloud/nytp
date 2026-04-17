#include "json_parser.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

Package JsonParser::parse(const std::filesystem::path& path) {
    std::ifstream file(path);
    json data = json::parse(file);

    Package pkg;
    pkg.name = data.value("name", "unknown");
    pkg.version = data.value("version", "0.0.0");
    pkg.repo_url = data.value("repo", "");
    pkg.build_cmd = data.value("build", "");

    // Dependencies
    if (data.contains("dependencies")) {
        pkg.dependencies = data["dependencies"].get<std::vector<std::string>>();
    }

    return pkg;
}


void JsonParser::write(const std::filesystem::path& path, const Package& pkg) {
    json data;
    data["name"] = pkg.name;
    data["version"] = pkg.version;
    data["repo"] = pkg.repo_url;
    data["build"] = pkg.build_cmd;
    data["dependencies"] = pkg.dependencies;

    std::ofstream file(path);
    if (file.is_open()) {
        file << data.dump(4); // 4 is for pretty printing (indentation)
        file.close();
    }
}

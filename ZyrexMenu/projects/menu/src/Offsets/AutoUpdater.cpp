#include "AutoUpdater.h"
#include "OffsetLoader.hpp"
#include <Console.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

static bool download_to_string(const char* url, std::string& out)
{
    char tmp[MAX_PATH + 1] = {};
    char temp_path[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temp_path)) return false;
    if (!GetTempFileNameA(temp_path, "rlx", 0, tmp)) return false;

    HRESULT hr = URLDownloadToFileA(nullptr, url, tmp, 0, nullptr);
    if (FAILED(hr)) { DeleteFileA(tmp); return false; }

    std::ifstream file(tmp, std::ios::binary);
    if (!file.is_open()) { DeleteFileA(tmp); return false; }

    std::stringstream buf;
    buf << file.rdbuf();
    out = buf.str();
    file.close();
    DeleteFileA(tmp);
    return !out.empty();
}

static bool parse_hpp_to_json(const std::string& hpp_content, std::string& out_json)
{
    nlohmann::json root;
    std::string current_ns;

    std::istringstream stream(hpp_content);
    std::string line;

    std::regex ns_open(R"(^\s*namespace\s+(\w+)\s*\{)");
    std::regex ns_close(R"(^\s*\})");
    std::regex offset_line(R"(^\s*inline\s+(constexpr\s+)?uintptr_t\s+(\w+)\s*=\s*(0x[0-9a-fA-F]+)\s*;)");
    std::regex string_line(R"(^\s*inline\s+std::string\s+(\w+)\s*=\s*\"([^\"]*)\"\s*;)");

    while (std::getline(stream, line))
    {
        std::smatch match;

        if (std::regex_search(line, match, ns_open))
        {
            current_ns = match[1].str();
            continue;
        }

        if (std::regex_search(line, match, ns_close))
        {
            current_ns.clear();
            continue;
        }

        if (std::regex_search(line, match, offset_line))
        {
            std::string name = match[2].str();
            std::string val_str = match[3].str();
            std::uint64_t val = std::stoull(val_str, nullptr, 16);

            if (!current_ns.empty())
                root[current_ns][name] = val;
            continue;
        }

        if (std::regex_search(line, match, string_line))
        {
            std::string name = match[1].str();
            std::string val = match[2].str();
            if (current_ns == "FFlagOffsets" && name == "ClientVersion")
                root["ClientVersion"] = val;
        }
    }

    if (root.empty())
        return false;

    out_json = root.dump(4);
    return true;
}

int AutoUpdater::update_offsets()
{
    Console::Info("[AutoUpdater] Downloading offsets from imtheo.lol...\n");

    std::string content;
    if (!download_to_string("https://offsets.imtheo.lol/offsets.hpp", content))
    {
        Console::Error("[AutoUpdater] FAILED (using compiled defaults)");
        return -1;
    }

    Console::Info("[AutoUpdater] Downloaded %zu bytes, parsing...\n", content.size());

    std::string json_str;
    if (!parse_hpp_to_json(content, json_str))
    {
        Console::Error("[AutoUpdater] Parse failed (using compiled defaults)");
        return -1;
    }

    char tmp_path[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tmp_path)) { strcpy_s(tmp_path, "."); }
    char json_path[MAX_PATH] = {};
    strcat_s(json_path, tmp_path);
    strcat_s(json_path, "auto_offsets.json");
    {
        std::ofstream f(json_path);
        if (!f.is_open())
        {
            Console::Error("[AutoUpdater] Failed to write temp file at %s", json_path);
            return -1;
        }
        f << json_str;
    }

    int result = OffsetLoader::LoadOffsets(json_path);

    try {
        auto j = nlohmann::json::parse(json_str);
        int count = 0;
        for (auto& [ns, vars] : j.items())
            if (vars.is_object()) count += static_cast<int>(vars.size());
        Console::Info("[AutoUpdater] Applied %d runtime offsets\n", count);
    } catch (...) {}

    DeleteFileA(json_path);
    return result;
}

void AutoUpdater::update_all()
{
    Console::Info("=== Auto Updater ===");
    Console::Info("Fetching latest offsets from https://offsets.imtheo.lol ...");
    update_offsets();
    Console::Info("Offsets updated at runtime.");
    Console::Info("====================");
}

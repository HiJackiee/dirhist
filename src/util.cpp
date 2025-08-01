/*
 * @file    src/util.cpp
 * @brief   This source file implements functions defined in include/drihist/util.h
 * @author  yannn
 * @date    2025-07-28
 */

#include <openssl/sha.h>
#include <chrono>
#include <array>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include "internal/util.h"

namespace util {
    std::array<uint8_t, 32> sha256(const std::string& data){
        std::array<uint8_t, 32> hash;
        SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash.data());
        return hash;
    }

    std::string hash_to_str(const std::array<uint8_t, 32>& hash){
        return std::string(reinterpret_cast<const char*>(hash.data()), hash.size());
    }

    int64_t now_ms(){
        return static_cast<std::int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    bool ends_with_suffix(const std::string& str, const std::string& suffix){
        return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool start_with_prefix(const std::string& str, const std::string& prefix){
        return str.rfind(prefix, 0) == 0;
    }

    bool is_snap_bin_file(const fs::path& p){
        // 必须是普通文件
        if (!fs::is_regular_file(p)) return false;

        // 取出文件名
        const std::string name = p.filename().string();
        return start_with_prefix(name, "snap-") && ends_with_suffix(name, ".bin");
    }

    std::string ts_str(int64_t ts) {
        auto tp = std::chrono::system_clock
                                ::time_point(std::chrono::milliseconds(ts));
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&t), "%F %T");
        return oss.str();
    }

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos)
            return ""; // 全部是空格
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    std::vector<std::string> split_by_comma(const std::string& str) {
        std::vector<std::string> result;
        std::string token;
        std::istringstream stream(str);

        while (std::getline(stream, token, ',')) {
            // 去除前导和后导空格
            token = trim(token);
            if (token == "") continue;
            result.push_back(token);
        }

        return result;
    }

    bool compare_paths(const fs::path& path1, const fs::path& path2){
        fs::path path1_abs = fs::canonical(fs::absolute(path1));
        fs::path path2_abs = fs::canonical(fs::absolute(path2));

        return path1_abs == path2_abs;
    }
}
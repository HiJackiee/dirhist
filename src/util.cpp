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
#include "internal/util.h"

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

bool is_snap_bin_file(const fs::path& p)
{
    // 必须是普通文件
    if (!fs::is_regular_file(p)) return false;

    // 取出文件名
    const std::string name = p.filename().string();

    // 长度至少为 "snap-.bin" 的大小
    constexpr std::size_t min_len = 5 + 1 + 4; // "snap-" + 任意字符 + ".bin"
    if (name.size() < min_len) return false;

    // 前缀 "snap-"，后缀 ".bin"
    return name.compare(0, 5, "snap-") == 0 &&
           name.compare(name.size() - 4, 4, ".bin") == 0;
}
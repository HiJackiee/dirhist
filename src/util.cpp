/*
 * @file    src/util.cpp
 * @brief   This source file implements functions defined in include/drihist/util.h
 * @author  yannn
 * @date    2025-07-28
 */

#include <openssl/sha.h>
#include <array>
#include <string>
#include "internal/util.h"

// 计算文件的SHA-256哈希值
std::array<uint8_t, 32> sha256(const std::string& data){
   std::array<uint8_t, 32> hash;
   SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash.data());
   return hash;
}

// 将哈希值转换为字符串
std::string hash_to_str(const std::array<uint8_t, 32>& hash){
    return std::string(reinterpret_cast<const char*>(hash.data()), hash.size());
}
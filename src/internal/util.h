/*
 * @file    src/internal/util.h
 * @brief   This header file defines utility functions for the dirhist project.
 * @author  yannn
 * @date    2025-07-28
 */

#pragma once
#include <array>
#include <string>
#include <cstdint>

// 简化命名空间名称书写
namespace fs = std::filesystem;

// @brief   计算文件的SHA-256哈希值
// @param data 待转换的字符串
// @return  返回计算后的SHA-256哈希值，长度32字节
std::array<uint8_t, 32> sha256(const std::string& data);

// @brief   将SHA-256哈希值按字节转换字符串
// @param hash 待转换的SHA-256哈希值
// @return 返回转换后的字符串
std::string hash_to_str(const std::array<uint8_t, 32>& hash);

// @brief 返回当前时间戳，精确到毫秒
// @return 返回毫秒级时间戳
int64_t now_ms();

// @brief 判断是否为快照文件
// @param path 目录或文件路径
bool is_snap_bin_file(const fs::path& path);
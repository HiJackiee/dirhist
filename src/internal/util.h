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

namespace util {
    // ANSI 颜色常量
    namespace color {
        constexpr const char* RED    = "\033[31m";
        constexpr const char* GREEN  = "\033[32m";
        constexpr const char* YELLOW = "\033[33m";
        constexpr const char* RESET  = "\033[0m";
    }

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

    // @brief 检查是否以指定字符串为后缀
    // @param str 目标字符串
    // @param prefix 待查找后缀字符串
    bool ends_with_suffix(const std::string& str, const std::string& suffix);

    // @brief 检查是否以指定字符串为前缀
    // @param str 目标字符串
    // @param prefix 待查找前缀字符串
    bool start_with_prefix(const std::string& str, const std::string& prefix);

    // @brief 判断是否为快照文件
    // @param path 目录或文件路径
    bool is_snap_bin_file(const fs::path& path);

    // @brief 将时间戳转换为可读字符串
    // @param ts 待转换时间戳
    // @return 返回可读字符串
    std::string ts_str(int64_t ts);

    // @brief 去除前导和后导空格
    // @return 返回处理后的字符串
    std::string trim(const std::string& str);

    // @brief 解析以逗号分隔的字符串并返回子串集合
    // @param str 待解析字符串
    // @result 返回解析后的子串集合
    std::vector<std::string> split_by_comma(const std::string& str);

    // @brief 比较两个路径（绝对或相对）是否相同
    // @param path1
    // @param path2
    // @return 相同返回true，否则false
    bool compare_paths(const fs::path& path1, const fs::path& path2);
}
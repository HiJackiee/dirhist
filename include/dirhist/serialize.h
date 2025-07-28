/* 
 * @file    include/dirhist/serialize.h
 * @author  yannn
 * @date    2025-07-28
 */ 

#pragma once
#include "dirhist/snapshot.h"

namespace{
    constexpr uint64_t MAGIC = 0x4448495354415040ULL;   // "DIRSTAP"
    constexpr uint8_t VERSION = 1; // 当前版本号

    // @brief 定义文件头部
    struct Header{
        uint64_t magic = MAGIC;     // 文件头标识
        uint8_t version = VERSION;  // 版本号
        int64_t timestamp = 0;      // 时间戳
        uint64_t root_offset = 0;   // 根节点偏移
        uint64_t root_size = 0;     // 根节点大小
    };
}

namespace dirhist {
    // @brief 序列化目录树
    // @param root 目录树根节点指针
    // @param ts 时间戳
    void write_snapshot(const Node& root, int64_t ts);
}
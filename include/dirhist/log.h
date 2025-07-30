/* 
 * @file    include/dirhist/log.h
 * @author  yannn
 * @date    2025-07-28
 */ 

#include "serialize.h"

namespace dirhist {
    // @brief 定义日志条目
    struct LogEntry {
        int64_t timestamp = 0;  // 时间戳
        uint64_t file_size = 0; // 文件大小
        std::string path;       // 路径（相对）
    };

    // @brief 读取快照日志
    // @param n 读取日志条目的数量
    // @param target_dir 目标文件夹，默认为 ./.dirhist
    void list_snapshots(int n = -1, const fs::path& target_dir = ".dirhist");
}
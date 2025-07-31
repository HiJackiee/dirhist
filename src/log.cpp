/*
 * @file    src/log.cpp
 * @brief   This source file implements the functions for 'log' command.
 * @author  yannn
 * @date    2025-07-28
 */
#include <algorithm>
#include "dirhist/log.h"
#include "internal/util.h"

namespace dirhist {
    void list_snapshots(int n, const fs::path& target_dir){
        // 确保target_dir为绝对路径
        fs::path target_abs = fs::absolute(target_dir);
        // 检查根目录是否存在
        if (!fs::exists(target_abs)){
            std::cerr << "target path does not exist: " << target_abs << std::endl;
            return;
        }

        std::vector<LogEntry> entries;
        for (const auto& e: fs::directory_iterator(target_dir)){
            if(!e.is_regular_file()) continue;
            if(!util::is_snap_bin_file(e.path())) continue;

            std::ifstream ifs(e.path());
            if (!ifs) {
                throw std::runtime_error("Error opening snapshot file: "
                                            + e.path().string());
            }
            // 读取文件头
            Header hdr;
            read(ifs, hdr);

            // 检查文件头
            if (hdr.magic != MAGIC || hdr.version != 1) continue;
            entries.emplace_back(LogEntry{hdr.timestamp, e.file_size(), e.path()});
        }

        if (!(entries.size())){
            std::cerr << "No snapshot file found at: " 
                      << target_dir.string() << std::endl
                      << "Using 'dirhist snap <directory_path>' to create snapshot"
                      << std::endl;
            return;
        }

        // 基于时间戳排序
        std::sort(entries.begin(), entries.end(), 
            [](const auto& a, const auto& b){return a.timestamp > b.timestamp;});
        
        // 打印日志
        std::cout << "Timestamp            Size        File" << std::endl;
        // 默认情况打印所有记录
        if (n == -1) {
            for (const auto& e: entries){
                std::cout << util::ts_str(e.timestamp) << "  "
                          << std::left << std::setw(9) << e.file_size << "  "
                          << e.path << std::endl;
            }
        } else if (n >= 0) {
            for (int i = 0; i < n && i < entries.size(); ++i){
                auto e = entries.at(i);
                std::cout << util::ts_str(e.timestamp) << "  "
                          << std::left << std::setw(9) << e.file_size << "  "
                          << e.path << std::endl;
            }
        } else {
            std::cerr << "Invaild num: " << n << "[num >= -1]"<< std::endl;
        }
    }
}
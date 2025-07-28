/* 
 * @file    include/dirhist/snapshot.h
 * @author  yannn
 * @date    2025-07-28
 */ 

#pragma once
#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <filesystem>

// 简化命名空间名称书写
namespace fs = std::filesystem;

namespace{
    // @brief Node节点，标识一个文件或者目录
    struct Node {
        std::string path;       // 文件或者目录路径（使用相对于根目录的相对路径）
        bool is_dir = false;    // 是否为目录
        bool is_symlink = false; // 是否为符号链接
        uint64_t size = 0;      // 文件或目录大小
        uint64_t mtime = 0;     // 最后修改时间
        std::array<uint8_t, 32> hash{0};    // 文件或目录的SHA256哈希值
        std::vector<std::unique_ptr<Node>> children{nullptr}; // 子结点
    };

    // @brief 辅助函数，递归遍历目录
    // @param current_path 当前遍历的路径
    // @param root 根目录路径
    // @return 返回构建目录树根节点指针
    std::unique_ptr<Node> walk_dir(const fs::path& current_path, const fs::path& root);
}

namespace dirhist {

    // @brief 构建目录树
    // @param root 根目录路径
    // @return 返回构建的目录树根节点指针
    std::unique_ptr<Node> build_tree(const fs::path& root);
}
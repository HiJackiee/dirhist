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

namespace dirhist {
    // @brief Node节点，标识一个文件或者目录
    struct Node {
        std::string path;       // 文件或者目录路径（使用相对于根目录的相对路径）
        std::string abs_root;   // 当前目录树根节点绝对路径
        bool is_dir = false;    // 是否为目录
        bool is_symlink = false; // 是否为符号链接
        uint64_t size = 0;       // 文件或目录大小
        int64_t mtime = 0;      // 最后修改时间
        std::array<uint8_t, 32> hash{0};    // 文件或目录的SHA256哈希值
        std::vector<std::unique_ptr<Node>> children; // 子节点列表
    };

    // @brief 辅助函数，递归遍历目录
    // @param current_path 当前遍历的路径
    // @param root 根目录路径（绝对路径）
    // @return 返回构建目录树根节点指针
    std::unique_ptr<Node> walk_dir(const fs::path& current_path, const fs::path& root);

    // @brief 构建目录树
    // @param root 根目录路径
    // @return 返回构建的目录树根节点指针
    std::unique_ptr<Node> build_tree(const fs::path& root);

    // @brief 辅助函数，递归打印目录结构
    // @param node 目录树节点指针，引用方式不会获取所有权
    // @param level 当前打印层级，用于控制缩进和控制打印深度
    // @param is_last 是否为最后一个子节点
    // @param prefix 当前节点的缩进前缀，默认为空字符串
    // @param max_depth 最大打印目录结构深度
    void aux_display_tree(const std::unique_ptr<Node>& node, int level = 0
        , bool is_last = false, std::string prefix = "", int max_depth = -1);

    // @brief 可视化目录结构
    // @param root 目录树根节点指针，引用方式不会获取所有权
    // @param max_depth 打印目录结构的深度，默认为-1时打印所有层级
    void display_tree(const std::unique_ptr<Node>& root, int max_depth = -1);
}
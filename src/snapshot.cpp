/*
 * @file    src/snapshot.cpp
 * @brief   This source file implements the functions from disk to Merkle tree.
 * @author  yannn
 * @date    2025-07-28
 */

#include "dirhist/snapshot.h"
#include "internal/util.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace dirhist {
    std::unique_ptr<Node>
        walk_dir(const fs::path& current_path, const fs::path& root){
        // 设置当前节点
        // 不直接使用 is_symlink()来判断是否为符号链接
        fs::file_status st = fs::symlink_status(current_path);
        std::unique_ptr<Node> current_node = std::make_unique<Node>();
        current_node->path = current_path.lexically_relative(root).string();
        current_node->is_dir = fs::is_directory(current_path);
        current_node->is_symlink = st.type() == fs::file_type::symlink;

        // 针对符号链接，需要确保目标存在才能获取最后修改时间
        try {
            current_node->mtime = fs::last_write_time(current_path)
                                                .time_since_epoch().count();
        }
        catch (const fs::filesystem_error& e){
            std::cerr << "Error getting last write time: " << e.what() 
                      << " for path: " << current_path << std::endl;
            current_node->mtime = 0;  // 设置为 0 或其他默认值
        }
        
        // 处理目录节点（内部节点），若符号链接目标为目录，此时 is_dir 亦为true
        if (current_node->is_dir && !current_node->is_symlink){
            std::vector<fs::directory_entry> entries;
            std::error_code ec; // 用于处理权限等错误
            for (const auto& entry: fs::directory_iterator(current_path, ec)){
                if (ec){
                    std::cerr << "Error accessing directory: " 
                            << ec.message() << std::endl;
                    continue; // 跳过无法访问的条目
                }

                entries.push_back(entry);
            }
            // 确保按照路径字典序排序
            std::sort(entries.begin(), entries.end()
                                    , [](const fs::directory_entry& a,
                                         const fs::directory_entry& b)
                                        {return a.path() < b.path();});
            
            // 目录节点的哈希值设置为SHA256(path+'\0'+所有子节点哈希按路径字典序拼接)
            std::string data = current_node->path + '\0';
            uint64_t total_size = 0;
            for (const auto& entry: entries){
                // 递归处理其子节点，同时计算子节点大小之和作目录节点大小
                std::unique_ptr<Node> child_node = walk_dir(entry.path(), root);
                if (child_node) {
                    // 将子节点的哈希值拼接到当前节点数据中
                    data += hash_to_str(child_node->hash);
                    // 累加子节点大小
                    total_size += child_node->size;
                    // 将子节点添加到当前节点的子节点列表中
                    current_node->children.push_back(std::move(child_node));
                }
            }
            // 设置节点大小为所有子节点大小之和，并计算哈希值
            current_node->size = total_size;
            current_node->hash = sha256(data);
        }
        // 处理符号链接（叶子节点）
        else if (current_node->is_symlink){
            try {
                // 将链接目标作为文件内容
                std::string target_path = fs::read_symlink(current_path).string();
                current_node->size = target_path.size();
                current_node->hash = sha256(current_node->path + '\0' + target_path);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error reading symlink: "
                          << e.what()<< "for path: "<< current_path << std::endl;
                return nullptr;
            }
        }
        // 处理文件节点（叶子节点）
        else {
            current_node->size = fs::file_size(current_path);
            // 若为文件节点（或符号链接），直接计算其SHA256哈希值
            std::ifstream file(current_path, std::ios::binary);
            if (!file){
                std::cerr << "Error opening file: "<< current_path << std::endl;
                return nullptr;
            }
            // 这里采用一次性读入文件内容方式，后续可以针对大文件优化为分块计算的方式
            std::stringstream ss;
            ss << file.rdbuf();
            // 设置当前节点哈希值
            // 计算方式为 SHA256(path+‘\0’+raw_bytes)
            current_node->hash 
                    = sha256(current_node->path + '\0' + std::move(ss).str());
        }
        return current_node;
    }

    std::unique_ptr<Node> build_tree(const fs::path& root){
        // 确保root为绝对路径
        fs::path root_abs = fs::absolute(root);
        // 检查根目录是否存在
        if (!fs::exists(root_abs)){
            std::cerr << "Root path does not exist: " << root_abs << std::endl;
            return nullptr;
        }
        // 递归遍历目录，构建目录树
        return walk_dir(root_abs, root_abs);
    }
}


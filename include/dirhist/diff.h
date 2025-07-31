/* 
 * @file    include/dirhist/diff.h
 * @author  yannn
 * @date    2025-07-28
 */ 

#include "dirhist/snapshot.h"

namespace dirhist {
    // Unchanged 用于占位
    enum class ChangeType {Added, Deleted, Modified};

    struct DiffEntry {
        ChangeType type = ChangeType::Added;    // 类型
        std::string path;       // 路径
        uint64_t old_size = 0;  // 先前文件大小
        uint64_t new_size = 0;  // 当前文件大小
        int64_t old_mtime = 0;  // 先前修改时间
        int64_t new_mtime = 0;  // 当前修改时间
        std::array<uint8_t, 32> old_hash{0};    // 先前的hash值
        std::array<uint8_t, 32> new_hash{0};    // 当前的hash值
    };

    // @brief 颜色高亮打印DiffEntry
    // @param entry 待打印的DiffEntry
    void print_colored_DiffEntry(const DiffEntry& entry);

    // @brief 标记整棵子树，针对全删或全增的情况
    // @param node 目录树节点
    // @param out 输出的目标 DiffEntry 列表
    void mark_subtree(const Node& node, ChangeType type, std::vector<DiffEntry>& out);

    // @brief 比较两棵 merkle树，返回 DiffEntry 列表（增|删|改）
    // @param old_node 旧merkle树根节点
    // @param new_node 新merkle树根节点
    // @return 返回 DiffEntry 列表
    void diff_nodes(const Node& old_node, const Node& new_node
                                                , std::vector<DiffEntry>& out);

    // @brief 比较两棵 merkle树，打印目录树变化（增|删|改）信息
    // @param old_root 旧merkle树根节点
    // @param new_root 新merkle树根节点
    void diff(const Node& old_root, const Node& new_root);

    // @brief 获取目标文件夹下的最新快照
    // @param target_dir 目标文件夹
    // @return 返回最新快照的路径（没有快照时返回空路径）
    fs::path latest_snap(const fs::path& target_dir);
}
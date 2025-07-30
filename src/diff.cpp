/*
 * @file    src/log.cpp
 * @brief   This source file implements the functions for 'log' command.
 * @author  yannn
 * @date    2025-07-28
 */

#include "dirhist/diff.h"

namespace dirhist {
    void mark_subtree(const Node& node, ChangeType type, std::vector<DiffEntry>& out) {
        // 无论内部节点还是叶子节点，都先处理自身
        // 增加的条目只需要记录当前的信息即可
        if (type == ChangeType::Added) {
            out.push_back(DiffEntry{.type = type, .path = node.path,
                            .new_size = node.size, .new_mtime = node.mtime, 
                            .new_hash = node.hash});
        }
        // 减少的条目只需要记录先前的信息即可
        else if (type == ChangeType::Deleted) {
            out.push_back(DiffEntry{.type = type, .path = node.path,
                            .old_size = node.size, .old_mtime = node.mtime,
                            .old_hash = node.hash});
        }
        // 叶子节点（文件或符号链接），处理后直接退出
        if (!node.is_dir || node.is_symlink) return;
        // 内部节点（目录）则递归处理
        else {
            // 遍历孩子节点
            for (const auto& child: node.children){
                mark_subtree(*child, type, out);
            }
        }
    }

    void diff_nodes(const Node& old_node, const Node& new_node
                                            , std::vector<DiffEntry>& out) {
        // 节点hash值相同，节点对应子树无变化
        if (old_node.hash == new_node.hash) return;
        
        // 旧节点为叶子节点，而新节点为内部节点
        if (!(old_node.is_dir && !old_node.is_symlink) 
            && (new_node.is_dir && !new_node.is_symlink)) {
            // 旧节点标记为删除，新节点标记为新增（包括其子树）
            out.push_back(DiffEntry{.type = ChangeType::Deleted, .path = old_node.path, 
                        .old_size = old_node.size, .old_mtime = old_node.mtime, 
                        .old_hash = old_node.hash});
            
            // 新节点及其子树标记为新增
            mark_subtree(new_node, ChangeType::Added, out);
        }
        // 旧节点为内部节点，而新节点为叶子节点
        else if ((old_node.is_dir && !old_node.is_symlink) 
            && !(new_node.is_dir && !new_node.is_symlink)) {
            // 旧节点及其子树标记为删除
            mark_subtree(old_node, ChangeType::Deleted, out);
            // 新节点标记为新增
            out.push_back(DiffEntry{.type = ChangeType::Added, .path = new_node.path,
                        .new_size = new_node.size, .new_mtime = new_node.mtime, 
                        .new_hash = new_node.hash});
        }
        // 旧节点和新节点均为叶子节点
        else if (!(old_node.is_dir && !old_node.is_symlink)
            && !(new_node.is_dir && !new_node.is_symlink)) {
            // 直接标记为修改即可
            out.push_back(DiffEntry{.type = ChangeType::Modified, .path = new_node.path, 
                        .old_size = old_node.size, .new_size = new_node.size, 
                        .old_mtime = old_node.mtime, .new_mtime = new_node.mtime, 
                        .old_hash = old_node.hash, .new_hash = new_node.hash});
        }
        // 否则，旧节点和新节点均为目录，递归处理其子节点
        else {
            // 目录树创建时，节点子节点已按照其路径字典序排序，故无需再次排序
            // 见 snapshot.cpp::walk_dir
            size_t i = 0, j = 0;
            size_t old_child_cnt = old_node.children.size();
            size_t new_child_cnt = new_node.children.size();
            while (i < old_child_cnt || j < new_child_cnt) {
                if (i < old_child_cnt && (j == new_child_cnt 
                    || old_node.children[i]->path < new_node.children[j]->path)) {
                        mark_subtree(*old_node.children[i], ChangeType::Deleted, out);
                }
                else if (j < new_child_cnt && (i == old_child_cnt
                    || new_node.children[j]->path < old_node.children[i]->path)) {
                        mark_subtree(*new_node.children[j], ChangeType::Added, out);
                }
                else {
                    diff_nodes(*old_node.children[i], *new_node.children[j], out);
                    ++i;++j;
                }
            }
        }
    }

    void diff(const Node& old_node, const Node& new_node) {

    }
}
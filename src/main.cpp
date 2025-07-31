/*
 * @file    src/main.cpp
 * @brief   This source file implements the main logic of the dirhist.
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I./include -o bin/dirhist src/main.cpp  src/snapshot.cpp src/serialize.cpp src/log.cpp src/diff.cpp src/util.cpp -lssl -lcrypto

#include <iostream>
#include <algorithm>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"
#include "dirhist/log.h"
#include "dirhist/diff.h"
#include "internal/util.h"

int process_snap(int argc, char *argv[]);  // "snap" 指令处理函数
int process_tree(int argc, char *argv[]);  // "tree" 指令处理函数
int process_log(int argc, char *argv[]);   // "log"  指令处理函数
int process_diff(int argc, char *argv[]);  // "diff" 指令处理函数
int process_rm(int argc, char *argv[]);    // "rm"   指令处理函数

int main(int argc, char *argv[]){
    // parse the command line instructions
    if (argc < 2){
        std::cerr << "Usage: dirhist <cmd> [[--dir=]<directory_path>]"
                  << "[<snapshot_file>] [<snapshot_file>] --[<option>]\n"
                  << "  cmd: snap| tree | log | diff" 
                  << std::endl;
        return -1;
    }

    std::string cmd = argv[1];
    if (cmd == "snap"){
        return process_snap(argc, argv);
    }
    else if (cmd == "tree"){
        return process_tree(argc, argv);
    }
    else if (cmd == "log"){
        return process_log(argc, argv);
    }
    else if (cmd == "diff") {
        return process_diff(argc, argv);
    }
    else if (cmd == "rm"){
        return process_rm(argc, argv);
    }
    else {
        std::cerr << "Unknown command: " << cmd << std::endl
                  << "Usage: dirhist <cmd> [[--dir=]<directory_path>]"
                  << "[<snapshot_file>] [<snapshot_file>] --[<option>]\n"
                  << "  cmd: snap| tree | log | diff" 
                  << std::endl;
        return -1;
    }
    return 0;
}

int process_snap(int argc, char *argv[]){
    // 处理 snap 逻辑
    if (argc != 3) {
        std::cerr << "Invaild instruction\n"
                    << "Usage: dirhist snap [--dir=]<directory_path>" 
                    << std::endl;
        return -1;
    }
    // dirhist snap --dir=<target_directory_path>
    // dirhist snap <target_directory_path>
    std::string path = argv[2];
    if (util::start_with_prefix(path, "--dir=")){
        path = path.substr(6);
    }
    std::unique_ptr<dirhist::Node> root = dirhist::build_tree(path);
    if(!root) return -1;

    dirhist::write_snapshot(*root, util::now_ms());
    return 0;
}

int process_tree(int argc, char *argv[]){
    // 处理 tree 逻辑，打印目录结构
    if (argc < 3) {
        std::cerr << "Invaild instruction" << std::endl
                << "Usage: dirhist tree [--dir=]<directory_path> [--max_depth=<n>]\n"
                << "     : dirhist tree <target_snapshot_file> [--max_depth=<n>]"
                << std::endl;
        return -1;
    }

    if (argc == 3) {
        std::string path = argv[2];
        std::unique_ptr<dirhist::Node> root;
        // dirhist tree <target_snapshot_file>
        if (util::is_snap_bin_file(path)) {
            root = dirhist::read_snapshot(path);
        }
        // dirhist tree --dir=<target_directory_path>
        // dirhist tree <target_directory_path>
        else {
            if (util::start_with_prefix(path, "--dir=")){
                path = path.substr(6);
            }
            root = dirhist::build_tree(path);
        }
        if (!root) return -1;
        dirhist::display_tree(root, -1);
    }
    else if (argc == 4) {
        // 确定 root 指针来源
        std::string path = argv[2];
        std::unique_ptr<dirhist::Node> root;
        // dirhist tree <target_snapshot_file> --max_depth=<n>
        if (util::is_snap_bin_file(path)) {
            root = dirhist::read_snapshot(path);
        }
        // dirhist tree <target_directory_path> --max_depth=<n>
        // dirhist tree --dir<target_directory_path> --max_depth=<n>
        else {
            if (util::start_with_prefix(path, "--dir=")){
                path = path.substr(6);
            }
            root = dirhist::build_tree(path);
        }
        if (!root) return -1;

        // 解析opt
        std::string opt = argv[3];
        int max_depth = -1;
        if (util::start_with_prefix(opt, "--max_depth=")) {
            std::string val = opt.substr(12);
            try {
                max_depth = std::stoi(val);
                dirhist::display_tree(root, max_depth);
            } catch (...) {
                std::cerr << "Invalid depth: " << val << '\n';
                return -1;
            }
        }
        else {
            std::cerr << "Unknown option: " << opt << std::endl 
                    << "Usage: dirhist tree [--dir=]<directory_path> [--max_depth=<n>]\n"
                    << "     : dirhist tree <target_snapshot_file> [--max_depth=<n>]"
                    << std::endl;
            return -1;
        }
    }
    else {
        std::cerr << "Usage: dirhist tree [--dir=]<directory_path> [--max_depth=<n>]\n"
                  << "     : dirhist tree <target_snapshot_file> [--max_depth=<n>]"
                  << std::endl;
        return -1; 
    }
    
    return 0;
}

int process_log(int argc, char *argv[]) {
    // 处理日志逻辑
    // dirhist log
    // 默认输出.dirhist目录
    if (argc == 2) {
        dirhist::list_snapshots();
    }
    else if (argc == 3) {
        std::string path_or_opt = argv[2];
        // dirhist log --num=<n>
        if (util::start_with_prefix(path_or_opt, "--num=")) {
            std::string val = path_or_opt.substr(6);
            try {
                int num = std::stoi(val);
                dirhist::list_snapshots(num);
            }
            catch(...) {
                std::cerr << "Invalid num: " << val << std::endl;
                return -1;
            }
        }
        // dirhist log --dir=<target_directory_path>
        // dirhist log <target_directory_path>
        else {
            if (util::start_with_prefix(path_or_opt, "--dir=")){
                path_or_opt = path_or_opt.substr(6);
            }
            dirhist::list_snapshots(-1, path_or_opt);
        }
    }
    // dirhist log --dir=<target_directory_path> --num=<n>
    // dirhist log <target_directory_path> --num=<n>
    else if (argc == 4) {
        std::string target_path = argv[2];
        if (util::start_with_prefix(target_path, "--dir=")){
            target_path = target_path.substr(6);
        }
        std::string opt = argv[3];
        int num = -1;
        if (util::start_with_prefix(opt, "--num=")) {
            std::string val = opt.substr(6);
            try {
                num = std::stoi(val);
                dirhist::list_snapshots(num, target_path);
            } catch (...) {
                std::cerr << "Invalid num: " << val << '\n';
                return -1;
            }
        }
        else {
            std::cerr << "Unknown option: " << opt << std::endl
                        << "Usage: dirhist log [--dir=]<target_directory_path> "
                        << "[--num=<n>]" << std::endl;
            return -1;
        }
    }
    else {
        std::cerr << "Invaild instruction\n"
                    << "Usage: dirhist log [--dir=]<target_directory_path> "
                    << "[--num=<n>]" << std::endl;
        return -1;
    }

    return 0;
}

int process_diff(int argc, char *argv[]) {
    // dirhist diff <old_snapshot_file>
    // 与默认目标文件夹(.dirhist)下的最新快照比较
    if (argc == 3) {
        std::string old_snap = argv[2];
        auto old_root = dirhist::read_snapshot(old_snap);
        auto new_root = dirhist::read_snapshot(dirhist::latest_snap(".dirhist"));
        dirhist::diff(*old_root, *new_root);
    }
    // dirhist diff <old_snapshot_file> <new_snapshot_file>
    // dirhist diff <old_snapshot_file> --dir=<target_directory_path>
    else if (argc == 4) {
        std::string old_snap = argv[2], file_dir = argv[3];
        std::unique_ptr<dirhist::Node> old_root = dirhist::read_snapshot(old_snap);
        std::unique_ptr<dirhist::Node> new_root;
        // dirhist diff <old_snapshot_file> --dir=<target_directory_path>
        if (util::start_with_prefix(file_dir, "--dir=")) {
            new_root = dirhist::read_snapshot(dirhist::latest_snap(file_dir));
        }
        // dirhist diff <old_snapshot_file> <new_snapshot_file>
        else {
            new_root = dirhist::read_snapshot(file_dir);
        }
        dirhist::diff(*old_root, *new_root);
    }
    else {
        std::cerr << "Invaild instruction\n"
                    << "Usage: dirhist diff <old_snapshot> [<new_snapshot>]\n" 
                    << "     : dirhist diff <old_snapshot> --dir=<snapshot_dir>" 
                    << std::endl;
        return -1;
    }
    return 0;
}

int process_rm(int argc, char *argv[]) {
    // dirhist rm
    // 默认删除 .dirhist下快照文件
    if (argc == 2) {
        std::cout << "clean all snapshots at: .dirhist" << std::endl;
        dirhist::clean_snapshots();
    }
    // dirhist rm --dir=<directory_path>
    else if (argc == 3) {
        std::string target_dir = argv[2];
        if (util::start_with_prefix(target_dir, "--dir=")){
            target_dir = target_dir.substr(6);
        }
        std::cout << "clean all snapshots at:" << target_dir << std::endl;
        dirhist::clean_snapshots(target_dir);
    }
    else {
        std::cerr << "Invaild instruction\n"
                    << "Usage: dirhist rm\n" 
                    << "     : dirhist rm [--dir=]<directory_path>" 
                    << std::endl;
        return -1;
    }

    return 0;
}
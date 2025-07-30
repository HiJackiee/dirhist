/*
 * @file    src/main.cpp
 * @brief   This source file implements the main logic of the dirhist.
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I./include -o src/display_tree src/main.cpp  src/snapshot.cpp src/util.cpp -lssl -lcrypto

#include <iostream>
#include <algorithm>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"
#include "dirhist/log.h"
#include "internal/util.h"

int main(int argc, char *argv[]){
    // parse the command line instructions
    if (argc < 2){
        std::cerr << "Usage: dirhist <cmd> <directory_path> --[<option>]\n" 
                  << "  cmd: snap| tree | log | diff | rm" 
                  << std::endl;
        return -1;
    }

    std::string cmd = argv[1];
    if (cmd == "snap"){
        // 处理 snap 逻辑
        if (argc != 3) {
            std::cerr << "Invaild instruction\n"
                      << "Usage: dirhist snap <directory_path>" 
                      << std::endl;
            return -1;
        }
        std::unique_ptr<dirhist::Node> root = dirhist::build_tree(argv[2]);
        dirhist::write_snapshot(*root, now_ms());
    }
    else if (cmd == "tree"){
        // 处理 tree 逻辑，打印目录结构
        if (argc < 3) {
            std::cerr << "Invaild instruction" << std::endl
                    << "Usage: dirhist tree <directory_path> [--max_depth=<n>]\n"
                    << "       dirhist tree <target_snapshot_file> [--max_depth=<n>]"
                    << std::endl;
            return -1;
        }

        if (argc == 3) {
            std::string path = argv[2];
            std::unique_ptr<dirhist::Node> root;
            // dirhist tree <target_snapshot_file>
            if (is_snap_bin_file(path)) {
                root = dirhist::read_snapshot(path);
            }
            // dirhist tree <directory_path>
            else {
                root = dirhist::build_tree(argv[2]);
            }
            dirhist::display_tree(root, -1);
        }
        else if (argc == 4) {
            // 确定 root 指针来源
            std::string path = argv[2];
            std::unique_ptr<dirhist::Node> root;
            // dirhist tree <target_snapshot_file> --depth=<n>
            if (is_snap_bin_file(path)) {
                root = dirhist::read_snapshot(path);
            }
            // dirhist tree <directory_path>--depth=<n>
            else {
                root = dirhist::build_tree(argv[2]);
            }

            // 解析opt
            std::string opt = argv[3];
            int max_depth = -1;
            if (opt.rfind("--max_depth", 0) == 0) {
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
                        << "Usage: dirhist tree <directory_path> [--max_depth=<n>]\n"
                        << "     : dirhist tree <target_snapshot_file> [--max_depth=<n>]"
                        << std::endl;
                return -1;
            }
        }
        
    }
    else if (cmd == "log"){
        // 处理日志逻辑
        // dirhist log
        if (argc == 2) {
            dirhist::list_snapshots();
        }
        else if (argc == 3) {
            std::string path_or_opt = argv[2];
            // dirhist log --num=<n>
            if (path_or_opt.rfind("--num", 0) == 0) {
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
            // dirhist log <target_directory_path>
            else {
                dirhist::list_snapshots(-1, path_or_opt);
            }
        }
        // dirhist log <target_directory_path> --num=<n>
        else if (argc == 4) {
            std::string target_path = argv[2];
            std::string opt = argv[3];
            int num = -1;
            if (opt.rfind("--num", 0) == 0) {
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
                          << "Usage: dirhist log [target_directory_path] [--num=<n>]" 
                          << std::endl;
                return -1;
            }
        }
        else {
            std::cerr << "Invaild instruction\n"
                      << "Usage: dirhist log [target_directory_path] [--num=<n>]" 
                      << std::endl;
            return -1;
        }
    }
    else if (cmd == "rm"){
        // dirhist rm，默认删除 .dirhist下快照文件
        if (argc == 2) {
            std::cout << "clean all snapshots at: .dirhist" << std::endl;
            dirhist::clean_snapshots();
        }
        // dirhist rm <directory_path>
        else if (argc == 3) {
            std::string target_dir = argv[2];
            std::cout << "clean all snapshots at:" << target_dir << std::endl;
            dirhist::clean_snapshots(target_dir);
        }
        else {
            std::cerr << "Invaild instruction\n"
                      << "Usage: dirhist rm\n" 
                      << "     : dirhist rm <directory_path>" 
                      << std::endl;
            return -1;
        }
    }
    else {
        std::cerr << "Unknown command: " << cmd << std::endl
                  << "Usage: dirhist <cmd> <directory_path> --[<option>]\n"
                  << "  cmd: snap| tree | log | diff" 
                  << std::endl;
        return -1;
    }
    return 0;
}

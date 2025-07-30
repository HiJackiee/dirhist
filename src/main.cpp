/*
 * @file    src/main.cpp
 * @brief   This source file implements the main logic of the dirhist.
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I./include -o src/display_tree src/main.cpp  src/snapshot.cpp src/util.cpp -lssl -lcrypto

#include <iostream>
#include <chrono>
#include <algorithm>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"

// @brief 返回当前时间戳，精确到毫秒
// @return 返回毫秒级时间戳
int64_t now_ms();

int main(int argc, char *argv[]){
    // parse the command line instructions
    if (argc < 3){
        std::cerr << "Usage: dirhist snap/tree <directory_path> --[<option>]" 
                  << std::endl;
        return -1;
    }

    std::string cmd = argv[1];
    if (cmd == "snap"){
        // 处理 snap 逻辑
        std::unique_ptr<dirhist::Node> root = dirhist::build_tree(argv[2]);
        dirhist::write_snapshot(*root, now_ms());
    }
    else if (cmd == "tree"){
        // 处理 tree 逻辑，打印目录结构
        std::unique_ptr<dirhist::Node> root = dirhist::build_tree(argv[2]);
        // 无 depth 参数
        if (argc == 3) {
            dirhist::display_tree(root, -1);
        }
        // 有 depth 参数
        else if (argc == 4) {
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
                std::cerr << "Unknown option: " << opt << std::endl;
                std::cerr << "Usage: dirhist tree <directory_path> [--max_depth=<n>]" 
                          << std::endl;
                return -1;
            }
        }
        
    }
    else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        std::cerr << "Usage: dirhist snap/tree <directory_path> [--<option>]" 
                  << std::endl;
        return -1;
    }
    return 0;
}

int64_t now_ms(){
    return static_cast<std::int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
}
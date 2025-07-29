/*
 * @file    src/main.cpp
 * @brief   This source file implements the main logic of the dirhist.
 * @author  yannn
 * @date    2025-07-28
 */

#include <iostream>
#include <chrono>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"

int main(int argc, char *argv[]){
    // parse the command line instructions
    if (argc < 3 || (std::string(argv[1]) != "snap")){
        std::cerr << "Usage: dirhist snap <directory_path>" << std::endl;
        return 1;
    }

    auto now = std::chrono::system_clock::now();  // 获取当前时间点
    auto duration = now.time_since_epoch();       // 从纪元时间开始的时间间隔
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    std::unique_ptr<Node> tree = dirhist::build_tree(argv[2]);
    dirhist::write_snapshot(*tree, timestamp.count(), ".dirhist");
    return 0;
}

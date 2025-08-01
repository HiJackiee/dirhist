/*
 * @file    src/main.cpp
 * @brief   This source file implements the main logic of the dirhist.
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I./include -o bin/dirhist src/main.cpp  src/snapshot.cpp src/serialize.cpp src/log.cpp src/diff.cpp src/util.cpp src/cli.cpp -lssl -lcrypto

#include <iostream>
#include <algorithm>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"
#include "dirhist/log.h"
#include "dirhist/diff.h"
#include "dirhist/cli.h"
#include "internal/util.h"

int main(int argc, char *argv[]){
    // parse the command line instructions
    if (argc < 2){
        std::cerr << "Usage: dirhist <cmd> [--options>]" << std::endl
                  << "  cmd: snap | tree | log | diff | rm" << std::endl;
        return -1;
    }

    std::string cmd = argv[1];
    if (cmd == "snap"){
        return dirhist::process_snap(argc, argv);
    }
    else if (cmd == "tree"){
        return dirhist::process_tree(argc, argv);
    }
    else if (cmd == "log"){
        return dirhist::process_log(argc, argv);
    }
    else if (cmd == "diff") {
        return dirhist::process_diff(argc, argv);
    }
    else if (cmd == "rm"){
        return dirhist::process_rm(argc, argv);
    }
    else {
        std::cerr << "Usage: dirhist <cmd> [--options>]" << std::endl
                  << "  cmd: snap | tree | log | diff | rm" << std::endl;
        return -1;
    }
    return 0;
}
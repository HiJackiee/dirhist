/*
 * @file    src/main.cpp
 * @brief   This source file implements the main logic of the dirhist.
 * @author  yannn
 * @date    2025-07-28
 */

#include <iostream>
#include "dirhist/snapshot.h"

int main(int argc, char *argv[]){
    // parse the command line instructions
    if (argc < 3 || (std::string(argv[1]) != "snap")){
        std::cerr << "Usage: dirhist snap <directory_path>" << std::endl;
        return 1;
    }

    return dirhist::create_snapshot(argv[2])? 0: 2;
    return 0;
}

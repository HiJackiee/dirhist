/*
 * @file    src/command.cpp
 * @brief   This source file implements the functions to parse command.
 * @author  yannn
 * @date    2025-07-28
 */

#include <iostream>
#include <algorithm>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"
#include "dirhist/log.h"
#include "dirhist/diff.h"
#include "dirhist/cli.h"
#include "internal/util.h"

namespace dirhist{
    bool check_vaild(const std::vector<std::string>& vaild_opts, const std::string& opt){
        // 检测是否存在于vaild_opts中
        if (std::find(vaild_opts.begin(), vaild_opts.end(), opt) != vaild_opts.end()){
            return true;
        }

        std::cerr << "Invaild option: " << opt << std::endl;
        return false;
    }

    Options parse_options(int argc, char* argv[]
            , const std::vector<std::string>& vaild_opts){
        Options opts;
        for (int i = 2; i < argc; ++i){
            std::string arg = argv[i];
            if (util::start_with_prefix(arg, "--dir=") 
                    && check_vaild(vaild_opts, "--dir")){
                opts.dir = arg.substr(6);
            }
            else if (util::start_with_prefix(arg, "--file=")
                    && check_vaild(vaild_opts, "--file")){
                opts.file = arg.substr(7);
            }
            else if (util::start_with_prefix(arg, "--old_snap=")
                    && check_vaild(vaild_opts, "--old_snap")){
                opts.old_snap = arg.substr(11);
            }
            else if (util::start_with_prefix(arg, "--new_snap=")
                    && check_vaild(vaild_opts, "--new_snap")){
                opts.old_snap = arg.substr(11);
            }
            else if (util::start_with_prefix(arg, "--max_depth=")
                    && check_vaild(vaild_opts, "--max_depth")){
                std::string val = arg.substr(12);
                try{
                    opts.max_depth = std::stoi(val);
                }
                catch(...){
                    std::cerr << "Invaild max_depth: " << val << std::endl;
                    opts.vaild_ins = false;
                }
            }
            else if (util::start_with_prefix(arg, "--num=")
                    && check_vaild(vaild_opts, "--num")){
                std::string val = arg.substr(6);
                try{
                    opts.num = std::stoi(val);
                }
                catch(...){
                    std::cerr << "Invaild num: " << val << std::endl;
                    opts.vaild_ins = false;
                }
            }
            else if (util::start_with_prefix(arg, "--all=")
                    && check_vaild(vaild_opts, "--all")){
                std::string val = arg.substr(6);
                if (val == "true") opts.all = true;
                else if (val == "false") opts.all = false;
                else {
                    std::cerr << "Invaild all<bool>: " << val << std::endl;
                    opts.vaild_ins = false;
                }
            }
            else if (util::start_with_prefix(arg, "--no=")
                    && check_vaild(vaild_opts, "--no")){
                std::string val = arg.substr(5);
                opts.no_list = util::split_by_comma(val);
            }
            else {
                std::cerr << "Unknown option: " << arg << std::endl;
                opts.vaild_ins = false;
            }
        }

        return opts;
    }

    int process_snap(int argc, char* argv[]){
        // dirhist snap --dir=<target_directory_path>
        if (argc < 3){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist snap --dir=<target_directory_path>"
            << std::endl;
            return -1;
        }
        std::vector<std::string> vaild_opts = {"--dir"};
        Options opts = parse_options(argc, argv, vaild_opts);

        if (!opts.vaild_ins) {
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist snap --dir=<target_directory_path>"
            << std::endl;
            return -1;
        }

        std::unique_ptr<dirhist::Node> root = dirhist::build_tree(opts.dir.value());
        if(!root) return -1;

        dirhist::write_snapshot(*root, util::now_ms());
        return 0;
    }

    int process_tree(int argc, char* argv[]){
        // dirhist tree --file=<target_snapfile_path> [--max_depth=<n>] [--all=<bool>] [--no=<csv_paths>]
        // dirhist tree --dir=<target_directory_path> [--max_depth=<n>] [--all=<bool>] [--no=<csv_paths>]
        if (argc < 3){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist tree --file=<target_snapfile_path> [--options]\n"
                      << "     : dirhist tree --dir=<target_directory_path> [--options]"
                      << std::endl;
            std::cerr << "Options: [--max_depth=<n>] [--all=<bool>] [--no=<csv_paths>]"
                      << std::endl;
            return -1;
        }

        std::vector<std::string> vaild_opts = {"--dir", "--file", "--max_depth",
                                               "--all", "--no"};
        Options opts = parse_options(argc, argv, vaild_opts);
        
        if (!opts.vaild_ins || (opts.file.has_value() && opts.dir.has_value())
                            || (!opts.file.has_value() && !opts.dir.has_value())){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist tree --file=<target_snapfile_path> [--options]\n"
                      << "     : dirhist tree --dir=<target_directory_path> [--options]"
                      << std::endl;
            std::cerr << "Options: [--max_depth=<n>] [--all=<bool>] [--no=<csv_paths>]"
                      << std::endl;
            return -1;
        }

        // 确定目标
        std::unique_ptr<dirhist::Node> root;

        if (opts.file.has_value()){
            if (util::is_snap_bin_file(opts.file.value())){
                root = read_snapshot(opts.file.value());
            }
            else {
                std::cerr << "Not a snapshot file: " 
                          << opts.file.value().string() << std::endl;
                return -1;
            }
        }
        else {
            root = build_tree(opts.dir.value());
        }

        // 基于选项调用
        int m_depth = opts.max_depth.has_value()? opts.max_depth.value(): -1;
        bool is_all = opts.all.has_value()? opts.all.value(): false;
        display_tree(root, m_depth, is_all, opts.no_list);
        return 0;
    }

    int process_log(int argc, char* argv[]){
        // dirhist log [--dir=<target_directory_path>] [--num=<n>]
        if (argc < 2){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist log [--options]"
                      << std::endl;
            std::cerr << "Options: [--dir=<target_directory_path>] [--num=<n>]" 
                      << std::endl;
            return -1;
        }

        std::vector<std::string> vaild_opts = {"--dir", "--num"};
        Options opts = parse_options(argc, argv, vaild_opts);

        if (!opts.vaild_ins){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist log [--options]"
                      << std::endl;
            std::cerr << "Options: [--dir=<target_directory_path>] [--num=<n>]" 
                      << std::endl;
            return -1;
        }

        fs::path target = opts.dir.has_value()? opts.dir.value(): ".dirhist";
        int n = opts.num.has_value()? opts.num.value(): -1;
        
        list_snapshots(n, target);
        return 0;
    }

    int process_diff(int argc, char* argv[]){
        // dirhist diff --old_snap=<old_snapshot_file> [--new=<new_snapshot_file>] 
        //                                         [--dir=<target_directory_path>]
        if (argc < 3){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist diff --old_snap=<old_snapshot_file> [--options]" 
                      << std::endl;
            std::cerr << "Options: [--new=<new_snapshot_file>] [--dir=<target_directory_path>]" 
                      << std::endl;
            return -1;
        }

        std::vector<std::string> vaild_opts = {"--dir", "--old_snap", "--new_snap"};
        Options opts = parse_options(argc, argv, vaild_opts);

        if (!opts.vaild_ins || !opts.old_snap.has_value()){
            std::cerr << "Invaild instruction" << std::endl;
            std::cerr << "Usage: dirhist diff --old_snap=<old_snapshot_file> [--options]" 
                      << std::endl;
            std::cerr << "Options: [--new=<new_snapshot_file>] [--dir=<target_directory_path>]" 
                      << std::endl;
            return -1;
        }
        
        fs::path target_dir = opts.dir.has_value()? opts.dir.value(): ".dirhist";
        fs::path new_snap = opts.new_snap.has_value()? 
                                opts.new_snap.value(): dirhist::latest_snap(".dirhist");

        auto old_root = dirhist::read_snapshot(opts.old_snap.value());
        auto new_root = dirhist::read_snapshot(new_snap);

        dirhist::diff(*old_root, *new_root);
        return 0;
    }

    int process_rm(int argc, char* argv[]){
        // dirhist rm [--dir=<directory_path>]
        std::vector<std::string> vaild_opts = {"--dir"};
        Options opts = parse_options(argc, argv, vaild_opts);

        if (!opts.vaild_ins){
            std::cerr << "Invaild instruction\n"
                << "Usage: dirhist rm [--dir=<directory_path>]" << std::endl;
            return -1;
        }

        fs::path target_dir = opts.dir.has_value()? opts.dir.value(): ".dirhist";
        dirhist::clean_snapshots(target_dir);
        return 0;
    }
}
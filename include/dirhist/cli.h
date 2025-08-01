/* 
 * @file    include/dirhist/command.h
 * @author  yannn
 * @date    2025-07-28
 */ 

#include <iostream>
#include <algorithm>
#include <optional>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"

namespace dirhist{
    // @brief 定义命令行选项
    struct Options{
        std::optional<fs::path> dir;
        std::optional<fs::path> file;
        std::optional<fs::path> old_snap;
        std::optional<fs::path> new_snap;
        std::optional<int> max_depth;
        std::optional<int> num;
        std::optional<bool> all;
        std::vector<std::string> no_list;
        bool vaild_ins = true;
    };

    // @brief 检测命令选项是否合法
    // @param vaild_opts 合法选项集合
    // @param opt 待检查选项（形如 --dir、--num等）
    // @return 合法返回true，否则false
    bool check_vaild(const std::vector<std::string>& vaild_opts
                                                    , const std::string& opt);

    // @brief 基于传入的命令行参数解析命令行选项
    // @param argc 命令行参数数量
    // @param argv 命令行参数数组指针
    // @param vaild_opts 合法选项集合
    // @return 返回解析后的Options
    Options parse_options(int argc, char* argv[]
            , const std::vector<std::string>& vaild_opts);
    
    // @brief 处理snap命令逻辑
    // @param argc 命令行参数数量
    // @param argv 命令行参数数组指针
    int process_snap(int argc, char* argv[]);

    // @brief 处理tree命令逻辑
    // @param argc 命令行参数数量
    // @param argv 命令行参数数组指针
    int process_tree(int argc, char* argv[]);

    // @brief 处理log命令逻辑
    // @param argc 命令行参数数量
    // @param argv 命令行参数数组指针
    int process_log(int argc, char* argv[]);

    // @brief 处理diff命令逻辑
    // @param argc 命令行参数数量
    // @param argv 命令行参数数组指针
    int process_diff(int argc, char* argv[]);

    // @brief 处理rm命令逻辑
    // @param argc 命令行参数数量
    // @param argv 命令行参数数组指针
    int process_rm(int argc, char* argv[]);
}
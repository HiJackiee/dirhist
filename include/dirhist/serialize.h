/* 
 * @file    include/dirhist/serialize.h
 * @author  yannn
 * @date    2025-07-28
 */ 

#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include "dirhist/snapshot.h"

namespace dirhist {
    constexpr uint64_t MAGIC = 0x4448495354415040ULL;   // "DIRSTAP"
    constexpr uint8_t VERSION = 1; // 当前版本号

    // @brief 定义文件头部
    struct Header{
        uint64_t magic = MAGIC;     // 文件头标识
        uint8_t version = VERSION;  // 版本号
        int64_t timestamp = 0;      // 时间戳
        uint64_t root_offset = 0;   // 根节点偏移
        uint64_t data_size = 0;     // 除文件头外的数据大小
    };

    // @brief 写POD对象到文件
    // @param ofs 输出文件流
    // @param obj 待写入POD对象
    template<typename T>
    void write(std::ofstream& ofs, const T& obj){
        try {
            ofs.write(reinterpret_cast<const char*>(&obj), sizeof(obj));
        }
        catch (const std::ios_base::failure& e){
            std::cerr << "Error writing to file: " << e.what() << std::endl;
        }
    }

    // @brief 从文件读取POD对象
    // @param ifs 输入文件流
    // @param obj 待读取POD对象
    template<typename T>
    void read(std::ifstream& ifs, T& obj){
        try {
            ifs.read(reinterpret_cast<char*>(&obj), sizeof(obj));
        }
        catch (const std::ios_base::failure& e){
            std::cerr << "Error reading from file: " << e.what() << std::endl;
        }
    }

    // @brief dfs 序列化
    // @param ofs 输出文件流
    // @param node 待写入节点
    // @param offset 节点偏移
    void write_node(std::ofstream& ofs, const Node& node, uint64_t& offset);

    // @brief dfs 反序列化
    // @param ifs 输入文件流
    // @param offset 节点偏移
    // @return 返回读取到的节点指针
    std::unique_ptr<Node> read_node(std::ifstream& ifs, uint64_t& offset);

    // @brief 序列化目录树
    // @param root 目录树根节点指针
    // @param ts 时间戳
    void write_snapshot(const Node& root, int64_t ts
                                , const fs::path& output_dir = "./.dirhist");

    // @brief 反序列化目录树
    // @param ts 时间戳
    // @param input_dir 文件输入目录
    // @return 返回读取到的目录树根节点指针
    // @note 该函数读取指定时间戳的快照文件，并返回根节点指针
    std::unique_ptr<Node> read_snapshot(int64_t ts
                                , const fs::path& input_dir = "./.dirhist");
}
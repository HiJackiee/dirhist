/*
 * @file    src/serialize.cpp
 * @brief   This source file implements the functions from Merkle tree to disk files.
 * @author  yannn
 * @date    2025-07-28
 */

#include "dirhist/serialize.h"

namespace dirhist {
    void write_node(std::ofstream& ofs, const Node& node, uint64_t& offset){
        // 先将文件指针移动到 offset 处
        ofs.seekp(offset);
        uint64_t cur = offset;
        // 写入节点基本信息
        write(ofs, static_cast<uint32_t>(node.path.size()));
        // 非POD类型——string 不能直接调用 write
        ofs.write(node.path.data(), node.path.size());
        write(ofs, uint8_t(node.is_dir));
        write(ofs, uint8_t(node.is_symlink));
        write(ofs, node.size);                
        write(ofs, node.mtime);
        ofs.write(reinterpret_cast<const char*>(node.hash.data()), node.hash.size());

        // 处理子节点
        write(ofs, static_cast<uint32_t>(node.children.size()));
        // 记录子节点偏移量的起始位置
        uint64_t child_offsets_offset = ofs.tellp();
        // 预留记录子节点偏移量的空间
        ofs.seekp(child_offsets_offset
             + sizeof(uint64_t) * node.children.size(), std::ios::beg);
        
        cur = ofs.tellp();
        // 递归写入子节点
        std::vector<uint64_t> child_offsets;
        for (const auto& child: node.children){
            child_offsets.push_back(cur);
            write_node(ofs, *child, cur);
            cur = ofs.tellp();
        }
        // 更新 offset
        offset = cur;
        // 回填子节点偏移
        uint64_t back = ofs.tellp();
        ofs.seekp(child_offsets_offset, std::ios::beg);
        for (const uint64_t& child_offset: child_offsets) write(ofs, child_offset);
        ofs.seekp(back, std::ios::beg);
    }

    std::unique_ptr<Node> read_node(std::ifstream& ifs, uint64_t& offset){
        ifs.seekg(offset, std::ios::beg);
        std::unique_ptr<Node> node = std::make_unique<Node>();

        // path
        uint32_t path_len = 0;
        read(ifs, path_len);
        node->path.resize(path_len);
        ifs.read(node->path.data(), path_len);

        // is_dir, is_symlink
        uint8_t tmp = 0;
        read(ifs, tmp);
        node->is_dir = tmp != 0;
        read(ifs, tmp);
        node->is_symlink = tmp != 0;

        // size, mtime
        uint64_t sz = 0;
        read(ifs, sz);
        node->size = sz;
        int64_t mt = 0;
        read(ifs, mt);
        node->mtime = mt;

        // hash
        ifs.read(reinterpret_cast<char*>(node->hash.data()), 32);

        // children
        uint32_t children_cnt = 0;
        read(ifs, children_cnt);
        for (uint32_t i = 0; i < children_cnt; ++i) {
            uint64_t child_offset = 0;
            // 读取子节点偏移量，并递归处理
            read(ifs, child_offset);
            if (child_offset != 0) 
                node->children.push_back(read_node(ifs, child_offset));
        }
        return node;
    }

    void write_snapshot(const Node& root, int64_t ts, const fs::path& output_dir){
        // 设置输出目录及文件
        fs::create_directories(output_dir);
        std::cout << "Created output dir: " << output_dir.string() << std::endl;
        fs::path output_file = output_dir / ("snap-" + std::to_string(ts) + ".bin");

        std::ofstream ofs(output_file, std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Error opening output file: "
                                            + output_file.string());
        }

        // 设置文件头
        Header hdr;
        hdr.timestamp = ts;
        hdr.root_offset = sizeof(Header);

        uint64_t offset = hdr.root_offset;
        write_node(ofs, root, offset);

        hdr.data_size = offset - hdr.root_offset;
        ofs.seekp(0, std::ios::beg);
        // 写入文件头
        write(ofs, hdr);
    }

    std::unique_ptr<Node> read_snapshot(int64_t ts, const fs::path& input_dir){
        // 设置输入文件路径
        fs::path input_file = input_dir / ("snap-" + std::to_string(ts) + ".bin");

        std::ifstream ifs(input_file, std::ios::binary);
        if (!ifs){
            throw std::runtime_error("Error opening input file: " 
                                            + input_file.string());
        }
        
        // 读取文件头，并作格式检查
        Header hdr;
        read(ifs, hdr);
        if (hdr.magic != MAGIC || hdr.version != VERSION){
            std::cerr << "Header.magic: " << hdr.magic << std::endl
                      << "Header.version: " << hdr.version << std::endl;
            throw std::runtime_error("Invaild snapshot format");
        }
        
        return read_node(ifs, hdr.root_offset);
    }
}
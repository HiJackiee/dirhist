/*
 * @file    src/serialize.cpp
 * @brief   This source file implements the functions from Merkle tree to disk files.
 * @author  yannn
 * @date    2025-07-28
 */

#include "dirhist/serialize.h"
#include "internal/util.h"

namespace dirhist {
    void write_node(std::ofstream& ofs, const Node& node, uint64_t& offset) {
        // 先将文件指针移动到 offset 处
        ofs.seekp(offset);

        // 写入节点基本信息
        write(ofs, static_cast<uint32_t>(node.path.size()));
        ofs.write(node.path.data(), node.path.size());
        write(ofs, static_cast<uint32_t>(node.abs_root.size()));
        ofs.write(node.abs_root.data(), node.abs_root.size());
        write(ofs, uint8_t(node.is_dir));
        write(ofs, uint8_t(node.is_symlink));
        write(ofs, node.size);
        write(ofs, node.mtime);
        ofs.write(reinterpret_cast<const char*>(node.hash.data()), node.hash.size());

        // 处理子节点
        write(ofs, static_cast<uint32_t>(node.children.size()));
        uint64_t child_offsets_offset = ofs.tellp();  // 记录子节点偏移量的起始位置

        // 预留记录子节点偏移量的空间
        ofs.seekp(child_offsets_offset
                 + sizeof(uint64_t) * node.children.size(), std::ios::beg);

        // 递归写入子节点
        std::vector<uint64_t> child_offsets;
        for (const auto& child : node.children) {
            offset = ofs.tellp();  // 更新当前偏移量
            child_offsets.push_back(offset);
            write_node(ofs, *child, offset);
        }

        // 回填子节点偏移量
        uint64_t back = ofs.tellp();  // 保存当前文件指针位置
        ofs.seekp(child_offsets_offset, std::ios::beg);  // 移动到子节点偏移量的起始位置
        for (const uint64_t& child_offset : child_offsets) {
            write(ofs, child_offset);
        }
        ofs.seekp(back, std::ios::beg);  // 回到之前的位置
    }

    std::unique_ptr<Node> read_node(std::ifstream& ifs, uint64_t& offset) {
        ifs.seekg(offset);

        auto node = std::make_unique<Node>();

        // 读节点头部
        uint32_t len;
        read(ifs, len);
        node->path.resize(len);
        ifs.read(node->path.data(), len);

        read(ifs, len);
        node->abs_root.resize(len);
        ifs.read(node->abs_root.data(), len);

        uint8_t flag;
        read(ifs, flag); node->is_dir = flag != 0;
        read(ifs, flag); node->is_symlink = flag != 0;

        read(ifs, node->size);
        read(ifs, node->mtime);
        ifs.read(reinterpret_cast<char*>(node->hash.data()), 32);

        // 读子节点偏移量
        uint32_t cnt;
        read(ifs, cnt);

        if (cnt == 0) return node;// 无子节点，直接返回

        // 一次性读出所有子节点偏移量
        std::vector<uint64_t> offsets(cnt);
        ifs.read(reinterpret_cast<char*>(offsets.data()), cnt * sizeof(uint64_t));

        // 递归读取子节点
        node->children.reserve(cnt);
        for (uint64_t child_offset : offsets) {
            if (child_offset != 0)
                node->children.emplace_back(read_node(ifs, child_offset));
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

    std::unique_ptr<Node> read_snapshot(const fs::path& snapshot) {
        std::ifstream ifs(snapshot, std::ios::binary);
        if (!ifs){
            throw std::runtime_error("Error opening input file: " 
                                                + snapshot.string());
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

    void clean_snapshots(const fs::path& target_dir){
        std::error_code ec;
        if (!std::filesystem::exists(target_dir, ec)) {
            std::cerr << "Directory does not exist: " << target_dir << '\n';
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(target_dir, ec)) {
            if (ec) continue;
            if (util::is_snap_bin_file(entry.path())) {
                std::filesystem::remove(entry.path(), ec);
                if (!ec) {
                    std::cout << "Removed: " << entry.path().filename() << '\n';
                } else {
                    std::cerr << "Failed to remove: " << entry.path() << '\n';
                }
            }
        }
        std::cout << "Clean done." << std::endl;
    }
}
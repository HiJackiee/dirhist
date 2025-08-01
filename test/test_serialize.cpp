/*
 * @file    test/test_serialize.cpp
 * @brief   This source file implemented to test the functions in src/serialize.cpp
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I/usr/local/googletest/include -I./include -o test/test_serialize test/test_serialize.cpp src/serialize.cpp  src/snapshot.cpp src/util.cpp -lgtest -lgtest_main -lpthread -lssl -lcrypto
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <cstdio>
#include "dirhist/snapshot.h"
#include "dirhist/serialize.h"

// 辅助函数：递归删除目录
void aux_remove_all(const std::filesystem::path& p) {
    std::error_code ec;
    std::filesystem::remove_all(p, ec);
}

// 辅助函数：创建测试文件并写入内容
void create_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::binary);
    ofs << content;
}

// 辅助函数：查找Node树中的某个节点
const dirhist::Node* find_node(const dirhist::Node* root, const std::string& rel_path) {
    if (!root) return nullptr;
    if (root->path == rel_path) return root;
    for (const auto& child : root->children) {
        if (const dirhist::Node* found = find_node(child.get(), rel_path)) return found;
    }
    return nullptr;
}

class SerializeTest : public ::testing::Test {
protected:
    std::filesystem::path test_dir;
    std::filesystem::path output_dir;

    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "dirhist_serialize_test_dir";
        output_dir = std::filesystem::temp_directory_path() / "dirhist_serialize_output";
        aux_remove_all(test_dir);
        aux_remove_all(output_dir);
        std::filesystem::create_directory(test_dir);
        std::filesystem::create_directory(output_dir);
    }

    void TearDown() override {
        aux_remove_all(test_dir);
        aux_remove_all(output_dir);
    }
};

// 测试快照的序列化和反序列化（空目录）
TEST_F(SerializeTest, SerializeAndDeserializeEmptyDir) {
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    int64_t ts = 123456789;
    dirhist::write_snapshot(*root, ts, output_dir);

    auto loaded = dirhist::read_snapshot(ts, output_dir);
    ASSERT_NE(loaded, nullptr);
    std::cout << loaded->path << std::endl;
    EXPECT_EQ(loaded->path, ".");
    EXPECT_TRUE(loaded->is_dir);
    EXPECT_EQ(loaded->children.size(), 0);
}

// 测试快照的序列化和反序列化（包含文件和子目录）
TEST_F(SerializeTest, SerializeAndDeserializeWithFilesAndDirs) {
    create_file(test_dir / "file1.txt", "hello");
    std::filesystem::create_directory(test_dir / "sub");
    create_file(test_dir / "sub" / "file2.txt", "world");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    int64_t ts = 987654321;
    dirhist::write_snapshot(*root, ts, output_dir);

    auto loaded = dirhist::read_snapshot(ts, output_dir);
    ASSERT_NE(loaded, nullptr);

    // 检查file1.txt
    const dirhist::Node* file1 = find_node(loaded.get(), "file1.txt");
    ASSERT_NE(file1, nullptr);
    EXPECT_FALSE(file1->is_dir);
    EXPECT_EQ(file1->size, 5);

    // 检查sub目录
    const dirhist::Node* sub = find_node(loaded.get(), "sub");
    ASSERT_NE(sub, nullptr);
    EXPECT_TRUE(sub->is_dir);

    // 检查sub/file2.txt
    const dirhist::Node* file2 = find_node(loaded.get(), "sub/file2.txt");
    ASSERT_NE(file2, nullptr);
    EXPECT_FALSE(file2->is_dir);
    EXPECT_EQ(file2->size, 5);
}

// 测试快照的序列化和反序列化（目录中存在多个文件）
TEST_F(SerializeTest, SerializeAndDeserializeMultipleFiles) {
    // 创建多个文件
    create_file(test_dir / "a.txt", "aaa");
    create_file(test_dir / "b.txt", "bbb");
    create_file(test_dir / "c.txt", "ccc");
    create_file(test_dir / "d.txt", "ddd");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    int64_t ts = 20250801;
    dirhist::write_snapshot(*root, ts, output_dir);

    auto loaded = dirhist::read_snapshot(ts, output_dir);
    ASSERT_NE(loaded, nullptr);

    // 检查所有文件
    const dirhist::Node* a = find_node(loaded.get(), "a.txt");
    ASSERT_NE(a, nullptr);
    EXPECT_FALSE(a->is_dir);
    EXPECT_EQ(a->size, 3);

    const dirhist::Node* b = find_node(loaded.get(), "b.txt");
    ASSERT_NE(b, nullptr);
    EXPECT_FALSE(b->is_dir);
    EXPECT_EQ(b->size, 3);

    const dirhist::Node* c = find_node(loaded.get(), "c.txt");
    ASSERT_NE(c, nullptr);
    EXPECT_FALSE(c->is_dir);
    EXPECT_EQ(c->size, 3);

    const dirhist::Node* d = find_node(loaded.get(), "d.txt");
    ASSERT_NE(d, nullptr);
    EXPECT_FALSE(d->is_dir);
    EXPECT_EQ(d->size, 3);

    // 检查根节点下有4个子节点
    EXPECT_EQ(loaded->children.size(), 4);
}

// 测试快照的序列化和反序列化（包含符号链接）
TEST_F(SerializeTest, SerializeAndDeserializeWithSymlink) {
    create_file(test_dir / "target.txt", "target");
    std::filesystem::create_symlink("target.txt", test_dir / "link.txt");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    int64_t ts = 20250729;
    dirhist::write_snapshot(*root, ts, output_dir);

    auto loaded = dirhist::read_snapshot(ts, output_dir);
    ASSERT_NE(loaded, nullptr);

    const dirhist::Node* link = find_node(loaded.get(), "link.txt");
    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_symlink);
    EXPECT_FALSE(link->is_dir);
    EXPECT_EQ(link->size, std::string("target.txt").size());
}

// 测试快照的序列化和反序列化（多层嵌套目录）
TEST_F(SerializeTest, SerializeAndDeserializeNestedDirs) {
    std::filesystem::create_directories(test_dir / "a/b/c");
    create_file(test_dir / "a/b/c/file.txt", "abc");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    int64_t ts = 20250730;
    dirhist::write_snapshot(*root, ts, output_dir);

    auto loaded = dirhist::read_snapshot(ts, output_dir);
    ASSERT_NE(loaded, nullptr);

    const dirhist::Node* file = find_node(loaded.get(), "a/b/c/file.txt");
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(file->size, 3);
    EXPECT_FALSE(file->is_dir);
}

// 测试快照的序列化和反序列化（空文件）
TEST_F(SerializeTest, SerializeAndDeserializeEmptyFile) {
    create_file(test_dir / "empty.txt", "");
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    int64_t ts = 20250731;
    dirhist::write_snapshot(*root, ts, output_dir);

    auto loaded = dirhist::read_snapshot(ts, output_dir);
    ASSERT_NE(loaded, nullptr);

    const dirhist::Node* empty = find_node(loaded.get(), "empty.txt");
    ASSERT_NE(empty, nullptr);
    EXPECT_EQ(empty->size, 0);
}

// 测试反序列化不存在的快照文件
TEST_F(SerializeTest, DeserializeNonExistentSnapshot) {
    int64_t ts = 99999999;
    EXPECT_THROW({
        dirhist::read_snapshot(ts, output_dir);
    }, std::runtime_error);
}
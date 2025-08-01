/*
 * @file    test/test_diff.cpp
 * @brief   This source file implemented to test the functions in src/diff.cpp
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I/usr/local/googletest/include -I./include -o test/test_diff test/test_diff.cpp src/serialize.cpp  src/snapshot.cpp src/diff.cpp src/util.cpp -lgtest -lgtest_main -lpthread -lssl -lcrypto

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include "dirhist/snapshot.h"
#include "dirhist/diff.h"

namespace fs = std::filesystem;

// 辅助函数：递归删除目录
void aux_remove_all(const fs::path& p) {
    std::error_code ec;
    fs::remove_all(p, ec);
}

// 辅助函数：创建测试文件并写入内容
void create_file(const fs::path& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::binary);
    ofs << content;
}

class DiffFuncRealTreeTest : public ::testing::Test {
protected:
    fs::path test_dir;

    void SetUp() override {
        test_dir = fs::temp_directory_path() / "dirhist_diff_func_test";
        aux_remove_all(test_dir);
        fs::create_directory(test_dir);
    }
    void TearDown() override {
        aux_remove_all(test_dir);
    }
};

// diff_nodes 测试
TEST_F(DiffFuncRealTreeTest, DiffNodes_AddedFile) {
    auto old_root = dirhist::build_tree(test_dir);
    create_file(test_dir / "a.txt", "hello");
    auto new_root = dirhist::build_tree(test_dir);

    std::vector<dirhist::DiffEntry> out;
    dirhist::diff_nodes(*old_root, *new_root, out);

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].type, dirhist::ChangeType::Added);
    EXPECT_EQ(out[0].path, "a.txt");
}

TEST_F(DiffFuncRealTreeTest, DiffNodes_DeletedFile) {
    create_file(test_dir / "b.txt", "world");
    auto old_root = dirhist::build_tree(test_dir);

    fs::remove(test_dir / "b.txt");
    auto new_root = dirhist::build_tree(test_dir);

    std::vector<dirhist::DiffEntry> out;
    dirhist::diff_nodes(*old_root, *new_root, out);

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].type, dirhist::ChangeType::Deleted);
    EXPECT_EQ(out[0].path, "b.txt");
}

TEST_F(DiffFuncRealTreeTest, DiffNodes_ModifiedFile) {
    create_file(test_dir / "c.txt", "old");
    auto old_root = dirhist::build_tree(test_dir);

    create_file(test_dir / "c.txt", "new content");
    auto new_root = dirhist::build_tree(test_dir);

    std::vector<dirhist::DiffEntry> out;
    dirhist::diff_nodes(*old_root, *new_root, out);

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].type, dirhist::ChangeType::Modified);
    EXPECT_EQ(out[0].path, "c.txt");
}

TEST_F(DiffFuncRealTreeTest, DiffNodes_NoChange) {
    create_file(test_dir / "d.txt", "same");
    auto old_root = dirhist::build_tree(test_dir);
    auto new_root = dirhist::build_tree(test_dir);

    std::vector<dirhist::DiffEntry> out;
    dirhist::diff_nodes(*old_root, *new_root, out);

    EXPECT_TRUE(out.empty());
}

// mark_subtree 测试（通过真实目录树间接测试）
TEST_F(DiffFuncRealTreeTest, MarkSubtree_AddedDirWithFiles) {
    auto old_root = dirhist::build_tree(test_dir);

    fs::create_directory(test_dir / "sub");
    create_file(test_dir / "sub" / "f.txt", "abc");
    auto new_root = dirhist::build_tree(test_dir);

    std::vector<dirhist::DiffEntry> out;
    dirhist::diff_nodes(*old_root, *new_root, out);

    // 应该有两个：sub 和 sub/f.txt
    ASSERT_EQ(out.size(), 2);
    EXPECT_EQ(out[0].type, dirhist::ChangeType::Added);
    EXPECT_EQ(out[1].type, dirhist::ChangeType::Added);
}

// print_colored_DiffEntry 测试
TEST_F(DiffFuncRealTreeTest, PrintColoredDiffEntry_Output) {
    create_file(test_dir / "e.txt", "1");
    auto old_root = dirhist::build_tree(test_dir);
    create_file(test_dir / "e.txt", "2");
    auto new_root = dirhist::build_tree(test_dir);

    std::vector<dirhist::DiffEntry> out;
    dirhist::diff_nodes(*old_root, *new_root, out);

    std::ostringstream oss;
    std::streambuf* old_cout = std::cout.rdbuf(oss.rdbuf());
    for (const auto& entry : out) {
        print_colored_DiffEntry(entry);
    }
    std::cout.rdbuf(old_cout);

    std::string output = oss.str();
    EXPECT_NE(output.find("M "), std::string::npos);
    EXPECT_NE(output.find("e.txt"), std::string::npos);
}

// diff 函数整体输出测试
TEST_F(DiffFuncRealTreeTest, DiffFunction_Output) {
    create_file(test_dir / "f.txt", "old");
    auto old_root = dirhist::build_tree(test_dir);
    create_file(test_dir / "f.txt", "new");
    auto new_root = dirhist::build_tree(test_dir);

    std::ostringstream oss;
    std::streambuf* old_cout = std::cout.rdbuf(oss.rdbuf());
    diff(*old_root, *new_root);
    std::cout.rdbuf(old_cout);

    std::string output = oss.str();
    EXPECT_NE(output.find("Changes between snapshots"), std::string::npos);
    EXPECT_NE(output.find("M "), std::string::npos);
    EXPECT_NE(output.find("f.txt"), std::string::npos);
}
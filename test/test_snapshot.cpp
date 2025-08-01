/*
 * @file    test/test_snapshot.cpp
 * @brief   This source file implemented to test the functions in src/snapshot.cpp
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I/usr/local/googletest/include -I./include -o test/test_snapshot test/test_snapshot.cpp src/snapshot.cpp src/util.cpp -lgtest -lgtest_main -lpthread -lssl -lcrypto

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include "dirhist/snapshot.h"

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

class SnapshotTest : public ::testing::Test {
protected:
    std::filesystem::path test_dir;
    std::string abs_test_dir;

    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "dirhist_test_dir";
        abs_test_dir = fs::absolute(test_dir).string();
        aux_remove_all(test_dir);
        std::filesystem::create_directory(test_dir);
    }

    void TearDown() override {
        aux_remove_all(test_dir);
    }
};

// 空目录快照测试
TEST_F(SnapshotTest, BuildTreeOnEmptyDir) {
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->path, ".");
    EXPECT_EQ(root->abs_root, abs_test_dir);
    EXPECT_TRUE(root->is_dir);
    EXPECT_EQ(root->children.size(), 0);
}

// 包含文件和子目录的快照测试
TEST_F(SnapshotTest, BuildTreeWithFilesAndDirs) {
    // test_dir/
    //   file1.txt
    //   sub/
    //     file2.txt
    create_file(test_dir / "file1.txt", "hello");
    std::filesystem::create_directory(test_dir / "sub");
    create_file(test_dir / "sub" / "file2.txt", "world");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(root->is_dir);

    // 检查file1.txt节点
    const dirhist::Node* file1 = find_node(root.get(), "file1.txt");
    ASSERT_NE(file1, nullptr);
    EXPECT_FALSE(file1->is_dir);
    EXPECT_EQ(file1->size, 5);

    // 检查sub目录节点
    const dirhist::Node* sub = find_node(root.get(), "sub");
    ASSERT_NE(sub, nullptr);
    EXPECT_TRUE(sub->is_dir);

    // 检查sub/file2.txt节点
    const dirhist::Node* file2 = find_node(root.get(), "sub/file2.txt");
    ASSERT_NE(file2, nullptr);
    EXPECT_FALSE(file2->is_dir);
    EXPECT_EQ(file2->size, 5);
}

// 根目录不存在时的快照测试
TEST_F(SnapshotTest, BuildTreeNonExistentRoot) {
    auto non_exist = test_dir / "not_exist";
    auto root = dirhist::build_tree(non_exist);
    EXPECT_EQ(root, nullptr);
}

// 多层嵌套目录快照测试
TEST_F(SnapshotTest, BuildTreeWithNestedDirs) {
    std::filesystem::create_directories(test_dir / "a/b/c");
    create_file(test_dir / "a/b/c/file.txt", "abc");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    const dirhist::Node* file = find_node(root.get(), "a/b/c/file.txt");
    ASSERT_NE(file, nullptr);
    EXPECT_EQ(file->size, 3);
    EXPECT_FALSE(file->is_dir);
}

// 空文件快照测试
TEST_F(SnapshotTest, BuildTreeWithEmptyFile) {
    create_file(test_dir / "empty.txt", "");
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    const dirhist::Node* empty = find_node(root.get(), "empty.txt");
    ASSERT_NE(empty, nullptr);
    EXPECT_EQ(empty->size, 0);
}

// 符号链接指向文件的快照测试
TEST_F(SnapshotTest, SymlinkToFile) {
    create_file(test_dir / "target.txt", "target");
    std::filesystem::create_symlink("target.txt", test_dir / "link.txt");

    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    const dirhist::Node* link = find_node(root.get(), "link.txt");
    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_symlink);
    EXPECT_FALSE(link->is_dir);
    EXPECT_GT(link->size, 0);
}

// 符号链接指向目录的快照测试
TEST_F(SnapshotTest, SymlinkToDirectory) {
    std::filesystem::create_directory(test_dir / "real_dir");
    std::filesystem::create_symlink("real_dir", test_dir / "dir_link");
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    const dirhist::Node* link = find_node(root.get(), "dir_link");
    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_symlink);
    EXPECT_TRUE(link->is_dir);  // 符号链接的目标也是目录，故应为true
    EXPECT_EQ(link->size, std::string("real_dir").size());
}

// 符号链接指向不存在目标的快照测试
TEST_F(SnapshotTest, SymlinkToNonExistentTarget) {
    std::filesystem::create_symlink("not_exist_target", test_dir / "broken_link");
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    const dirhist::Node* link = find_node(root.get(), "broken_link");
    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_symlink);
    EXPECT_FALSE(link->is_dir);
    EXPECT_EQ(link->size, std::string("not_exist_target").size());
}

// 符号链接形成循环的快照测试
TEST_F(SnapshotTest, SymlinkLoop) {
    std::filesystem::create_directory(test_dir / "loopdir");
    std::filesystem::create_symlink("../loopdir", test_dir / "loopdir/loop");
    auto root = dirhist::build_tree(test_dir);
    ASSERT_NE(root, nullptr);

    const dirhist::Node* link = find_node(root.get(), "loopdir/loop");
    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_symlink);
    EXPECT_TRUE(link->is_dir);
    EXPECT_EQ(link->size, std::string("../loopdir").size());
}
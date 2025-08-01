/*
 * @file    test/test_util.cpp
 * @brief   This source file implemented to test the functions in src/util.cpp
 * @author  yannn
 * @date    2025-07-28
 */
// g++ -std=c++17 -I/usr/local/googletest/include -I./src -o test/test_util test/test_util.cpp src/util.cpp -lgtest -lgtest_main -lpthread -lssl -lcrypto
#include <gtest/gtest.h>
#include <array>
#include <string>
#include <fstream>
#include <filesystem>
#include "internal/util.h"

namespace fs = std::filesystem;

// 辅助函数：将哈希结果转为十六进制字符串
std::string to_hex(const std::array<uint8_t, 32>& hash) {
    static const char hex_digits[] = "0123456789abcdef";
    std::string hex;
    for (auto b : hash) {
        hex += hex_digits[(b >> 4) & 0xF];
        hex += hex_digits[b & 0xF];
    }
    return hex;
}

TEST(UtilTest, Sha256EmptyString) {
    auto hash = util::sha256("");
    // SHA-256("") 的标准哈希值
    EXPECT_EQ(
        to_hex(hash),
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    );
}

TEST(UtilTest, Sha256KnownString) {
    auto hash = util::sha256("abc");
    // SHA-256("abc") 的标准哈希值
    EXPECT_EQ(
        to_hex(hash),
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
    );
}


TEST(UtilTest, Sha256DifferentInputs) {
    auto hash1 = util::sha256("abc");
    auto hash2 = util::sha256("abcd");
    EXPECT_NE(hash1, hash2);
}

TEST(UtilTest, HashToStrCorrectness) {
    // 构造一个已知内容的哈希数组
    std::array<uint8_t, 32> hash = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x00, 0xff, 0x11, 0xee, 0x22, 0xdd, 0x33, 0xcc,
        0x44, 0xbb, 0x55, 0xaa, 0x66, 0x99, 0x77, 0x88
    };
    std::string str = util::hash_to_str(hash);
    // 检查长度
    EXPECT_EQ(str.size(), 32);
    // 检查内容
    for (size_t i = 0; i < hash.size(); ++i) {
        EXPECT_EQ(static_cast<uint8_t>(str[i]), hash[i]);
    }
}

TEST(UtilTest, NowMsMonotonicity) {
    int64_t t1 = util::now_ms();
    int64_t t2 = util::now_ms();
    EXPECT_LE(t1, t2); // t2 应该大于等于 t1
}

TEST(UtilTest, IsSnapBinFile) {
    // 创建临时文件名
    fs::path valid = "snap-123456.bin";
    fs::path invalid1 = "snap-123456.txt";
    fs::path invalid2 = "foo-123456.bin";
    fs::path invalid3 = "snap-.bin";
    fs::path invalid4 = "snap-1.bin.bak";

    // 创建一个实际的普通文件用于测试
    std::ofstream(valid).close();

    EXPECT_TRUE(util::is_snap_bin_file(valid));
    EXPECT_FALSE(util::is_snap_bin_file(invalid1));
    EXPECT_FALSE(util::is_snap_bin_file(invalid2));
    EXPECT_FALSE(util::is_snap_bin_file(invalid3));
    EXPECT_FALSE(util::is_snap_bin_file(invalid4));

    // 清理
    fs::remove(valid);
}

TEST(UtilTest, TsStrFormat) {
    // 2025-07-28 12:34:56 UTC+8
    int64_t ts = 1753686896000; // 2025-07-28 12:34:56 UTC+8
    std::string s = util::ts_str(ts);
    // 只检查格式，不检查具体时区
    EXPECT_EQ(s.size(), 19);
    EXPECT_EQ(s[4], '-');
    EXPECT_EQ(s[7], '-');
    EXPECT_EQ(s[10], ' ');
    EXPECT_EQ(s[13], ':');
    EXPECT_EQ(s[16], ':');
}
### 依赖
&ensp;&ensp;运行测试文件需要提前安装好googletest等依赖库。

### 编译测试文件
&ensp;&ensp;在终端中运行一下命令对测试文件进行编译：
```bash
g++ -std=c++17 -I/usr/local/googletest/include -I./include -o test/test_util test/test_util.cpp src/util.cpp -lgtest -lgtest_main -lpthread -lssl -lcrypto
```
> 注意：为了使用 `-lgtest` 及 `-lgtest_main` 选项链接依赖库，需要提前设置好环境变量 `LIBRARY_PATH`，如临时设置：
> `export LIBRARY_PATH=/usr/local/googletest/lib:$LIBRARY_PATH`

&ensp;&ensp;运行成功后会生成一个名为 `test_util` 的可执行文件。

### 运行测试
&ensp;&ensp;在终端中运行上述可执行文件：
```bash
./test/test_util
```
&ensp;&ensp;若测试通过，则会输出类似一下内容：
```
Running main() from /home/yannn/googletest/googletest/src/gtest_main.cc
[==========] Running 3 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 3 tests from UtilTest
[ RUN      ] UtilTest.Sha256EmptyString
[       OK ] UtilTest.Sha256EmptyString (3 ms)
[ RUN      ] UtilTest.Sha256KnownString
[       OK ] UtilTest.Sha256KnownString (0 ms)
[ RUN      ] UtilTest.Sha256DifferentInputs
[       OK ] UtilTest.Sha256DifferentInputs (0 ms)
[----------] 3 tests from UtilTest (3 ms total)

[----------] Global test environment tear-down
[==========] 3 tests from 1 test suite ran. (4 ms total)
[  PASSED  ] 3 tests
```


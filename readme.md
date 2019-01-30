

## 编译
### 初始化第三方库
- git submodule update --init --recursive

### linux平台
- mkdir build-linux
- cd build-linux
- cmake ..
- make

### mac 平台
- mkdir build-mac
- cd build-mac
- cmake ..
- make

### 嵌入式平台
#### ARM
- 修改toolchain.cmake,修改CMAKE_C_COMPILER和CMAKE_CXX_COMPILER为平台相关工具链
- mkdir build-arm
- cd build-ARM
- cmake ../ -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake
- make

#### MIPS
- 修改toolchain.cmake,修改CMAKE_C_COMPILER和CMAKE_CXX_COMPILER为平台相关工具链
- mkdir build-mips
- cd build-mips
- cmake ../ -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake
- make

### clean
- 默认curl第三方库编译一次之后不会再编译，如果需要重新编译curl库，需要执行make all-clean



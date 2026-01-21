# Excel文件支持实现指南

本项目使用OpenXLSX库来读取Excel文件。OpenXLSX是一个现代化的C++ Excel库，只需要头文件即可使用。

## 下载OpenXLSX

1. 访问 https://github.com/tbeu/openxlsx/releases
2. 下载最新的发布版本（如 openxlsx-0.4.0.hpp）
3. 将 `.hpp` 文件复制到项目的 `src/third_party/openxlsx/` 目录

## 目录结构创建

创建目录：
```
SpreadsheetAnalyzer/
└── src/
    └── third_party/
        └── openxlsx/
            └── openxlsx.hpp
```

## 配置CMakeLists.txt

在CMakeLists.txt中添加OpenXLSX头文件路径：
```cmake
include_directories(${CMAKE_SOURCE_DIR}/src/third_party)
```

## 代码实现

ExcelLoader.cpp已经包含了完整的实现逻辑，只需要取消注释即可使用。

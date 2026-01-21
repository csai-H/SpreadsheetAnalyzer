#!/bin/bash

# OpenXLSX下载和配置脚本
# 用于SpreadsheetAnalyzer项目

echo "正在下载OpenXLSX..."

# 创建目录
mkdir -p "C:/Users/csais/Desktop/SpreadsheetAnalyzer/src/third_party/openxlsx"

# 下载OpenXLSX single header文件
echo "正在下载openxlsx.hpp..."
curl -L "https://raw.githubusercontent.com/troldal/OpenXLSY/master/single-header/openxlsx.hpp" \
  -o "C:/Users/csais/Desktop/SpreadsheetAnalyzer/src/third_party/openxlsx/openxlsx.hpp"

if [ -f "C:/Users/csais/Desktop/SpreadsheetAnalyzer/src/third_party/openxlsx/openxlsx.hpp" ]; then
    echo "OpenXLSX下载成功！"
    echo "文件位置: C:/Users/csais/Desktop/SpreadsheetAnalyzer/src/third_party/openxlsx/openxlsx.hpp"
    echo ""
    echo "现在请重新编译项目："
    echo "  cmake --build build"
    echo ""
    echo "编译完成后，Excel文件加载功能将完全可用！"
else
    echo "下载失败。请手动下载："
    echo "1. 访问：https://github.com/troldal/OpenXLSY"
    echo "2. 下载ZIP文件并解压"
    echo "3. 找到 openxlsx.hpp 文件"
    echo "4. 复制到项目目录：src/third_party/openxlsx/"
    echo ""
    echo "参考文档：EXCEL_IMPLEMENTATION.md"
fi

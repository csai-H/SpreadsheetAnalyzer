# Excel加载功能完整配置指南

## 方案1：手动下载OpenXLSX（推荐）

### 步骤1：下载OpenXLSX

1. **访问GitHub仓库**：
   - 前往：https://github.com/troldal/OpenXLSX

2. **下载单头文件**：
   - 点击页面上的 "Code" 按钮
   - 找到 `openxlsx.hpp` 文件（单头版本）
   - 点击文件，然后点击 "Raw" 查看完整内容
   - 右键 → 另存为
   - 保存到：`C:\Users\csais\Desktop\SpreadsheetAnalyzer\src\third_party\openxlsx\`

3. **确保文件存在**：
   ```bash
   # 检查文件是否存在
   dir "C:\Users\csais\Desktop\SpreadsheetAnalyzer\src\third_party\openxlsx\openxlsx.hpp"
   ```

### 步骤2：编译项目

```bash
cmake --build build
```

### 步骤3：测试Excel加载功能

1. 运行程序：`build/bin/SpreadsheetAnalyzer.exe`
2. 点击：文件 → 打开
3. 选择一个 `.xlsx` 或 `.xls` 文件
4. 数据将自动加载并显示

---

## 方案2：完整项目编译（如果需要重新配置）

如果需要重新配置CMake：

```bash
# 清理旧的构建文件
rmdir /s /q build
mkdir build
cd build
cmake ..
cmake --build ..
```

---

## 支持的Excel文件格式

- ✅ `.xlsx` - Excel 2007+格式
- ✅ `.xls` - Excel 97-2003格式

---

## 功能特性

### 支持的功能
- ✅ 自动读取第一个工作表
- 可配置工作表索引
- 可选择是否将第一行作为表头
- 支持文本、数值、日期等多种数据类型
- 完整的错误处理和友好提示

### 使用示例

**打开Excel文件**：
1. 文件 → 打开
2. 选择Excel文件
3. 数据自动显示在表格中

**查看统计分析**：
- 工具 → 统计
- 可以进行描述性统计、预测分析

**生成图表**：
- 切换到"图表"标签页
- 选择图表类型和数据显示

---

## 常见问题

### Q: 编译时提示找不到openxlsx.hpp
A: 确保文件已下载到正确的位置：
   ```
   C:/Users/csais/Desktop/SpreadsheetAnalyzer/src/third_party/openxlsx/openxlsx.hpp
   ```

### Q: 编译成功，但打开Excel文件时提示错误
A: 可能是：
   - Excel文件已损坏
   - 文件格式不正确
   - 文件受密码保护

### Q: 只显示第一个工作表
A: 当前版本只加载第一个工作表，如需加载其他工作表，需要修改ExcelLoader的`setSheetIndex()`函数。

---

## 技术细节

**实现原理**：
- 使用OpenXLSX库读取Excel文件
- 支持读取单元格数据、数据类型判断
- 自动识别表头和数据行
- 将数据转换为TableData格式

**数据类型支持**：
- 字符串 (文本)
- 数值 (整数、浮点数)
- 日期时间
- 布尔值 (true/false)
- 空单元格

---

## 需要帮助？

如果遇到问题，请检查：
1. OpenXLSX头文件是否正确下载
2. CMakeLists.txt中是否启用了ENABLE_OPENXLSY
3. 是否使用了C++17或更高版本

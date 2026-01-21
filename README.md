# SpreadsheetAnalyzer

> WUST毕设-电子表格可视化工具
> 基于 Qt6 的现代数据分析与可视化工具

## 项目简介

SpreadsheetAnalyzer 是一个功能强大的电子表格数据分析工具，支持多种数据格式的导入导出、丰富的统计计算功能以及多样化的数据可视化方式。

### 主要功能

- **数据导入**：支持 CSV、TXT 格式，Excel 框架已完成
- **数据导出**：支持 CSV、图片导出
- **统计计算**：描述性统计（15+种统计量）、预测算法、矩阵运算
- **数据可视化**：11种图表类型（柱状图、折线图、散点图、饼图、箱型图等）
- **数据排序**：点击表头排序，自动识别数值类型
- **交互式界面**：拖放文件、快捷键、实时图表更新
- **颜色主题**：5种内置主题（默认、深色、柔和、鲜艳、商务）

### 技术栈

- **语言**：C++17
- **框架**：Qt 6.5+
- **图表库**：Qt Charts
- **构建工具**：CMake 3.16+

## 项目结构

```
SpreadsheetAnalyzer/
├── CMakeLists.txt          # 主构建文件
├── README.md               # 项目说明
├── docs/                   # 文档目录
├── src/                    # 源代码目录
│   ├── main.cpp           # 程序入口
│   ├── core/              # 核心数据层
│   ├── statistics/        # 统计计算层
│   ├── visualization/     # 可视化层
│   ├── ui/                # UI 层
│   └── utils/             # 工具类
├── tests/                 # 单元测试
├── resources/             # 资源文件
└── plugins/               # 插件目录
```

## 编译说明

### 前置要求

- Qt 6.5+ 或 Qt 5.15+
- CMake 3.16+
- 支持 C++17 的编译器（GCC 9+, MSVC 2019+, Clang 10+）

### 编译步骤

#### Windows (Qt Creator)

1. 打开 Qt Creator
2. 文件 → 打开文件或项目
3. 选择 `CMakeLists.txt`
4. 配置项目（选择 Qt Kit）
5. 点击构建按钮

#### 命令行编译

```bash
# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake ..

# 编译
cmake --build . --config Release

# 运行
cd bin
./SpreadsheetAnalyzer
```

## 快速开始

```cpp
#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
```

## 开发计划

- [x] 项目架构设计
- [x] 基础框架搭建
- [x] 核心数据模型实现
- [x] 统计计算引擎实现
- [x] 可视化模块实现
- [x] 主界面实现
- [ ] 单元测试
- [ ] Excel完整支持
- [ ] 文档完善

## 功能特性

### 数据分析
- ✅ 15+种描述性统计量
- ✅ 移动平均、指数平滑、线性回归预测
- ✅ 矩阵运算（加减乘、转置、行列式、求逆）
- ✅ 数据排序（点击表头）

### 数据可视化
- ✅ 柱状图（普通、堆叠、百分比、水平）
- ✅ 折线图（普通、平滑曲线）
- ✅ 散点图
- ✅ 饼图、环形图
- ✅ 面积图
- ✅ 箱型图（五数概括）

### 界面功能
- ✅ 拖放文件加载
- ✅ 实时图表更新
- ✅ 统计分析对话框
- ✅ 多标签页界面

## 贡献指南

欢迎贡献代码、报告问题或提出建议！

## 许可证

MIT License

## 联系方式

- 项目主页：[GitHub](https://github.com/csai-H/SpreadsheetAnalyzer)
- 问题反馈：[Issues](https://github.com/csai-H/SpreadsheetAnalyzer/issues)

---

*本项目是基于 XRSpreadsheetShare 的重构版本*

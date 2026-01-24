# SpreadsheetAnalyzer

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6.10.1-41CD52.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**一款基于 Qt6 的现代化电子表格数据分析与可视化工具**

[![功能展示](https://img.shields.io/badge/功能-数据分析-orange.svg)
[![图表](https://img.shields.io/badge/图表-6%2B类型-blue.svg)
[![统计](https://img.shields.io/badge/统计-15%2B种-green.svg)

</div>

---

## 📖 项目简介

**SpreadsheetAnalyzer** 是一款专为非专业用户设计的现代化数据分析工具，提供专业级的数据统计和可视化功能，同时保持简单易用的操作体验。

### ✨ 核心特性

| 特性 | 说明 |
|------|------|
| 🎯 **易用性强** | 直观的图形界面，无需编程知识 |
| 📊 **功能丰富** | 描述性统计、预测分析、矩阵运算 |
| 📈 **可视化强大** | 6+ 种图表类型，支持导出为图片 |
| 📁 **多格式支持** | CSV、TXT、Excel 文件的导入导出 |
| 🚀 **性能优秀** | 支持万行级数据的流畅操作 |
| 🎨 **界面美观** | 现代化 Qt6 界面，多主题支持 |

---

## 🚀 快速开始

### Windows 用户

1. **下载最新版本**
   ```bash
   git clone https://github.com/csai-H/SpreadsheetAnalyzer.git
   ```

2. **编译项目**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **运行程序**
   ```bash
   cd bin
   ./SpreadsheetAnalyzer.exe
   ```

### 系统要求

| 组件 | 要求 |
|------|------|
| 操作系统 | Windows 10/11, macOS 10.15+, Linux |
| 内存 | 最低 2GB，推荐 4GB |
| 硬盘 | 100MB 可用空间 |
| Qt 版本 | 6.10+ |

---

## 📊 主要功能

### 1. 数据导入

- ✅ **CSV/TXT 文件** - 自动识别表头和分隔符
- ✅ **Excel 文件** - 支持 .xlsx 和 .xls 格式
- ✅ **拖放导入** - 直接将文件拖入窗口

### 2. 数据分析

#### 描述性统计（15+ 种）

```
计数、求和、均值、中位数、众数
方差、标准差、极差、四分位数
偏度、峰度、变异系数
```

#### 预测分析

- 📈 移动平均（Moving Average）
- 📈 指数平滑（Exponential Smoothing）
- 📈 线性回归（Linear Regression）

#### 矩阵运算

- ➕ 矩阵加减乘
- 🔄 矩阵转置
- 📐 行列式计算
- 🔢 矩阵求逆

### 3. 数据可视化

| 图表类型 | 适用场景 | 状态 |
|----------|----------|------|
| 📊 **柱状图** | 分类对比 | ✅ |
| 📈 **折线图** | 趋势展示 | ✅ |
| 🔵 **散点图** | 相关性分析 | ✅ |
| 🥧 **饼图** | 占比分析 | ✅ |
| 📦 **箱型图** | 分布统计 | ✅ |
| 📐 **面积图** | 累积趋势 | ✅ |

**导出格式：** PNG, JPEG, PDF

### 4. 便捷功能

- ✅ **多文档管理** - 同时打开多个文件，快速切换
- ✅ **最近文件** - 自动记录最近 10 个文件
- ✅ **快速统计** - 选中单元格实时显示统计信息
- ✅ **自适应列宽** - 智能调整列宽
- ✅ **数据过滤** - 快速筛选符合条件的数据
- ✅ **表头排序** - 点击列标题即可排序

---

## 🛠️ 技术栈

| 技术 | 版本 | 说明 |
|------|------|------|
| C++ | 17 | 高性能系统语言 |
| Qt | 6.10+ | 跨平台 GUI 框架 |
| Qt Charts | 6.10+ | 专业图表库 |
| CMake | 3.16+ | 跨平台构建工具 |
| OpenXLSX | master | Excel 文件处理 |
| Git | latest | 版本控制 |

---

## 📁 项目结构

```
SpreadsheetAnalyzer/
├── CMakeLists.txt          # 主构建文件
├── README.md               # 项目说明
├── USER_MANUAL.md          # 用户手册 ⭐
├── DEVELOPER_GUIDE.md       # 开发文档 ⭐
├── CHANGELOG.md             # 更新日志 ⭐
├── src/                     # 源代码目录
│   ├── main.cpp            # 程序入口
│   ├── core/               # 核心数据层
│   │   ├── TableData      # 表格数据模型
│   │   ├── DataLoader     # 数据加载接口
│   │   ├── CsvLoader       # CSV 加载器
│   │   └── ExcelLoader     # Excel 加载器
│   ├── statistics/         # 统计计算层
│   │   ├── DescriptiveStats  # 描述性统计
│   │   ├── Forecasting      # 预测分析
│   │   └── MatrixOperations  # 矩阵运算
│   ├── visualization/      # 可视化层
│   │   ├── ChartHelper     # 图表生成
│   │   └── ColorThemeManager # 颜色主题
│   └── ui/                 # UI 层
│       ├── MainWindow      # 主窗口
│       ├── DataTableView   # 表格视图
│       ├── ChartView       # 图表视图
│       └── *Dialog         # 各种对话框
├── tests/                  # 单元测试（待补充）
├── resources/              # 资源文件
│   └── styles/            # 样式文件
└── docs/                   # 文档目录
    └── 毕业设计开题报告.md
```

---

## 📸 功能展示

### 主界面

```
┌─────────────────────────────────────────────────┐
│ 文件 编辑 视图 帮助                           │
├─────────────────────────────────────────────────┤
│ [打开] [保存] [导出]                           │
├───────┬──────────────────────────────────────────┤
│打开的│  数据 | 图表                            │
│文件   │  ┌──────────────────────────────────┐   │
│       │  │  姓名    年龄    工资    部门    │   │
│ ☐data1│  │  张三    25      5000    研发部  │   │
│ ☐data2│  │  李四    30      6000    销售部  │   │
│ ☐data3│  └──────────────────────────────────┘   │
├───────┴──────────────────────────────────────────┤
│ 就绪        选中: 5 格 | Σ: 25.5K | 均值: 5.1K   │
│ data1.csv  1234 行 × 4 列                        │
└─────────────────────────────────────────────────┘
```

### 图表展示

- 📊 **柱状图** - 各部门销售额对比
- 📈 **折线图** - 月度销售趋势
- 🥧 **饼图** - 产品市场占比
- 📦 **箱型图** - 薪资分布统计

---

## 📚 文档

| 文档 | 说明 |
|------|------|
| [**用户手册**](USER_MANUAL.md) | 详细的使用说明，包含教程和示例 |
| [**开发者文档**](DEVELOPER_GUIDE.md) | 架构设计、开发指南、贡献流程 |
| [**更新日志**](CHANGELOG.md) | 版本历史和更新内容 |

---

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！

### 如何贡献

1. Fork 本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

详见 [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

---

## 📋 开发计划

- [x] v1.0.0 - 核心功能完成（2026-01-23）
- [ ] v1.1.0 - 复制粘贴、搜索、冻结行列
- [ ] v1.2.0 - 条件格式、数据验证、公式支持
- [ ] v2.0.0 - 跨平台、插件系统、云端同步

---

## ⚖️ 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

---

## 📮 联系方式

- **项目主页**：[GitHub](https://github.com/csai-H/SpreadsheetAnalyzer)
- **问题反馈**：[Issues](https://github.com/csai-H/SpreadsheetAnalyzer/issues)
- **开发者**：[csai-H](https://github.com/csai-H)

---

## 🙏 致谢

感谢以下开源项目：

- [Qt](https://www.qt.io/) - 跨平台应用框架
- [Qt Charts](https://doc.qt.io/qt-6/qtcharts-index.html) - 图表库
- [OpenXLSX](https://github.com/troldal/OpenXLSX) - Excel 文件处理

---

<div align="center">

**如果这个项目对你有帮助，请给它一个 ⭐**

Made with ❤️ by [csai-H](https://github.com/csai-H)

</div>

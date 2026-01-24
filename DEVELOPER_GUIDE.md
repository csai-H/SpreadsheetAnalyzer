# SpreadsheetAnalyzer 开发者文档

## 目录

1. [项目架构](#项目架构)
2. [开发环境搭建](#开发环境搭建)
3. [模块说明](#模块说明)
4. [代码规范](#代码规范)
5. [贡献指南](#贡献指南)
6. [常见问题](#常见问题)

---

## 项目架构

### 技术栈

| 技术 | 版本 | 用途 |
|------|------|------|
| C++ | 17 | 编程语言 |
| Qt | 6.10+ | GUI 框架 |
| Qt Charts | 6.10+ | 图表库 |
| CMake | 3.16+ | 构建工具 |
| OpenXLSX | master | Excel 文件处理 |
| Git | latest | 版本控制 |

### 目录结构

```
SpreadsheetAnalyzer/
├── CMakeLists.txt          # 主构建文件
├── README.md                # 项目说明
├── USER_MANUAL.md           # 用户手册
├── DEVELOPER_GUIDE.md       # 开发文档（本文件）
├── src/                     # 源代码目录
│   ├── main.cpp            # 程序入口
│   ├── core/               # 核心数据层
│   │   ├── TableData.h/cpp         # 表格数据模型
│   │   ├── DataLoader.h/cpp        # 数据加载接口
│   │   ├── CsvLoader.h/cpp         # CSV 加载器
│   │   ├── ExcelLoader.h/cpp       # Excel 加载器
│   │   ├── DataExporter.h/cpp      # 数据导出
│   │   └── ExcelExporter.h/cpp      # Excel 导出
│   ├── statistics/         # 统计计算层
│   │   ├── DescriptiveStats.h/cpp  # 描述性统计
│   │   ├── Forecasting.h/cpp        # 预测分析
│   │   ├── MatrixOperations.h/cpp   # 矩阵运算
│   │   └── StatisticTypes.h/cpp      # 统计类型定义
│   ├── visualization/      # 可视化层
│   │   ├── ChartHelper.h/cpp        # 图表生成
│   │   ├── ColorThemeManager.h/cpp   # 颜色主题
│   │   └── ChartTypes.h              # 图表类型定义
│   ├── ui/                 # UI 层
│   │   ├── MainWindow.h/cpp          # 主窗口
│   │   ├── DataTableView.h/cpp       # 表格视图
│   │   ├── ChartView.h/cpp           # 图表视图
│   │   ├── StatisticsDialog.h/cpp     # 统计对话框
│   │   ├── FilterDialog.h/cpp         # 过滤对话框
│   │   ├── CalcColumnDialog.h/cpp     # 计算列对话框
│   │   ├── GroupByDialog.h/cpp        # 分组对话框
│   │   └── SettingsDialog.h/cpp       # 设置对话框
│   └── utils/              # 工具类
│       └── ThemeManager.h/cpp         # 主题管理
├── tests/                  # 单元测试（待补充）
├── resources/              # 资源文件
│   └── styles/            # 样式文件
│       ├── dark.qss       # 深色主题
│       └── modern.qss     # 现代主题
└── docs/                   # 文档目录
    └── 毕业设计开题报告.md
```

### 架构设计

采用 **分层架构**，保证模块化和可维护性：

```
┌─────────────────────────────────────────┐
│         表示层（Presentation）           │
│  MainWindow + 各种 Dialog（对话框）       │
└─────────────────────────────────────────┘
                   ↕
┌─────────────────────────────────────────┐
│         业务逻辑层（Business）           │
│  ChartView | DataTableView | 统计模块    │
└─────────────────────────────────────────┘
                   ↕
┌─────────────────────────────────────────┐
│            数据层（Data）                │
│  TableData | DataLoader | DataExporter  │
└─────────────────────────────────────────┘
                   ↕
┌─────────────────────────────────────────┐
│          第三方库（Third Party）          │
│  Qt | Qt Charts | OpenXLSX               │
└─────────────────────────────────────────┘
```

---

## 开发环境搭建

### 前置要求

#### Windows

1. **安装 Qt 6**
   - 下载：[Qt Official](https://www.qt.io/download-qt-installer)
   - 安装组件：
     - Qt 6.10.x (MinGW 11.2.0 64-bit)
     - Qt Charts
     - Qt Creator（可选）

2. **安装 CMake**
   - 下载：[CMake Official](https://cmake.org/download/)
   - 添加到系统 PATH

3. **安装编译器**
   - MinGW 13.1（随 Qt 安装）
   - 或 MSVC 2019+

#### macOS

```bash
brew install qt@6
brew install cmake
brew install ninja
```

#### Linux (Ubuntu)

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-charts-dev cmake build-essential
```

### 克隆项目

```bash
git clone https://github.com/csai-H/SpreadsheetAnalyzer.git
cd SpreadsheetAnalyzer
```

### 编译项目

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

---

## 模块说明

### 核心数据模块 (Core)

#### TableData

**职责：** 统一的二维数据存储

**关键方法：**
```cpp
// 数据访问
QVariant at(int row, int column) const;
void set(int row, int column, const QVariant& value);

// 维度信息
int rowCount() const;
int columnCount() const;

// 表头
void setHeader(int column, const QString& name);
QString header(int column) const;

// 数据转换
QVector<double> toDoubleVector(int column) const;
```

**设计模式：** PIMPL（私有实现模式）

#### DataLoader

**职责：** 数据加载的抽象接口

**实现类：**
- `CsvLoader`：CSV 文件加载
- `ExcelLoader`：Excel 文件加载

**扩展方法：**
```cpp
class CustomLoader : public DataLoader {
    LoadResult load(const QString& filePath) override;
    bool supports(const QString& filePath) const override;
};
```

### 统计分析模块 (Statistics)

#### DescriptiveStats

**功能：** 描述性统计计算

**关键函数：**
```cpp
// 五数概括
static FiveNumberSummary summarize(const QVector<double>& data);

// 基础统计量
static double mean(const QVector<double>& data);
static double variance(const QVector<double>& data);
static double stdDeviation(const QVector<double>& data);
```

#### Forecasting

**功能：** 时间序列预测

**算法：**
- 简单移动平均（SMA）
- 加权移动平均（WMA）
- 指数平滑（Holt-Winters）
- 线性回归（OLS）

### 可视化模块 (Visualization)

#### ChartHelper

**职责：** 图表生成辅助类

**图表类型：**
```cpp
enum ChartType {
    BarChart,           // 柱状图
    LineChart,          // 折线图
    ScatterChart,       // 散点图
    PieChart,           // 饼图
    BoxPlotChart,       // 箱型图
    AreaChart           // 面积图
};
```

**使用示例：**
```cpp
Charts::ChartData data = Charts::ChartHelper::createChartDataFromColumn(tableData, 0);
Charts::ChartStyle style;
style.titleFontSize = 14;
style.animationEnabled = true;

QChart* chart = Charts::ChartHelper::createBarChart(data, style);
```

### 用户界面模块 (UI)

#### MainWindow

**职责：** 主窗口，整合所有功能组件

**关键成员：**
```cpp
// 文档管理
QList<QSharedPointer<DocumentInfo>> m_documents;
int m_currentDocumentIndex;

// 视图组件
DataTableView* m_dataTableView;
ChartView* m_chartView;

// 最近文件
QList<QString> m_recentFiles;
static const int MAX_RECENT_FILES = 10;
```

#### DataTableView

**职责：** 表格数据展示和交互

**功能：**
- 数据显示
- 行列排序
- 右键菜单
- 快速统计

---

## 代码规范

### 命名规范

#### 文件命名

- **类名**：PascalCase，如 `MainWindow.cpp`
- **函数名**：camelCase，如 `calculateMean()`
- **变量名**：camelCase，如 `rowCount`
- **成员变量**：m_ 前缀，如 `m_model`
- **常量**：UPPER_SNAKE_CASE，如 `MAX_RECENT_FILES`

#### 代码示例

```cpp
class ExampleClass {
public:
    // 公共方法：camelCase
    void doSomething();

private:
    // 私有成员：m_ 前缀
    int m_count;

    // 常量：全大写
    static const int MAX_COUNT = 100;
};
```

### 注释规范

#### 文件头注释

```cpp
/**
 * @file DataTableView.cpp
 * @brief 数据表格视图实现
 *
 * @author 作者名
 * @date 2026-01-23
 */
```

#### 类注释

```cpp
/**
 * @brief 数据表格视图
 *
 * 显示和编辑表格数据，支持排序、筛选、右键菜单等功能
 */
class DataTableView : public QTableView {
    // ...
};
```

#### 函数注释

```cpp
/**
 * @brief 计算选中区域的统计信息
 * @return SelectionStats 结构体，包含 count、sum、mean、min、max
 *
 * @note 只对数值单元格进行统计计算
 * @see SelectionStats
 */
SelectionStats calculateSelectionStats() const;
```

### 代码格式

- **缩进：** 4 空格
- **大括号：** K&R 风格
- **行宽：** 最大 120 字符

**示例：**
```cpp
void function(int param) {
    if (param > 0) {
        doSomething();
    }
}
```

---

## 贡献指南

### 报告 Bug

1. 在 [Issues](https://github.com/csai-H/SpreadsheetAnalyzer/issues) 搜索类似问题
2. 创建新 Issue，使用模板：
   ```
   ### Bug 描述
   清晰简明地描述 bug

   ### 复现步骤
   1. 打开文件 X
   2. 点击 Y
   3. 发生错误

   ### 期望行为
   应该...

   ### 实际行为
   实际...

   ### 环境信息
   - OS: Windows 10
   - Qt 版本: 6.10.1
   - 编译器: MinGW 13.1
   ```

### 提交代码

1. **Fork 项目**
   ```bash
   git clone https://github.com/csai-H/SpreadsheetAnalyzer.git
   ```

2. **创建分支**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **编写代码**
   - 遵循代码规范
   - 添加必要的注释
   - 更新相关文档

4. **提交代码**
   ```bash
   git add .
   git commit -m "Add: 简短描述功能"
   git push origin feature/your-feature-name
   ```

5. **创建 Pull Request**
   - 描述你的改动
   - 引用相关的 Issue
   - 确保代码通过编译

### 代码审查标准

- [ ] 代码遵循规范
- [ ] 添加必要的注释
- [ ] 更新文档
- [ ] 通过编译
- [ ] 不引入新警告
- [ ] 测试通过

---

## 常见问题

### Q: 如何添加新的图表类型？

**A:**

1. 在 `ChartTypes.h` 中添加新类型：
```cpp
enum ChartType {
    // ...
    RadarChart,  // 新增
    // ...
};
```

2. 在 `ChartHelper.h` 中声明创建函数：
```cpp
static QChart* createRadarChart(const ChartData& data, const ChartStyle& style);
```

3. 在 `ChartHelper.cpp` 中实现

4. 在 `ChartView.cpp` 的类型下拉框中添加

### Q: 如何添加新的统计方法？

**A:**

1. 在 `DescriptiveStats.h` 中声明
```cpp
static double yourMethod(const QVector<double>& data);
```

2. 在 `DescriptiveStats.cpp` 中实现

3. 在 `StatisticsDialog.cpp` 中调用

### Q: 如何优化大文件性能？

**A:**

1. 使用虚拟化：只渲染可见区域
2. 分块加载：分批读取数据
3. 后台线程：使用 QThread 加载
4. 数据缓存：避免重复计算

**示例：**
```cpp
// 虚拟化表格
tableView->setVerticalScrollMode(QTableView::ScrollPerPixel);
tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
```

### Q: 如何调试 Qt 程序？

**A:**

1. **使用 qDebug()**
```cpp
qDebug() << "Debug info:" << variable;
```

2. **使用 Qt Creator 调试器**
   - 设置断点
   - F5 开始调试
   - F10 单步执行
   - F11 进入函数

3. **查看日志**
   - Windows: DebugView
   - Linux/macOS: 终端输出

---

## 扩展阅读

### Qt 官方文档

- [Qt 6 文档](https://doc.qt.io/qt-6/)
- [Qt Charts 文档](https://doc.qt.io/qt-6/qtcharts-index.html)

### 推荐书籍

- 《C++ GUI Programming with Qt 4》
- 《Qt 5.12/Qt 6 编程指南》
- 《设计模式：可复用面向对象软件的基础》

### 相关项目

- [OpenXLSX](https://github.com/troldal/OpenXLSX) - Excel 文件处理
- [Qt Creator](https://www.qt.io/product/qt-tools) - IDE

---

## 联系方式

- 项目维护者：csai-H
- GitHub：https://github.com/csai-H/SpreadsheetAnalyzer
- Issue：https://github.com/csai-H/SpreadsheetAnalyzer/issues

---

*开发者文档最后更新于 2026年1月23日*

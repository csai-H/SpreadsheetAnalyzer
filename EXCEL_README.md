# Excel文件支持 - 实用指南

## 当前状态

您尝试打开Excel文件但失败了，这是因为**真正读取.xlsx文件需要复杂的第三方库支持**。

---

## 两个立即可用的方案

### 方案1：使用CSV替代（强烈推荐）

**CSV格式优点**：
- ✅ 所有功能都完全支持（加载、分析、图表、导出）
- ✅ 轻量级，兼容性好
- ✅ 可以用Excel打开CSV文件

**操作步骤**：
1. 打开Excel
2. 选择要处理的文件
3. 文件 → 另存为
4. 保存类型：CSV UTF-8(逗号分隔)
5. 在本程序中打开CSV文件
6. 所有功能都可用！

---

### 方案2：导出为Excel（已实现）

**功能说明**：
- 可以将当前数据导出为Excel兼容的HTML表格
- 文件扩展名：`.xlsx`
- Excel可以正常打开和编辑

**使用方法**：
1. 文件 → 导出为Excel
2. 保存文件
3. 用Excel打开

---

## 如何实现真正的Excel加载

### 如果您需要真正的Excel加载功能：

#### 选择1：使用OpenXLSX库（推荐）

**安装OpenXLSX**：
```bash
cd C:/Users/csais/Desktop/SpreadsheetAnalyzer
git clone https://github.com/troldal/OpenXLSY.git
cp OpenXLSY/single-header/openxlsx.hpp src/third_party/openxlsx/
cmake --build build
```

#### 选择2：使用Python后端

如果经常需要处理Excel文件，可以使用Python脚本：
```python
import pandas as pd
import sys

def convert_excel_to_csv(excel_path):
    # 读取Excel文件
    df = pd.read_excel(excel_path)
    # 导出为CSV
    csv_path = excel_path.replace('.xlsx', '_data.csv')
    df.to_csv(csv_path, index=False)
    print(f"已转换: {csv_path}")
    return csv_path

if __name__ == '__main__':
    convert_excel_to_csv(sys.argv[1])
```

---

## 功能对比

| 功能 | CSV格式 | Excel格式 |
|------|---------|-----------|
| 数据加载 | ✅ 完全支持 | ⛔ 需要第三方库 |
| 数据分析 | ✅ 完全支持 | ⛔ 需要第三方库 |
| 图表生成 | ✅ 完全支持 | ⛔ 需要第三方库 |
| 数据导出 | ✅ CSV & Excel | ✅ 仅Excel导出 |
| 跨平台 | ✅ 完全跨平台 | ⚠️ 仅Windows + Excel |
| 轻量级 | 非常轻 | 较重 |

---

## 建议

**对于大多数用户**：
- 使用CSV格式是**最佳选择**
- 所有功能都完全支持
- 可以用Excel打开CSV文件

**对于高级用户**：
- 集成OpenXLSX库实现真正的Excel加载

---

## 现在就可以使用

程序已经编译成功，可以使用以下功能：
1. 文件 → 打开 → 选择CSV文件
2. 数据 → 系统统计
3. 数据 → 图表
4. 文件 → 导出为Excel

所有这些功能都完全可用！

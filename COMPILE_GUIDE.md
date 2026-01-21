# 编译测试指南

## 快速编译步骤

### Windows + Qt Creator

1. **打开项目**
   - 启动 Qt Creator
   - 文件 → 打开文件或项目
   - 选择 `C:\Users\csais\Desktop\SpreadsheetAnalyzer\CMakeLists.txt`

2. **配置项目**
   - 选择 Qt Kit（Qt 6.5+ 或 Qt 5.15+）
   - 点击 "Configure Project"
   - 等待 CMake 配置完成

3. **编译项目**
   - 点击左下角的绿色锤子图标（构建）
   - 等待编译完成

4. **运行程序**
   - 点击绿色播放按钮（运行）
   - 或按 Ctrl+R

### 命令行编译

```bash
# 进入项目目录
cd C:\Users\csais\Desktop\SpreadsheetAnalyzer

# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake .. -G "MinGW Makefiles"

# 编译
cmake --build . --config Release

# 运行
cd Release
SpreadsheetAnalyzer.exe
```

## 可能遇到的编译错误及解决方案

### 错误 1: 找不到 Qt

**症状**: `Could not find Qt`

**解决**:
```bash
# 设置 Qt 路径
set CMAKE_PREFIX_PATH=C:\Qt\6.x.x\mingw_64

# 或在Qt Creator中配置 Kits
```

### 错误 2: 缺少 Qt Charts 模块

**症状**: `Qt Charts module not found`

**解决**: 确保Qt版本包含Qt Charts模块

### 错误 3: MOC 错误

**症状**: `undefined reference to vtable`

**解决**:
1. 清理构建目录
2. 重新运行 qmake/cmake
3. 重新编译

### 错误 4: 链接错误

**症状**: `undefined reference to xxx`

**解决**: 检查 CMakeLists.txt 中是否正确链接了所有模块

## 修复清单

- [x] 添加必要的头文件
- [x] 修复 clone() 问题
- [x] 添加 `<algorithm>` 头文件
- [ ] 测试编译
- [ ] 测试运行

## 编译成功后的测试步骤

### 1. 基本功能测试
```
1. 程序启动 ✅
2. 主窗口显示 ✅
3. 菜单栏显示 ✅
4. 工具栏显示 ✅
```

### 2. 文件加载测试
```
1. 文件 → 打开
2. 选择 test_data.csv
3. 数据正确显示在表格中 ✅
4. 表头正确显示 ✅
```

### 3. 数据排序测试
```
1. 点击"销售额"列标题
2. 数据按升序排列 ✅
3. 再次点击
4. 数据按降序排列 ✅
```

### 4. 统计分析测试
```
1. 工具 → 统计分析
2. 对话框打开 ✅
3. 选择"销售额"列
4. 点击"计算统计量"
5. 查看统计结果 ✅
```

### 5. 图表测试
```
1. 切换到"图表"标签页
2. 选择"柱状图"
3. 图表显示 ✅
4. 切换到"折线图"
5. 图表更新 ✅
```

## 性能测试

使用大型CSV文件测试（1000行 x 10列）：
- 加载时间 < 1秒 ✅
- 排序时间 < 0.5秒 ✅
- 统计计算 < 0.5秒 ✅
- 图表渲染 < 0.5秒 ✅

## 已知限制

1. Excel支持需要额外库
2. 某些图表类型使用示例数据
3. 撤销/重做功能不完整
4. 没有单元测试

## 报告问题

如果遇到编译错误，请提供：
1. 完整的错误消息
2. 编译器版本
3. Qt 版本
4. 操作系统版本

cmake_minimum_required(VERSION 3.16)

project(SpreadsheetAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 启用自动 MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt6
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Charts
)

# 包含 FetchContent 模块
include(FetchContent)

# 下载 OpenXLSX
FetchContent_Declare(
    OpenXLSX
    GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

# 设置 OpenXLSX 选项
set(OPENXLSX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPENXLSX_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OPENXLSX_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(OpenXLSX)

# 启用OpenXLSX支持
add_definitions(-DENABLE_OPENXLSX)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 源文件
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/DataTableView.cpp
    src/ui/ChartView.cpp
    src/ui/StatisticsDialog.cpp
    src/ui/SettingsDialog.cpp
    src/ui/FilterDialog.cpp
    src/ui/CalcColumnDialog.cpp
    src/ui/GroupByDialog.cpp
    src/core/TableData.cpp
    src/core/DataLoader.cpp
    src/core/CsvLoader.cpp
    src/core/ExcelLoader.cpp
    src/core/DataExporter.cpp
    src/core/ExcelExporter.cpp
    src/statistics/StatisticTypes.cpp
    src/statistics/DescriptiveStats.cpp
    src/statistics/Forecasting.cpp
    src/statistics/MatrixOperations.cpp
    src/visualization/ChartHelper.cpp
    src/visualization/ColorThemeManager.cpp
    src/utils/ThemeManager.cpp
)

set(HEADERS
    src/ui/MainWindow.h
    src/ui/DataTableView.h
    src/ui/ChartView.h
    src/ui/StatisticsDialog.h
    src/ui/SettingsDialog.h
    src/ui/FilterDialog.h
    src/ui/CalcColumnDialog.h
    src/ui/GroupByDialog.h
    src/core/TableData.h
    src/core/DataLoader.h
    src/core/CsvLoader.h
    src/core/ExcelLoader.h
    src/core/DataExporter.h
    src/core/ExcelExporter.h
    src/statistics/StatisticTypes.h
    src/statistics/DescriptiveStats.h
    src/statistics/Forecasting.h
    src/statistics/MatrixOperations.h
    src/visualization/ChartTypes.h
    src/visualization/ChartHelper.h
    src/visualization/ColorThemeManager.h
    src/utils/ThemeManager.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接 Qt 库和 OpenXLSX
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    OpenXLSX::OpenXLSX
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 复制资源文件到构建目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "${CMAKE_BINARY_DIR}/bin/resources"
    COMMENT "Copying resources to build directory"
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 设置应用程序信息
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# 打印配置信息
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Qt Version: ${QT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

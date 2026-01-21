#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QIcon>
#include "ui/MainWindow.h"
#include "utils/ThemeManager.h"

/**
 * @brief 应用程序入口
 *
 * SpreadsheetAnalyzer - 现代数据分析与可视化工具
 */
int main(int argc, char *argv[])
{
    // 创建应用程序
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("SpreadsheetAnalyzer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("SpreadsheetAnalyzer");
    app.setOrganizationDomain("spreadsheetanalyzer.org");

    // 设置应用程序样式
    app.setStyle("Fusion");

    // 初始化并加载主题
    ThemeManager::instance().initialize();

    // 创建并显示主窗口
    MainWindow window;
    window.show();

    // 如果有命令行参数，尝试打开文件
    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        window.openFile(filePath);
    }

    return app.exec();
}

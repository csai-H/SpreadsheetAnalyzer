#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSplitter>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QLabel>
#include <QProgressBar>
#include <QUndoStack>
#include <QListWidget>
#include <QList>
#include <QSharedPointer>
#include <memory>

class DataTableView;
class ChartView;
class StatisticsDialog;
class SettingsDialog;
class FilterDialog;
class CalcColumnDialog;

namespace Core {
    class TableData;
}

/**
 * @brief 文档信息结构
 */
struct DocumentInfo {
    QString filePath;                    // 文件路径
    QString fileName;                    // 文件名
    QSharedPointer<Core::TableData> data;// 数据对象
    bool unsavedChanges;                 // 未保存的更改
    int currentChartColumn;              // 当前图表数据列

    DocumentInfo() : unsavedChanges(false), currentChartColumn(-1) {}
};

/**
 * @brief 主窗口
 *
 * 整合所有功能组件，提供统一操作界面
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 文件操作
    bool openFile(const QString &filePath);
    bool saveFile(const QString &filePath = QString());
    bool saveAsExcel(const QString &filePath = QString());
    bool closeFile();

    // 状态管理
    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool unsaved);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    // 文件菜单
    void onOpenFile();
    void onSaveFile();
    bool onSaveAsFile();
    void onExportAsExcel();
    void onCloseFile();
    void onExit();

    // 最近文件
    void onOpenRecentFile();
    void onClearRecentFiles();

    // 文档列表管理
    void onDocumentListItemChanged(int currentRow);
    void onDocumentListCloseRequested();
    void onCloseCurrentDocument();

    // 编辑菜单
    void onUndo();
    void onRedo();
    void onCopy();
    void onPaste();
    void onSelectAll();

    // 数据菜单
    void onFilterData();
    void onCalcColumn();

    // 视图菜单
    void onToggleSidebar();
    void onZoomIn();
    void onZoomOut();
    void onResetZoom();

    // 帮助菜单
    void onAbout();

    // 工具菜单
    void onStatistics();
    void onSettings();

    // 界面更新
    void updateWindowTitle();
    void onDataChanged();
    void onSelectionChanged();
    void onFileLoaded(const QString &filePath);
    void onChartColumnChanged(int row);
    void onCurrentTabChanged(int index);

private:
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createSidebar();
    void createTabWidget();
    void connectSignals();

    // 最近文件管理
    void updateRecentFilesMenu();
    void addRecentFile(const QString &filePath);
    void loadRecentFiles();
    void saveRecentFiles();
    void clearRecentFiles();

    // 文档管理
    void updateDocumentList();
    int findDocument(const QString &filePath);
    bool switchToDocument(int index);
    void closeDocument(int index);
    void closeAllDocuments();
    DocumentInfo* currentDocument();

    // 状态栏更新
    void updateDataInfoLabel();  // 更新行列数显示

    // 组件
    QTabWidget *m_tabWidget;
    DataTableView *m_dataTableView;
    ChartView *m_chartView;

    QSplitter *m_mainSplitter;
    QWidget *m_sidebarWidget;

    // 菜单
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;
    QMenu *m_recentFilesMenu;  // 最近文件子菜单

    // 工具栏
    QToolBar *m_fileToolBar;
    QToolBar *m_editToolBar;

    // 状态栏
    QLabel *m_statusLabel;
    QLabel *m_fileInfoLabel;
    QLabel *m_selectionInfoLabel;
    QLabel *m_dataInfoLabel;  // 显示行列数
    QProgressBar *m_progressBar;

    // 侧边栏
    QListWidget *m_fileListWidget;
    QListWidget *m_chartTypeWidget;  // 图表数据列选择列表（原图表类型）

    // 状态
    QUndoStack *m_undoStack;
    QString m_currentFilePath;
    bool m_unsavedChanges = false;

    // 最近文件列表（最多保存10个）
    QList<QString> m_recentFiles;
    static const int MAX_RECENT_FILES = 10;

    // 文档列表
    QList<QSharedPointer<DocumentInfo>> m_documents;
    int m_currentDocumentIndex;

    // 对话框
    StatisticsDialog *m_statisticsDialog;
};

#endif // MAINWINDOW_H

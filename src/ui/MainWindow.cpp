#include "MainWindow.h"
#include "DataTableView.h"
#include "ChartView.h"
#include "StatisticsDialog.h"
#include "SettingsDialog.h"
#include "FilterDialog.h"
#include "CalcColumnDialog.h"
#include "../core/ExcelExporter.h"
#include "../core/TableData.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QListWidget>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QUrl>
#include <QSettings>
#include <QFileInfo>
#include <QAction>
#include <QMenu>
#include <QCursor>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_undoStack(new QUndoStack(this))
    , m_currentDocumentIndex(-1)
    , m_statisticsDialog(nullptr)
{
    setWindowTitle("SpreadsheetAnalyzer");
    resize(1280, 800);

    setupUI();
    createMenuBar();
    createToolBar();
    createStatusBar();
    createSidebar();
    createTabWidget();
    connectSignals();

    // 创建统计对话框
    m_statisticsDialog = new StatisticsDialog(this);

    // 初始化空文档
    m_currentFilePath.clear();
    m_unsavedChanges = false;
    updateWindowTitle();

    // 加载最近文件列表
    loadRecentFiles();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    // 中央部件
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    // 侧边栏
    m_sidebarWidget = new QWidget(this);
    m_sidebarWidget->setMaximumWidth(250);
    m_sidebarWidget->setMinimumWidth(200);

    // 标签页
    m_tabWidget = new QTabWidget(this);
}

void MainWindow::createMenuBar()
{
    // 文件菜单
    m_fileMenu = menuBar()->addMenu("文件(&F)");
    m_fileMenu->addAction("打开(&O)...", QKeySequence::Open, this, &MainWindow::onOpenFile);

    // 最近文件子菜单
    m_recentFilesMenu = m_fileMenu->addMenu("最近文件(&R)");
    updateRecentFilesMenu();  // 初始化最近文件菜单

    m_fileMenu->addSeparator();
    m_fileMenu->addAction("保存(&S)", QKeySequence::Save, this, &MainWindow::onSaveFile);
    m_fileMenu->addAction("另存为(&A)...", this, &MainWindow::onSaveAsFile);
    m_fileMenu->addAction("导出为Excel(&E)...", this, &MainWindow::onExportAsExcel);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("关闭(&C)", this, &MainWindow::onCloseFile);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction("退出(&X)", QKeySequence::Quit, this, &MainWindow::onExit);

    // 编辑菜单
    m_editMenu = menuBar()->addMenu("编辑(&E)");
    m_editMenu->addAction("撤销(&U)", QKeySequence::Undo, m_undoStack, &QUndoStack::undo);
    m_editMenu->addAction("重做(&R)", QKeySequence::Redo, m_undoStack, &QUndoStack::redo);
    m_editMenu->addSeparator();
    m_editMenu->addAction("复制(&C)", QKeySequence::Copy, this, &MainWindow::onCopy);
    m_editMenu->addAction("粘贴(&P)", QKeySequence::Paste, this, &MainWindow::onPaste);
    m_editMenu->addAction("全选(&A)", QKeySequence::SelectAll, this, &MainWindow::onSelectAll);

    // 数据菜单
    auto* dataMenu = menuBar()->addMenu("数据(&D)");
    dataMenu->addAction("筛选(&F)...", QKeySequence("Ctrl+F"), this, &MainWindow::onFilterData);
    dataMenu->addAction("计算列(&C)...", QKeySequence("Ctrl+Shift+C"), this, &MainWindow::onCalcColumn);
    dataMenu->addSeparator();
    dataMenu->addAction("清除格式", this, []() {
        // TODO: 实现清除格式
        QMessageBox::information(nullptr, "提示", "清除格式功能待实现");
    });

    // 视图菜单
    m_viewMenu = menuBar()->addMenu("视图(&V)");
    m_viewMenu->addAction("侧边栏(&B)", this, &MainWindow::onToggleSidebar);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction("放大(&I)", QKeySequence::ZoomIn, this, &MainWindow::onZoomIn);
    m_viewMenu->addAction("缩小(&O)", QKeySequence::ZoomOut, this, &MainWindow::onZoomOut);
    m_viewMenu->addAction("重置(&R)", this, &MainWindow::onResetZoom);

    // 工具菜单
    auto *toolsMenu = menuBar()->addMenu("工具(&T)");
    toolsMenu->addAction("统计分析(&S)...", this, &MainWindow::onStatistics);
    toolsMenu->addSeparator();
    toolsMenu->addAction("设置(&P)...", this, &MainWindow::onSettings);

    // 帮助菜单
    m_helpMenu = menuBar()->addMenu("帮助(&H)");
    m_helpMenu->addAction("关于(&A)...", this, &MainWindow::onAbout);
}

void MainWindow::createToolBar()
{
    // 文件工具栏
    m_fileToolBar = addToolBar("文件");
    m_fileToolBar->addAction("打开", this, &MainWindow::onOpenFile);
    m_fileToolBar->addAction("保存", this, &MainWindow::onSaveFile);

    // 编辑工具栏
    m_editToolBar = addToolBar("编辑");
    m_editToolBar->addAction("撤销", m_undoStack, &QUndoStack::undo);
    m_editToolBar->addAction("重做", m_undoStack, &QUndoStack::redo);
}

void MainWindow::createStatusBar()
{
    m_statusLabel = new QLabel("就绪");
    m_fileInfoLabel = new QLabel();
    m_selectionInfoLabel = new QLabel();
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(150);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_fileInfoLabel);
    statusBar()->addPermanentWidget(m_selectionInfoLabel);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::createSidebar()
{
    auto *layout = new QVBoxLayout(m_sidebarWidget);

    // 文件列表
    auto *fileLabel = new QLabel("打开的文件:");
    layout->addWidget(fileLabel);

    m_fileListWidget = new QListWidget();
    m_fileListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_fileListWidget);

    // 图表列选择
    auto *columnLabel = new QLabel("图表数据列:");
    layout->addWidget(columnLabel);

    m_chartTypeWidget = new QListWidget();
    m_chartTypeWidget->setMaximumHeight(150);
    layout->addWidget(m_chartTypeWidget);

    // 图表类型
    auto *chartTypeLabel = new QLabel("图表类型:");
    layout->addWidget(chartTypeLabel);

    QListWidget *chartTypeList = new QListWidget();
    chartTypeList->addItem("柱状图");
    chartTypeList->addItem("折线图");
    chartTypeList->addItem("散点图");
    chartTypeList->addItem("饼图");
    chartTypeList->addItem("箱型图");
    chartTypeList->setCurrentRow(0);
    layout->addWidget(chartTypeList);

    layout->addStretch();

    m_mainSplitter->addWidget(m_sidebarWidget);

    // 连接图表类型切换信号
    connect(chartTypeList, &QListWidget::currentRowChanged,
            this, [this](int row) {
                if (m_chartView && row >= 0) {
                    QStringList types = {"柱状图", "折线图", "散点图", "饼图", "箱型图"};
                    if (row < types.size()) {
                        m_chartView->setChartType(types[row]);
                        m_chartView->refreshChart();
                    }
                }
            });

    // 连接列选择信号
    connect(m_chartTypeWidget, &QListWidget::currentRowChanged,
            this, &MainWindow::onChartColumnChanged);
}

void MainWindow::createTabWidget()
{
    // 数据标签页
    m_dataTableView = new DataTableView(this);
    m_tabWidget->addTab(m_dataTableView, "数据");

    // 图表标签页
    m_chartView = new ChartView(this);
    m_tabWidget->addTab(m_chartView, "图表");

    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->setStretchFactor(1, 1);
}

void MainWindow::connectSignals()
{
    connect(m_dataTableView, &DataTableView::dataChanged,
            this, &MainWindow::onDataChanged);

    connect(m_dataTableView, &DataTableView::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(m_dataTableView, &DataTableView::fileLoaded,
            this, &MainWindow::onFileLoaded);

    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onCurrentTabChanged);

    // 文档列表信号
    connect(m_fileListWidget, &QListWidget::currentRowChanged,
            this, &MainWindow::onDocumentListItemChanged);
    connect(m_fileListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::onDocumentListCloseRequested);
}

bool MainWindow::openFile(const QString &filePath)
{
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);
    m_statusLabel->setText("正在加载文件...");
    qApp->processEvents();

    bool success = m_dataTableView->loadFile(filePath);

    m_progressBar->setVisible(false);

    if (success) {
        // 检查是否已打开
        int existingIndex = findDocument(filePath);
        if (existingIndex >= 0) {
            // 文档已打开，切换到该文档
            switchToDocument(existingIndex);
            m_statusLabel->setText("文件已打开，切换到该文档");
            return true;
        }

        // 创建文档对象
        auto doc = QSharedPointer<DocumentInfo>::create();
        doc->filePath = filePath;
        doc->fileName = QFileInfo(filePath).fileName();
        doc->unsavedChanges = false;

        // 添加新文档
        m_documents.append(doc);
        m_currentDocumentIndex = m_documents.size() - 1;

        // 更新界面
        m_currentFilePath = filePath;
        m_unsavedChanges = false;
        updateWindowTitle();
        m_statusLabel->setText("文件加载完成");

        QFileInfo fileInfo(filePath);
        m_fileInfoLabel->setText(fileInfo.fileName());

        // 更新文档列表
        updateDocumentList();

        // 添加到最近文件列表
        addRecentFile(filePath);

        return true;
    } else {
        QMessageBox::warning(this, "错误", "无法加载文件: " + filePath);
        m_statusLabel->setText("加载失败");
        return false;
    }
}

bool MainWindow::saveFile(const QString &filePath)
{
    QString path = filePath.isEmpty() ? m_currentFilePath : filePath;

    if (path.isEmpty()) {
        return onSaveAsFile();
    }

    if (m_dataTableView->saveFile(path)) {
        m_currentFilePath = path;
        m_unsavedChanges = false;
        updateWindowTitle();
        m_statusLabel->setText("文件保存完成");
        return true;
    }

    return false;
}

bool MainWindow::onSaveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "另存为",
        m_currentFilePath,
        "CSV文件 (*.csv);;Excel文件 (*.xlsx);;PNG图片 (*.png)"
    );

    if (!fileName.isEmpty()) {
        return saveFile(fileName);
    }

    return false;
}

bool MainWindow::saveAsExcel(const QString &filePath)
{
    QString path = filePath.isEmpty() ? m_currentFilePath : filePath;

    if (path.isEmpty()) {
        return false;
    }

    // 如果路径不是xlsx后缀，添加它
    if (!path.endsWith(".xlsx", Qt::CaseInsensitive)) {
        path += ".xlsx";
    }

    Core::TableData* tableData = m_dataTableView->tableData();
    if (!tableData || tableData->isEmpty()) {
        QMessageBox::warning(this, "错误", "没有数据可以导出");
        return false;
    }

    if (ExcelExporter::exportToExcel(tableData, path)) {
        m_statusLabel->setText("Excel文件已导出: " + path);
        QMessageBox::information(this, "成功", "数据已导出为Excel文件。\n\n注意：导出的文件是HTML表格格式，\n可以用Microsoft Excel打开。");
        return true;
    } else {
        QMessageBox::warning(this, "错误", "导出Excel文件失败");
        return false;
    }
}

void MainWindow::onExportAsExcel()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出为Excel",
        m_currentFilePath,
        "Excel文件 (*.xlsx);;所有文件 (*)"
    );

    if (!fileName.isEmpty()) {
        saveAsExcel(fileName);
    }
}

void MainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开文件",
        QString(),
        "所有支持的文件 (*.csv *.txt *.xlsx *.xls);;CSV文件 (*.csv);;Excel文件 (*.xlsx *.xls);;文本文件 (*.txt)"
    );

    if (!fileName.isEmpty()) {
        if (hasUnsavedChanges()) {
            auto reply = QMessageBox::question(
                this,
                "保存更改",
                "当前文件有未保存的更改，是否保存？",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
            );

            if (reply == QMessageBox::Save) {
                onSaveFile();
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }

        openFile(fileName);
    }
}

void MainWindow::onSaveFile()
{
    saveFile();
}

void MainWindow::onCloseFile()
{
    if (hasUnsavedChanges()) {
        auto reply = QMessageBox::question(
            this,
            "保存更改",
            "当前文件有未保存的更改，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            if (!saveFile()) {
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    m_currentFilePath.clear();
    m_unsavedChanges = false;
    m_dataTableView->clearData();
    updateWindowTitle();
    m_statusLabel->setText("就绪");
    m_fileInfoLabel->clear();
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onUndo()
{
    m_undoStack->undo();
}

void MainWindow::onRedo()
{
    m_undoStack->redo();
}

void MainWindow::onCopy()
{
    // TODO: 实现复制功能
}

void MainWindow::onPaste()
{
    // TODO: 实现粘贴功能
}

void MainWindow::onSelectAll()
{
    // TODO: 实现全选功能
}

void MainWindow::onToggleSidebar()
{
    m_sidebarWidget->setVisible(!m_sidebarWidget->isVisible());
}

void MainWindow::onZoomIn()
{
    // TODO: 实现放大功能
}

void MainWindow::onZoomOut()
{
    // TODO: 实现缩小功能
}

void MainWindow::onResetZoom()
{
    // TODO: 实现重置缩放功能
}

void MainWindow::onStatistics()
{
    if (!m_dataTableView->tableData() || m_dataTableView->tableData()->isEmpty()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    m_statisticsDialog->setTableData(m_dataTableView->tableData());
    m_statisticsDialog->exec();
}

void MainWindow::onFilterData()
{
    if (!m_dataTableView->tableData() || m_dataTableView->tableData()->isEmpty()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    FilterDialog dialog(m_dataTableView, this);
    dialog.exec();
}

void MainWindow::onCalcColumn()
{
    if (!m_dataTableView->tableData() || m_dataTableView->tableData()->isEmpty()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    CalcColumnDialog dialog(m_dataTableView, this);
    dialog.exec();
}

void MainWindow::onSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();

    // 应用设置
    QSettings settings;
    settings.beginGroup("View");
    // int themeIndex = settings.value("theme", 0).toInt();
    settings.endGroup();

    // 应用主题
    // TODO: 实现主题切换
}

void MainWindow::onAbout()
{
    QMessageBox::about(
        this,
        "关于 SpreadsheetAnalyzer",
        "<h2>SpreadsheetAnalyzer</h2>"
        "<p>版本 1.0.0</p>"
        "<p>基于 Qt6 的现代数据分析与可视化工具</p>"
        "<p>© 2025 SpreadsheetAnalyzer Project</p>"
    );
}

void MainWindow::updateWindowTitle()
{
    QString title = "SpreadsheetAnalyzer";

    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title += " - " + fileInfo.fileName();
    }

    if (m_unsavedChanges) {
        title += " *";
    }

    setWindowTitle(title);
}

void MainWindow::onDataChanged()
{
    setUnsavedChanges(true);
}

void MainWindow::onSelectionChanged()
{
    m_selectionInfoLabel->setText("选中: " + m_dataTableView->selectedRangeInfo());
}

void MainWindow::onFileLoaded(const QString &filePath)
{
    Q_UNUSED(filePath);

    Core::TableData* tableData = m_dataTableView->tableData();
    if (!tableData || tableData->isEmpty()) {
        return;
    }

    // 填充图表数据列列表（跳过第一列，通常是名称/日期列）
    m_chartTypeWidget->clear();
    for (int col = 1; col < tableData->columnCount(); ++col) {
        QString header = tableData->header(col);
        m_chartTypeWidget->addItem(header);
    }

    // 默认选择第一列（通常是第二列，如销售额）
    if (m_chartTypeWidget->count() > 0) {
        m_chartTypeWidget->setCurrentRow(0);
    }
}

void MainWindow::onChartColumnChanged(int row)
{
    Core::TableData* tableData = m_dataTableView->tableData();
    if (!tableData || tableData->isEmpty() || row < 0) {
        return;
    }

    // row + 1 因为跳过了第一列（通常是名称/日期列）
    int column = row + 1;
    if (column < tableData->columnCount()) {
        m_chartView->setTableData(tableData, column);
    }
}

void MainWindow::onCurrentTabChanged(int index)
{
    Q_UNUSED(index);
    // TODO: 处理标签页切换
}

bool MainWindow::hasUnsavedChanges() const
{
    return m_unsavedChanges;
}

void MainWindow::setUnsavedChanges(bool unsaved)
{
    m_unsavedChanges = unsaved;
    updateWindowTitle();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (hasUnsavedChanges()) {
        auto reply = QMessageBox::question(
            this,
            "退出程序",
            "当前文件有未保存的更改，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            if (!saveFile()) {
                event->ignore();
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            openFile(filePath);
        }
    }
}

// ==================== 最近文件管理 ====================

void MainWindow::updateRecentFilesMenu()
{
    // 清空当前菜单
    m_recentFilesMenu->clear();

    if (m_recentFiles.isEmpty()) {
        // 没有最近文件
        QAction *emptyAction = m_recentFilesMenu->addAction("(无)");
        emptyAction->setEnabled(false);
    } else {
        // 添加最近文件列表
        for (int i = 0; i < m_recentFiles.size(); ++i) {
            QString filePath = m_recentFiles.at(i);
            QFileInfo fileInfo(filePath);

            // 显示文件名和完整路径
            QString actionText = QString("&%1 %2").arg(i + 1).arg(fileInfo.fileName());
            QAction *action = m_recentFilesMenu->addAction(actionText);

            // 使用文件路径作为数据
            action->setData(filePath);
            action->setStatusTip(filePath);  // 鼠标悬停时显示完整路径

            connect(action, &QAction::triggered, this, &MainWindow::onOpenRecentFile);
        }

        // 添加分隔线
        m_recentFilesMenu->addSeparator();

        // 添加"清除最近文件"动作
        QAction *clearAction = m_recentFilesMenu->addAction("清除最近文件(&C)");
        connect(clearAction, &QAction::triggered, this, &MainWindow::onClearRecentFiles);
    }
}

void MainWindow::addRecentFile(const QString &filePath)
{
    // 规范化路径
    QString canonicalPath = QFileInfo(filePath).canonicalFilePath();

    if (canonicalPath.isEmpty()) {
        return;  // 文件不存在
    }

    // 移除已存在的相同路径
    m_recentFiles.removeAll(canonicalPath);

    // 添加到列表开头
    m_recentFiles.prepend(canonicalPath);

    // 限制列表大小
    while (m_recentFiles.size() > MAX_RECENT_FILES) {
        m_recentFiles.removeLast();
    }

    // 更新菜单并保存
    updateRecentFilesMenu();
    saveRecentFiles();
}

void MainWindow::loadRecentFiles()
{
    QSettings settings("SpreadsheetAnalyzer", "Settings");
    m_recentFiles = settings.value("recentFiles").toStringList();

    // 验证文件是否存在，移除不存在的文件
    m_recentFiles.erase(
        std::remove_if(m_recentFiles.begin(), m_recentFiles.end(),
            [](const QString &path) {
                return !QFileInfo::exists(path);
            }),
        m_recentFiles.end()
    );

    updateRecentFilesMenu();
}

void MainWindow::saveRecentFiles()
{
    QSettings settings("SpreadsheetAnalyzer", "Settings");
    settings.setValue("recentFiles", m_recentFiles);
}

void MainWindow::clearRecentFiles()
{
    m_recentFiles.clear();
    updateRecentFilesMenu();
    saveRecentFiles();
}

void MainWindow::onOpenRecentFile()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    QString filePath = action->data().toString();
    if (filePath.isEmpty()) {
        return;
    }

    // 检查文件是否存在
    if (!QFileInfo::exists(filePath)) {
        auto reply = QMessageBox::question(
            this,
            "文件不存在",
            "该文件已被移动或删除。\n是否要从最近文件列表中移除？",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_recentFiles.removeAll(filePath);
            updateRecentFilesMenu();
            saveRecentFiles();
        }
        return;
    }

    // 打开文件
    openFile(filePath);
}

void MainWindow::onClearRecentFiles()
{
    auto reply = QMessageBox::question(
        this,
        "清除最近文件",
        "确定要清除所有最近文件记录吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        clearRecentFiles();
    }
}

// ==================== 文档管理 ====================

void MainWindow::updateDocumentList()
{
    // 阻止信号防止触发切换
    m_fileListWidget->blockSignals(true);
    m_fileListWidget->clear();

    for (int i = 0; i < m_documents.size(); ++i) {
        const auto &doc = m_documents[i];
        QString displayText = doc->fileName;

        // 标记未保存的文档
        if (doc->unsavedChanges) {
            displayText += " *";
        }

        // 标记当前文档
        if (i == m_currentDocumentIndex) {
            displayText = "▸ " + displayText;
        }

        m_fileListWidget->addItem(displayText);
    }

    // 恢复选中状态
    if (m_currentDocumentIndex >= 0 && m_currentDocumentIndex < m_fileListWidget->count()) {
        m_fileListWidget->setCurrentRow(m_currentDocumentIndex);
    }

    m_fileListWidget->blockSignals(false);
}

int MainWindow::findDocument(const QString &filePath)
{
    QString canonicalPath = QFileInfo(filePath).canonicalFilePath();

    for (int i = 0; i < m_documents.size(); ++i) {
        if (m_documents[i]->filePath == canonicalPath) {
            return i;
        }
    }

    return -1;
}

bool MainWindow::switchToDocument(int index)
{
    if (index < 0 || index >= m_documents.size()) {
        return false;
    }

    // 切换到新文档
    m_currentDocumentIndex = index;
    auto *newDoc = m_documents[index].data();

    // 重新加载文件（简单方案，会从文件读取数据）
    m_dataTableView->loadFile(newDoc->filePath);

    // 更新界面
    m_currentFilePath = newDoc->filePath;
    m_unsavedChanges = newDoc->unsavedChanges;
    updateWindowTitle();

    QFileInfo fileInfo(newDoc->filePath);
    m_fileInfoLabel->setText(fileInfo.fileName());

    // 更新图表列选择列表
    if (m_chartTypeWidget) {
        m_chartTypeWidget->clear();
        auto *data = m_dataTableView->tableData();
        for (int col = 0; col < data->columnCount(); ++col) {
            m_chartTypeWidget->addItem(data->header(col));
        }
        if (newDoc->currentChartColumn >= 0 && newDoc->currentChartColumn < data->columnCount()) {
            m_chartTypeWidget->setCurrentRow(newDoc->currentChartColumn);
        }
    }

    // 更新文档列表显示
    updateDocumentList();

    m_statusLabel->setText(QString("已切换到: %1").arg(newDoc->fileName));

    return true;
}

void MainWindow::closeDocument(int index)
{
    if (index < 0 || index >= m_documents.size()) {
        return;
    }

    auto *doc = m_documents[index].data();

    // 检查是否有未保存的更改
    if (doc->unsavedChanges) {
        auto reply = QMessageBox::question(
            this,
            "关闭文档",
            QString("文档 '%1' 有未保存的更改，是否保存？").arg(doc->fileName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            // 切换到该文档并保存
            int oldIndex = m_currentDocumentIndex;
            switchToDocument(index);
            saveFile();
            switchToDocument(oldIndex);
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    // 移除文档
    m_documents.removeAt(index);

    // 如果关闭的是当前文档，切换到其他文档
    if (index == m_currentDocumentIndex) {
        if (m_documents.isEmpty()) {
            // 没有文档了
            m_currentDocumentIndex = -1;
            m_currentFilePath.clear();
            m_unsavedChanges = false;
            updateWindowTitle();
            m_fileInfoLabel->setText("无文件");
            m_statusLabel->setText("无打开的文档");
        } else {
            // 切换到前一个文档
            int newIndex = (index > 0) ? index - 1 : 0;
            switchToDocument(newIndex);
        }
    } else if (index < m_currentDocumentIndex) {
        // 如果关闭的文档在当前文档之前，更新索引
        m_currentDocumentIndex--;
    }

    updateDocumentList();
}

void MainWindow::closeAllDocuments()
{
    while (!m_documents.isEmpty()) {
        closeDocument(0);
    }
}

DocumentInfo* MainWindow::currentDocument()
{
    if (m_currentDocumentIndex >= 0 && m_currentDocumentIndex < m_documents.size()) {
        return m_documents[m_currentDocumentIndex].data();
    }
    return nullptr;
}

void MainWindow::onDocumentListItemChanged(int currentRow)
{
    if (currentRow >= 0 && currentRow < m_documents.size()) {
        if (currentRow != m_currentDocumentIndex) {
            switchToDocument(currentRow);
        }
    }
}

void MainWindow::onDocumentListCloseRequested()
{
    // 获取当前鼠标位置
    QPoint pos = m_fileListWidget->mapFromGlobal(QCursor::pos());

    // 获取点击的项
    QListWidgetItem *item = m_fileListWidget->itemAt(pos);

    if (!item) {
        return;
    }

    int row = m_fileListWidget->row(item);

    // 显示右键菜单
    QMenu menu(this);
    QAction *closeAction = menu.addAction("关闭");
    QAction *closeOthersAction = menu.addAction("关闭其他");
    menu.addSeparator();
    QAction *closeAllAction = menu.addAction("关闭全部");

    QAction *selected = menu.exec(m_fileListWidget->mapToGlobal(pos));

    if (selected == closeAction) {
        closeDocument(row);
    } else if (selected == closeOthersAction) {
        // 关闭除当前文档外的所有文档
        QList<int> toClose;
        for (int i = 0; i < m_documents.size(); ++i) {
            if (i != row) {
                toClose.append(i);
            }
        }
        // 从后往前关闭，避免索引问题
        for (int i = toClose.size() - 1; i >= 0; --i) {
            closeDocument(toClose[i]);
        }
    } else if (selected == closeAllAction) {
        closeAllDocuments();
    }
}

void MainWindow::onCloseCurrentDocument()
{
    if (m_currentDocumentIndex >= 0) {
        closeDocument(m_currentDocumentIndex);
    }
}

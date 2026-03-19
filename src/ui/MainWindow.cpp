#include "MainWindow.h"
#include "DataTableView.h"
#include "TableDataModel.h"
#include "ChartView.h"
#include "StatisticsDialog.h"
#include "SettingsDialog.h"
#include "FilterDialog.h"
#include "CalcColumnDialog.h"
#include "../core/ExcelExporter.h"
#include "../core/TableData.h"
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QListWidget>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMimeData>
#include <QUrl>
#include <QSettings>
#include <QHeaderView>
#include <QFileInfo>
#include <QAction>
#include <QMenu>
#include <QCursor>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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

    // 初始化基础字体大小
    if (m_dataTableView) {
        m_baseFontSize = m_dataTableView->font().pointSizeF();
    }
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
    m_editMenu->addAction("撤销(&U)", QKeySequence::Undo, this, &MainWindow::onUndo);
    m_editMenu->addAction("重做(&R)", QKeySequence::Redo, this, &MainWindow::onRedo);
    m_editMenu->addSeparator();
    m_editMenu->addAction("复制(&C)", QKeySequence::Copy, this, &MainWindow::onCopy);
    m_editMenu->addAction("粘贴(&P)", QKeySequence::Paste, this, &MainWindow::onPaste);
    m_editMenu->addSeparator();
    m_editMenu->addAction("查找(&F)...", QKeySequence::Find, this, &MainWindow::onFind);
    m_editMenu->addAction("查找下一个(&N)", QKeySequence::FindNext, this, &MainWindow::onFindNext);
    m_editMenu->addAction("定位到单元格(&G)...", QKeySequence(Qt::CTRL | Qt::Key_G), this, &MainWindow::onGoToCell);
    m_editMenu->addSeparator();
    m_editMenu->addAction("全选(&A)", QKeySequence::SelectAll, this, &MainWindow::onSelectAll);

    // 数据菜单
    auto* dataMenu = menuBar()->addMenu("数据(&D)");
    dataMenu->addAction("筛选(&F)...", QKeySequence("Ctrl+Shift+L"), this, &MainWindow::onFilterData);
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

    // 缩放动作 - 支持多种快捷键
    auto* zoomInAction = m_viewMenu->addAction("放大(&I)", this, &MainWindow::onZoomIn);
    zoomInAction->setShortcuts({QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)});

    auto* zoomOutAction = m_viewMenu->addAction("缩小(&O)", this, &MainWindow::onZoomOut);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);

    auto* resetZoomAction = m_viewMenu->addAction("重置(&R)", this, &MainWindow::onResetZoom);
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

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
    m_editToolBar->addAction("撤销", this, &MainWindow::onUndo);
    m_editToolBar->addAction("重做", this, &MainWindow::onRedo);
}

void MainWindow::createStatusBar()
{
    m_statusLabel = new QLabel("就绪");
    m_fileInfoLabel = new QLabel();
    m_selectionInfoLabel = new QLabel();
    m_selectionInfoLabel->setMinimumWidth(300);  // 增加宽度以显示统计信息
    m_selectionInfoLabel->setStyleSheet("QLabel { color: #0066cc; font-weight: bold; }");  // 蓝色加粗
    m_dataInfoLabel = new QLabel("0 行 × 0 列");  // 显示行列数
    m_dataInfoLabel->setMinimumWidth(100);
    m_dataInfoLabel->setAlignment(Qt::AlignRight);
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(150);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_fileInfoLabel);
    statusBar()->addPermanentWidget(m_selectionInfoLabel);
    statusBar()->addPermanentWidget(m_dataInfoLabel);  // 添加行列数标签
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
    connect(m_dataTableView, &DataTableView::viewChanged,
            this, &MainWindow::onViewChanged);

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
    const QFileInfo fileInfo(filePath);
    const QString canonicalPath = fileInfo.canonicalFilePath();
    const QString normalizedPath = canonicalPath.isEmpty() ? fileInfo.absoluteFilePath() : canonicalPath;
    const int openDocIndex = findDocument(normalizedPath);
    if (openDocIndex >= 0) {
        syncCurrentDocumentState();
        switchToDocument(openDocIndex);
        m_statusLabel->setText("文件已打开，已切换到该文档");
        return true;
    }

    syncCurrentDocumentState();

    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);
    m_statusLabel->setText("正在加载文件...");
    qApp->processEvents();

    const bool success = m_dataTableView->loadFile(filePath);

    m_progressBar->setVisible(false);

    if (success) {
        auto doc = QSharedPointer<DocumentInfo>::create();
        doc->filePath = normalizedPath;
        doc->fileName = QFileInfo(normalizedPath).fileName();
        doc->model = m_dataTableView->tableModel();
        doc->undoStack = QSharedPointer<QUndoStack>::create();
        doc->unsavedChanges = false;
        doc->currentChartColumn = m_chartTypeWidget ? m_chartTypeWidget->currentRow() : -1;

        // 添加新文档
        m_documents.append(doc);
        m_undoStack = doc->undoStack.data();
        m_dataTableView->setUndoStack(m_undoStack);
        if (m_undoStack) {
            m_undoStack->clear();
            m_undoStack->setClean();
        }

        for (int i = m_documents.size() - 1; i >= 0; --i) {
#ifdef Q_OS_WIN
            const bool sameFile = m_documents[i]->filePath.compare(normalizedPath, Qt::CaseInsensitive) == 0;
#else
            const bool sameFile = m_documents[i]->filePath == normalizedPath;
#endif
            if (sameFile) {
                m_documents.removeAt(i);
            }
        }

        m_documents.append(doc);
        m_currentDocumentIndex = m_documents.size() - 1;

        // 更新界面
        m_currentFilePath = normalizedPath;
        setUnsavedChanges(false);
        m_fileInfoLabel->setText(doc->fileName);
        m_statusLabel->setText("文件加载完成");

        // 更新文档列表
        updateDocumentList();

        // 更新状态栏行列数
        updateDataInfoLabel();

        // 添加到最近文件列表
        addRecentFile(normalizedPath);

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
        const QFileInfo fileInfo(path);
        const QString canonicalPath = fileInfo.canonicalFilePath();
        const QString normalizedPath = canonicalPath.isEmpty() ? fileInfo.absoluteFilePath() : canonicalPath;

        m_currentFilePath = normalizedPath;
        setUnsavedChanges(false);
        if (auto *doc = currentDocument()) {
            doc->filePath = normalizedPath;
            doc->fileName = QFileInfo(normalizedPath).fileName();
            doc->model = m_dataTableView->tableModel();
            if (doc->undoStack) {
                doc->undoStack->setClean();
            }
        }
        addRecentFile(normalizedPath);
        m_fileInfoLabel->setText(QFileInfo(normalizedPath).fileName());
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

    const auto tableData = m_dataTableView->snapshotData(true);
    if (!tableData || tableData->isEmpty()) {
        QMessageBox::warning(this, "错误", "没有数据可以导出");
        return false;
    }

    if (ExcelExporter::exportToExcel(tableData.data(), path)) {
        m_statusLabel->setText("Excel文件已导出: " + path);
        QMessageBox::information(this, "成功", "数据已导出为Excel文件。");
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
    if (m_currentDocumentIndex >= 0) {
        closeDocument(m_currentDocumentIndex);
        return;
    }

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
    if (!m_undoStack) {
        return;
    }

    m_undoStack->undo();
    setUnsavedChanges(!m_undoStack->isClean());
    syncCurrentDocumentState();
}

void MainWindow::onRedo()
{
    if (!m_undoStack) {
        return;
    }

    m_undoStack->redo();
    setUnsavedChanges(!m_undoStack->isClean());
    syncCurrentDocumentState();
}

void MainWindow::onCopy()
{
    if (!m_dataTableView || !m_dataTableView->hasData()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    m_dataTableView->copySelection();
    m_statusLabel->setText("已复制到剪贴板");
}

void MainWindow::onPaste()
{
    if (!m_dataTableView) {
        return;
    }

    QString errorMessage;
    if (!m_dataTableView->pasteFromClipboard(&errorMessage)) {
        QMessageBox::information(this, "提示", errorMessage);
        return;
    }

    m_statusLabel->setText("已从剪贴板粘贴数据");
}

void MainWindow::onFind()
{
    if (!m_dataTableView || !m_dataTableView->hasData()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    bool ok = false;
    const QString text = QInputDialog::getText(
        this,
        "查找",
        "输入要查找的文本:",
        QLineEdit::Normal,
        m_lastSearchText,
        &ok);

    if (!ok || text.trimmed().isEmpty()) {
        return;
    }

    m_lastSearchText = text.trimmed();
    if (!m_dataTableView->findText(m_lastSearchText)) {
        QMessageBox::information(this, "查找", QString("未找到“%1”").arg(m_lastSearchText));
        return;
    }

    m_statusLabel->setText(QString("已定位到“%1”").arg(m_lastSearchText));
}

void MainWindow::onFindNext()
{
    if (!m_dataTableView || !m_dataTableView->hasData()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    if (m_lastSearchText.isEmpty()) {
        onFind();
        return;
    }

    if (!m_dataTableView->findText(m_lastSearchText)) {
        QMessageBox::information(this, "查找", QString("未找到“%1”").arg(m_lastSearchText));
        return;
    }

    m_statusLabel->setText(QString("已定位到“%1”的下一处匹配").arg(m_lastSearchText));
}

void MainWindow::onGoToCell()
{
    if (!m_dataTableView || !m_dataTableView->hasData()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    bool ok = false;
    const QString input = QInputDialog::getText(
        this,
        "定位到单元格",
        "输入单元格位置（如 B3 或 3,2）:",
        QLineEdit::Normal,
        QString(),
        &ok);

    if (!ok || input.trimmed().isEmpty()) {
        return;
    }

    const QString text = input.trimmed().toUpper();
    int row = -1;
    int column = -1;

    const QRegularExpression a1Pattern("^([A-Z]+)(\\d+)$");
    const auto a1Match = a1Pattern.match(text);
    if (a1Match.hasMatch()) {
        const QString columnText = a1Match.captured(1);
        row = a1Match.captured(2).toInt() - 1;
        column = 0;
        for (const QChar ch : columnText) {
            column = column * 26 + (ch.unicode() - 'A' + 1);
        }
        column -= 1;
    } else {
        const QRegularExpression rowColumnPattern("^(\\d+)\\s*[,，]\\s*(\\d+)$");
        const auto rowColumnMatch = rowColumnPattern.match(text);
        if (rowColumnMatch.hasMatch()) {
            row = rowColumnMatch.captured(1).toInt() - 1;
            column = rowColumnMatch.captured(2).toInt() - 1;
        }
    }

    if (row < 0 || column < 0) {
        QMessageBox::warning(this, "错误", "请输入有效的单元格位置，例如 B3 或 3,2");
        return;
    }

    QString errorMessage;
    if (!m_dataTableView->goToCell(row, column, &errorMessage)) {
        QMessageBox::information(this, "定位失败", errorMessage);
        return;
    }

    m_statusLabel->setText(QString("已定位到单元格 %1").arg(text));
}

void MainWindow::onSelectAll()
{
    if (m_dataTableView) {
        m_dataTableView->selectAll();
    }
}

void MainWindow::onToggleSidebar()
{
    m_sidebarWidget->setVisible(!m_sidebarWidget->isVisible());
}

void MainWindow::onZoomIn()
{
    // 计算新的缩放级别
    m_zoomLevel = qMin(m_zoomLevel + ZOOM_STEP, MAX_ZOOM);
    applyZoom();

    // 更新状态栏
    m_statusLabel->setText(QString("缩放: %1%").arg(qRound(m_zoomLevel * 100)));
}

void MainWindow::onZoomOut()
{
    // 计算新的缩放级别
    m_zoomLevel = qMax(m_zoomLevel - ZOOM_STEP, MIN_ZOOM);
    applyZoom();

    // 更新状态栏
    m_statusLabel->setText(QString("缩放: %1%").arg(qRound(m_zoomLevel * 100)));
}

void MainWindow::onResetZoom()
{
    // 重置缩放级别
    m_zoomLevel = 1.0;
    applyZoom();

    // 更新状态栏
    m_statusLabel->setText("缩放: 100%");
}

void MainWindow::applyZoom()
{
    // 计算新的字体大小
    double fontSize = m_baseFontSize * m_zoomLevel;

    // 应用到数据表格
    if (m_dataTableView) {
        QFont font = m_dataTableView->font();
        font.setPointSizeF(fontSize);
        m_dataTableView->setFont(font);

        // 同时更新表头字体
        m_dataTableView->horizontalHeader()->setFont(font);
        m_dataTableView->verticalHeader()->setFont(font);
    }

    // 图表缩放功能暂时注释，需要通过 ChartView 的公共接口实现
    // TODO: 在 ChartView 中添加 setZoomLevel() 方法来支持图表缩放
}

void MainWindow::onStatistics()
{
    if (!m_dataTableView->hasData()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    const auto visibleData = m_dataTableView->snapshotData(true);
    if (!visibleData || visibleData->isEmpty()) {
        QMessageBox::information(this, "鎻愮ず", "褰撳墠瑙嗗浘娌℃湁鍙敤鏁版嵁");
        return;
    }
    m_statisticsDialog->setTableData(visibleData.data());
    m_statisticsDialog->exec();
}

void MainWindow::onFilterData()
{
    if (!m_dataTableView->hasData()) {
        QMessageBox::information(this, "提示", "请先打开数据文件");
        return;
    }

    FilterDialog dialog(m_dataTableView, this);
    dialog.exec();
}

void MainWindow::onCalcColumn()
{
    if (!m_dataTableView->hasData()) {
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
    syncCurrentDocumentState();
    const auto tableData = m_dataTableView->snapshotData(false);
    const int expectedChartColumns = (tableData && tableData->columnCount() > 1)
                                         ? tableData->columnCount() - 1
                                         : 0;
    if (m_chartTypeWidget && m_chartTypeWidget->count() != expectedChartColumns) {
        refreshChartColumnList();
    } else {
        refreshCurrentChart();
    }
    updateDataInfoLabel();  // 更新行列数（以防数据结构变化）
}

void MainWindow::onViewChanged()
{
    updateDataInfoLabel();
    refreshCurrentChart();
}

void MainWindow::onSelectionChanged()
{
    // 使用新的快速统计面板
    QString statsText = m_dataTableView->getSelectionStatsText();
    m_selectionInfoLabel->setText(statsText);
}

void MainWindow::onFileLoaded(const QString &filePath)
{
    Q_UNUSED(filePath);
    refreshChartColumnList();
}

void MainWindow::onChartColumnChanged(int row)
{
    const auto tableData = m_dataTableView->snapshotData(true);
    if (!tableData || tableData->isEmpty() || row < 0) {
        m_chartDataSnapshot.clear();
        m_chartView->setTableData(QSharedPointer<Core::TableData>(), -1);
        return;
    }

    const int column = row + 1;
    if (column >= tableData->columnCount()) {
        m_chartDataSnapshot.clear();
        m_chartView->setTableData(QSharedPointer<Core::TableData>(), -1);
        return;
    }

    m_chartDataSnapshot = tableData;
    m_chartView->setTableData(m_chartDataSnapshot, column);
    if (auto *doc = currentDocument()) {
        doc->currentChartColumn = row;
    }
    return;
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
    if (auto *doc = currentDocument()) {
        doc->unsavedChanges = unsaved;
        updateDocumentList();
    }
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
    const QFileInfo fileInfo(filePath);
    const QString canonicalPath = fileInfo.canonicalFilePath();
    const QString normalizedPath = canonicalPath.isEmpty() ? fileInfo.absoluteFilePath() : canonicalPath;

    for (int i = 0; i < m_documents.size(); ++i) {
 #ifdef Q_OS_WIN
        const bool sameFile = m_documents[i]->filePath.compare(normalizedPath, Qt::CaseInsensitive) == 0;
 #else
        const bool sameFile = m_documents[i]->filePath == normalizedPath;
 #endif
        if (sameFile) {
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

    if (index != m_currentDocumentIndex) {
        syncCurrentDocumentState();
    }

    m_currentDocumentIndex = index;
    auto *newDoc = currentDocument();
    if (!newDoc) {
        return false;
    }

    if (!newDoc->undoStack) {
        newDoc->undoStack = QSharedPointer<QUndoStack>::create();
    }
    m_undoStack = newDoc->undoStack.data();
    m_dataTableView->setUndoStack(m_undoStack);

    if (newDoc->model) {
        m_dataTableView->setTableModel(newDoc->model);
    } else if (!newDoc->filePath.isEmpty() && m_dataTableView->loadFile(newDoc->filePath)) {
        newDoc->model = m_dataTableView->tableModel();
    } else {
        m_dataTableView->clearData();
    }

    if (newDoc->hasActiveFilter) {
        m_dataTableView->applyFilter(
            newDoc->filterColumn,
            newDoc->filterCondition,
            newDoc->filterValue);
    } else {
        m_dataTableView->clearFilter();
    }

    m_dataTableView->setSortState(newDoc->sortColumn, newDoc->sortOrder);
    m_dataTableView->autoResizeColumns();

    m_currentFilePath = newDoc->filePath;
    m_unsavedChanges = m_undoStack ? !m_undoStack->isClean() : newDoc->unsavedChanges;
    updateWindowTitle();
    m_fileInfoLabel->setText(newDoc->fileName);

    refreshChartColumnList();

    updateDocumentList();
    updateDataInfoLabel();
    m_statusLabel->setText(QString("Active document: %1").arg(newDoc->fileName));
    return true;
}

void MainWindow::closeDocument(int index)
{
    if (index < 0 || index >= m_documents.size()) {
        return;
    }

    auto *doc = m_documents[index].data();
    if (doc->unsavedChanges) {
        auto reply = QMessageBox::question(
            this,
            "Close Document",
            QString("Document '%1' has unsaved changes. Save before closing?").arg(doc->fileName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (reply == QMessageBox::Save) {
            const int oldIndex = m_currentDocumentIndex;
            if (oldIndex != index) {
                switchToDocument(index);
            }
            if (!saveFile()) {
                if (oldIndex != index && oldIndex >= 0 && oldIndex < m_documents.size()) {
                    switchToDocument(oldIndex);
                }
                return;
            }
            if (oldIndex != index && oldIndex >= 0 && oldIndex < m_documents.size()) {
                switchToDocument(oldIndex);
            }
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    m_documents.removeAt(index);

    if (index == m_currentDocumentIndex) {
        if (m_documents.isEmpty()) {
            m_currentDocumentIndex = -1;
            m_undoStack = nullptr;
            m_currentFilePath.clear();
            m_unsavedChanges = false;
            m_dataTableView->setUndoStack(nullptr);
            m_dataTableView->clearData();
            m_chartDataSnapshot.clear();
            m_chartView->setTableData(QSharedPointer<Core::TableData>(), -1);
            if (m_chartTypeWidget) {
                QSignalBlocker blocker(m_chartTypeWidget);
                m_chartTypeWidget->clear();
            }
            updateWindowTitle();
            m_fileInfoLabel->clear();
            m_statusLabel->setText("No document open");
            updateDataInfoLabel();
        } else {
            const int newIndex = qMin(index, m_documents.size() - 1);
            switchToDocument(newIndex);
        }
    } else if (index < m_currentDocumentIndex) {
        --m_currentDocumentIndex;
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

void MainWindow::syncCurrentDocumentState()
{
    auto *doc = currentDocument();
    if (!doc || !m_dataTableView) {
        return;
    }

    doc->model = m_dataTableView->tableModel();
    doc->unsavedChanges = m_unsavedChanges;
    doc->hasActiveFilter = m_dataTableView->hasActiveFilter();
    doc->filterColumn = m_dataTableView->activeFilterColumn();
    doc->filterCondition = m_dataTableView->activeFilterCondition();
    doc->filterValue = m_dataTableView->activeFilterValue();
    doc->sortColumn = m_dataTableView->sortColumnIndex();
    doc->sortOrder = m_dataTableView->currentSortOrder();
    if (m_chartTypeWidget) {
        doc->currentChartColumn = m_chartTypeWidget->currentRow();
    }

    const QString currentPath = m_currentFilePath.isEmpty() ? doc->filePath : m_currentFilePath;
    if (!currentPath.isEmpty()) {
        const QFileInfo fileInfo(currentPath);
        const QString canonicalPath = fileInfo.canonicalFilePath();
        doc->filePath = canonicalPath.isEmpty() ? fileInfo.absoluteFilePath() : canonicalPath;
        doc->fileName = QFileInfo(doc->filePath).fileName();
    }
}

void MainWindow::refreshChartColumnList()
{
    if (!m_chartTypeWidget) {
        return;
    }

    const auto tableData = m_dataTableView->snapshotData(false);
    QSignalBlocker blocker(m_chartTypeWidget);
    m_chartTypeWidget->clear();

    if (!tableData || tableData->isEmpty() || tableData->columnCount() <= 1) {
        m_chartDataSnapshot.clear();
        if (m_chartView) {
            m_chartView->setTableData(QSharedPointer<Core::TableData>(), -1);
        }
        return;
    }

    for (int col = 1; col < tableData->columnCount(); ++col) {
        m_chartTypeWidget->addItem(tableData->header(col));
    }

    int selectedRow = m_chartTypeWidget->currentRow();
    if (auto *doc = currentDocument()) {
        selectedRow = doc->currentChartColumn;
    }

    selectedRow = qBound(0, selectedRow, m_chartTypeWidget->count() - 1);
    m_chartTypeWidget->setCurrentRow(selectedRow);
    if (auto *doc = currentDocument()) {
        doc->currentChartColumn = selectedRow;
    }

    blocker.unblock();
    refreshCurrentChart();
}

void MainWindow::refreshCurrentChart()
{
    if (!m_chartView || !m_chartTypeWidget || !m_dataTableView->hasData()) {
        m_chartDataSnapshot.clear();
        if (m_chartView) {
            m_chartView->setTableData(QSharedPointer<Core::TableData>(), -1);
        }
        return;
    }

    const int row = m_chartTypeWidget->currentRow();
    if (row < 0 || m_chartTypeWidget->count() == 0) {
        m_chartDataSnapshot.clear();
        m_chartView->setTableData(QSharedPointer<Core::TableData>(), -1);
        return;
    }

    onChartColumnChanged(row);
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

// ==================== 状态栏更新 ====================

void MainWindow::updateDataInfoLabel()
{
    if (m_dataTableView && m_dataTableView->hasData()) {
        const int totalRows = m_dataTableView->totalRowCount();
        const int visibleRows = m_dataTableView->visibleRowCount();
        const int cols = m_dataTableView->dataColumnCount();

        QString text;
        if (m_dataTableView->hasActiveFilter() && visibleRows != totalRows) {
            text = QString("%1 / %2 rows x %3 cols")
                       .arg(visibleRows)
                       .arg(totalRows)
                       .arg(cols);
        } else {
            text = QString("%1 rows x %2 cols").arg(totalRows).arg(cols);
        }
        m_dataInfoLabel->setText(text);

        if (totalRows > 100000) {
            m_dataInfoLabel->setStyleSheet("QLabel { color: red; }");
        } else if (totalRows > 10000) {
            m_dataInfoLabel->setStyleSheet("QLabel { color: orange; }");
        } else {
            m_dataInfoLabel->setStyleSheet("");
        }
        return;
    }

    m_dataInfoLabel->setText("0 rows x 0 cols");
    m_dataInfoLabel->setStyleSheet("");
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QProgressBar>
#include <QSharedPointer>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QTabWidget>
#include <QToolBar>
#include <QUndoStack>

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMenu;
class DataTableView;
class ChartView;
class StatisticsDialog;
class SettingsDialog;
class FilterDialog;
class CalcColumnDialog;
class TableDataModel;

namespace Core {
class TableData;
}

struct DocumentInfo {
    QString filePath;
    QString fileName;
    QSharedPointer<TableDataModel> model;
    QSharedPointer<QUndoStack> undoStack;
    bool unsavedChanges;
    int currentChartColumn;
    bool hasActiveFilter;
    int filterColumn;
    int filterCondition;
    QString filterValue;
    int sortColumn;
    Qt::SortOrder sortOrder;

    DocumentInfo()
        : unsavedChanges(false)
        , currentChartColumn(-1)
        , hasActiveFilter(false)
        , filterColumn(-1)
        , filterCondition(0)
        , sortColumn(-1)
        , sortOrder(Qt::AscendingOrder)
    {
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool openFile(const QString &filePath);
    bool saveFile(const QString &filePath = QString());
    bool saveAsExcel(const QString &filePath = QString(), bool visibleOnly = false);
    bool closeFile();

    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool unsaved);

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onOpenFile();
    void onSaveFile();
    bool onSaveAsFile();
    void onExportAsExcel();
    void onCloseFile();
    void onExit();

    void onOpenRecentFile();
    void onClearRecentFiles();

    void onDocumentListItemChanged(int currentRow);
    void onDocumentListCloseRequested();
    void onCloseCurrentDocument();

    void onUndo();
    void onRedo();
    void onCopy();
    void onPaste();
    void onFind();
    void onFindNext();
    void onGoToCell();
    void onSelectAll();

    void onFilterData();
    void onCalcColumn();

    void onToggleSidebar();
    void onZoomIn();
    void onZoomOut();
    void onResetZoom();

    void onAbout();

    void onStatistics();
    void onSettings();

    void updateWindowTitle();
    void onDataChanged();
    void onViewChanged();
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

    void updateRecentFilesMenu();
    void addRecentFile(const QString &filePath);
    void loadRecentFiles();
    void saveRecentFiles();
    void clearRecentFiles();

    void updateDocumentList();
    int findDocument(const QString &filePath);
    bool switchToDocument(int index);
    void closeDocument(int index);
    void closeAllDocuments();
    DocumentInfo *currentDocument();
    void syncCurrentDocumentState();
    void refreshChartColumnList();
    void refreshCurrentChart();
    void updateDataInfoLabel();
    void applyZoom();

    QTabWidget *m_tabWidget = nullptr;
    DataTableView *m_dataTableView = nullptr;
    ChartView *m_chartView = nullptr;

    QSplitter *m_mainSplitter = nullptr;
    QWidget *m_sidebarWidget = nullptr;

    QMenu *m_fileMenu = nullptr;
    QMenu *m_editMenu = nullptr;
    QMenu *m_viewMenu = nullptr;
    QMenu *m_helpMenu = nullptr;
    QMenu *m_recentFilesMenu = nullptr;

    QToolBar *m_fileToolBar = nullptr;
    QToolBar *m_editToolBar = nullptr;

    QLabel *m_statusLabel = nullptr;
    QLabel *m_fileInfoLabel = nullptr;
    QLabel *m_selectionInfoLabel = nullptr;
    QLabel *m_dataInfoLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;

    QListWidget *m_fileListWidget = nullptr;
    QListWidget *m_chartTypeWidget = nullptr;

    QUndoStack *m_undoStack = nullptr;
    QString m_currentFilePath;
    QString m_lastSearchText;
    bool m_unsavedChanges = false;

    QList<QString> m_recentFiles;
    static const int MAX_RECENT_FILES = 10;

    QList<QSharedPointer<DocumentInfo>> m_documents;
    int m_currentDocumentIndex;
    QSharedPointer<Core::TableData> m_chartDataSnapshot;

    double m_zoomLevel = 1.0;
    double m_baseFontSize = 10.0;
    static constexpr double MIN_ZOOM = 0.6;
    static constexpr double MAX_ZOOM = 2.0;
    static constexpr double ZOOM_STEP = 0.1;

    StatisticsDialog *m_statisticsDialog = nullptr;
};

#endif // MAINWINDOW_H

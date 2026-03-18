#ifndef DATATABLEVIEW_H
#define DATATABLEVIEW_H

#include <QMenu>
#include <QSharedPointer>
#include <QStyledItemDelegate>
#include <QTableView>

#include "../core/TableData.h"

class QAbstractItemModel;
class QStandardItemModel;
class TableFilterProxyModel;

class TableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TableItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class DataTableView : public QTableView
{
    Q_OBJECT

public:
    explicit DataTableView(QWidget *parent = nullptr);
    ~DataTableView() override;

    bool loadFile(const QString &filePath);
    bool saveFile(const QString &filePath);
    void clearData();
    void setTableData(const QSharedPointer<Core::TableData> &data);

    bool hasData() const;
    int totalRowCount() const;
    int visibleRowCount() const;
    int dataColumnCount() const;

    Core::TableData *tableData() const;
    QSharedPointer<Core::TableData> snapshotData(bool visibleOnly = false) const;

    void applyFilter(int column, int condition, const QString &value);
    void clearFilter();
    bool hasActiveFilter() const;

    QString selectedRangeInfo() const;

    struct SelectionStats {
        int count = 0;
        double sum = 0.0;
        double mean = 0.0;
        double min = 0.0;
        double max = 0.0;
        bool hasNumericData = false;
    };

    SelectionStats calculateSelectionStats() const;
    QString getSelectionStatsText() const;

    void resizeColumnsToContents();
    void autoResizeColumns();
    void copySelection();

signals:
    void dataChanged();
    void selectionChanged();
    void viewChanged();
    void fileLoaded(const QString &filePath);

private slots:
    void onContextMenuRequested(const QPoint &pos);
    void onHeaderClicked(int column);

private:
    void setupContextMenu();
    void populateModelFromTableData(const Core::TableData *data);
    QSharedPointer<Core::TableData> buildSnapshot(QAbstractItemModel *model) const;
    void invalidateSnapshots() const;
    void connectSourceModelSignals();
    void sortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);
    bool isNumericColumn(int column) const;

    QStandardItemModel *m_sourceModel;
    TableFilterProxyModel *m_proxyModel;
    QMenu *m_contextMenu;

    mutable QSharedPointer<Core::TableData> m_fullSnapshotCache;
    mutable QSharedPointer<Core::TableData> m_visibleSnapshotCache;
    mutable bool m_snapshotCacheDirty = true;
    bool m_bulkUpdating = false;

    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

#endif // DATATABLEVIEW_H

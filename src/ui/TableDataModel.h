#ifndef TABLEDATAMODEL_H
#define TABLEDATAMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>
#include <QStringList>

#include "core/TableData.h"

class QUndoStack;

class TableDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableDataModel(QObject *parent = nullptr);
    explicit TableDataModel(const QSharedPointer<Core::TableData> &data, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setTableData(const QSharedPointer<Core::TableData> &data);
    QSharedPointer<Core::TableData> tableData() const;
    void setUndoStack(QUndoStack *undoStack);
    QUndoStack *undoStack() const;
    int currentSortColumn() const;
    Qt::SortOrder currentSortOrder() const;
    void setCurrentSortState(int column, Qt::SortOrder order);

    QVariant cellValue(int row, int column) const;
    QString headerValue(int section) const;
    QVector<QVector<QVariant>> rowsSnapshot(int row, int count) const;

    bool setCellValueDirect(int row, int column, const QVariant &value);
    bool setHeaderValueDirect(int section, const QString &value);
    bool insertRowsDirect(int row, int count);
    bool insertColumnsDirect(int column, int count);
    bool removeRowsDirect(int row, int count);
    bool removeColumnsDirect(int column, int count);
    bool restoreRowsDirect(int row, const QVector<QVector<QVariant>> &rows);
    bool restoreColumnsDirect(int column, const QStringList &headers,
                              const QVector<QVector<QVariant>> &columnValues);
    bool applyRowOrderDirect(const QVector<QVector<QVariant>> &rows,
                             int sortColumn,
                             Qt::SortOrder sortOrder);

signals:
    void sortStateChanged(int column, Qt::SortOrder order);

private:
    void ensureData();

    QSharedPointer<Core::TableData> m_tableData;
    QUndoStack *m_undoStack = nullptr;
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

#endif // TABLEDATAMODEL_H

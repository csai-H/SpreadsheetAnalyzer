#ifndef TABLEDATAMODEL_H
#define TABLEDATAMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "core/TableData.h"

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
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setTableData(const QSharedPointer<Core::TableData> &data);
    QSharedPointer<Core::TableData> tableData() const;

private:
    void ensureData();

    QSharedPointer<Core::TableData> m_tableData;
};

#endif // TABLEDATAMODEL_H

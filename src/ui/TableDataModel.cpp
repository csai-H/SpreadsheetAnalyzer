#include "TableDataModel.h"

#include <QVector>

#include <algorithm>

TableDataModel::TableDataModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_tableData(QSharedPointer<Core::TableData>::create())
{
}

TableDataModel::TableDataModel(const QSharedPointer<Core::TableData> &data, QObject *parent)
    : QAbstractTableModel(parent)
    , m_tableData(data ? data : QSharedPointer<Core::TableData>::create())
{
}

int TableDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_tableData) {
        return 0;
    }

    return m_tableData->rowCount();
}

int TableDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_tableData) {
        return 0;
    }

    return m_tableData->columnCount();
}

QVariant TableDataModel::data(const QModelIndex &index, int role) const
{
    if (!m_tableData || !index.isValid()) {
        return {};
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_tableData->at(index.row(), index.column());
    }

    return {};
}

bool TableDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!m_tableData || !index.isValid() || role != Qt::EditRole) {
        return false;
    }

    m_tableData->set(index.row(), index.column(), value);
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

QVariant TableDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!m_tableData || role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        return m_tableData->header(section);
    }

    return section + 1;
}

bool TableDataModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (!m_tableData || orientation != Qt::Horizontal || role != Qt::EditRole) {
        return false;
    }

    m_tableData->setHeader(section, value.toString());
    emit headerDataChanged(orientation, section, section);
    return true;
}

Qt::ItemFlags TableDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool TableDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0) {
        return false;
    }

    ensureData();

    const int oldRowCount = m_tableData->rowCount();
    const int columnCount = m_tableData->columnCount();
    row = qBound(0, row, oldRowCount);

    beginInsertRows(QModelIndex(), row, row + count - 1);

    const QVector<QVector<QVariant>> trailingRows = [&]() {
        QVector<QVector<QVariant>> rows;
        rows.reserve(oldRowCount - row);
        for (int currentRow = row; currentRow < oldRowCount; ++currentRow) {
            rows.append(m_tableData->getRow(currentRow));
        }
        return rows;
    }();

    m_tableData->resize(oldRowCount + count, columnCount);

    for (int currentRow = oldRowCount - 1; currentRow >= row; --currentRow) {
        m_tableData->setRow(currentRow + count, trailingRows[currentRow - row]);
    }

    QVector<QVariant> emptyRow(columnCount);
    for (int currentRow = row; currentRow < row + count; ++currentRow) {
        m_tableData->setRow(currentRow, emptyRow);
    }

    endInsertRows();
    return true;
}

bool TableDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0) {
        return false;
    }

    ensureData();

    const int rowCount = m_tableData->rowCount();
    const int oldColumnCount = m_tableData->columnCount();
    column = qBound(0, column, oldColumnCount);

    beginInsertColumns(QModelIndex(), column, column + count - 1);

    const QStringList oldHeaders = m_tableData->headers();
    QVector<QVector<QVariant>> oldRows;
    oldRows.reserve(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        oldRows.append(m_tableData->getRow(row));
    }

    m_tableData->resize(rowCount, oldColumnCount + count);

    for (int currentColumn = 0; currentColumn < column; ++currentColumn) {
        m_tableData->setHeader(currentColumn, oldHeaders.value(currentColumn));
    }
    for (int currentColumn = column; currentColumn < oldColumnCount; ++currentColumn) {
        m_tableData->setHeader(currentColumn + count, oldHeaders.value(currentColumn));
    }

    for (int row = 0; row < rowCount; ++row) {
        QVector<QVariant> newRow(oldColumnCount + count);
        const QVector<QVariant> &oldRow = oldRows[row];

        for (int currentColumn = 0; currentColumn < column; ++currentColumn) {
            newRow[currentColumn] = oldRow.value(currentColumn);
        }
        for (int currentColumn = column; currentColumn < oldColumnCount; ++currentColumn) {
            newRow[currentColumn + count] = oldRow.value(currentColumn);
        }

        m_tableData->setRow(row, newRow);
    }

    endInsertColumns();
    return true;
}

bool TableDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!m_tableData || parent.isValid() || count <= 0 || row < 0 || row >= m_tableData->rowCount()) {
        return false;
    }

    const int oldRowCount = m_tableData->rowCount();
    const int columnCount = m_tableData->columnCount();
    const int lastRow = qMin(row + count, oldRowCount);
    count = lastRow - row;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int currentRow = row; currentRow < oldRowCount - count; ++currentRow) {
        m_tableData->setRow(currentRow, m_tableData->getRow(currentRow + count));
    }

    m_tableData->resize(oldRowCount - count, columnCount);

    endRemoveRows();
    return true;
}

void TableDataModel::sort(int column, Qt::SortOrder order)
{
    if (!m_tableData || column < 0 || column >= m_tableData->columnCount() || m_tableData->rowCount() <= 1) {
        return;
    }

    QVector<QVector<QVariant>> rows;
    rows.reserve(m_tableData->rowCount());
    for (int row = 0; row < m_tableData->rowCount(); ++row) {
        rows.append(m_tableData->getRow(row));
    }

    const auto compare = [column, order](const QVector<QVariant> &left, const QVector<QVariant> &right) {
        const QVariant &leftValue = left[column];
        const QVariant &rightValue = right[column];

        bool leftOk = false;
        bool rightOk = false;
        const double leftNumber = leftValue.toString().trimmed().toDouble(&leftOk);
        const double rightNumber = rightValue.toString().trimmed().toDouble(&rightOk);

        if (leftOk && rightOk) {
            return order == Qt::AscendingOrder ? leftNumber < rightNumber : leftNumber > rightNumber;
        }

        const QString leftText = leftValue.toString();
        const QString rightText = rightValue.toString();
        return order == Qt::AscendingOrder ? leftText < rightText : leftText > rightText;
    };

    emit layoutAboutToBeChanged();
    std::stable_sort(rows.begin(), rows.end(), compare);
    for (int row = 0; row < rows.size(); ++row) {
        m_tableData->setRow(row, rows[row]);
    }
    emit layoutChanged();
}

void TableDataModel::setTableData(const QSharedPointer<Core::TableData> &data)
{
    beginResetModel();
    m_tableData = data ? data : QSharedPointer<Core::TableData>::create();
    endResetModel();
}

QSharedPointer<Core::TableData> TableDataModel::tableData() const
{
    return m_tableData;
}

void TableDataModel::ensureData()
{
    if (m_tableData.isNull()) {
        m_tableData = QSharedPointer<Core::TableData>::create();
    }
}

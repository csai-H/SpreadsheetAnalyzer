#include "TableDataModel.h"

#include <QUndoCommand>
#include <QUndoStack>
#include <QVector>

#include <algorithm>

namespace {

class EditCellCommand final : public QUndoCommand
{
public:
    EditCellCommand(TableDataModel *model,
                    int row,
                    int column,
                    const QVariant &oldValue,
                    const QVariant &newValue)
        : m_model(model)
        , m_row(row)
        , m_column(column)
        , m_oldValue(oldValue)
        , m_newValue(newValue)
    {
        setText(QObject::tr("编辑单元格"));
    }

    void undo() override
    {
        m_model->setCellValueDirect(m_row, m_column, m_oldValue);
    }

    void redo() override
    {
        m_model->setCellValueDirect(m_row, m_column, m_newValue);
    }

private:
    TableDataModel *m_model;
    int m_row;
    int m_column;
    QVariant m_oldValue;
    QVariant m_newValue;
};

class EditHeaderCommand final : public QUndoCommand
{
public:
    EditHeaderCommand(TableDataModel *model,
                      int section,
                      const QString &oldValue,
                      const QString &newValue)
        : m_model(model)
        , m_section(section)
        , m_oldValue(oldValue)
        , m_newValue(newValue)
    {
        setText(QObject::tr("重命名列"));
    }

    void undo() override
    {
        m_model->setHeaderValueDirect(m_section, m_oldValue);
    }

    void redo() override
    {
        m_model->setHeaderValueDirect(m_section, m_newValue);
    }

private:
    TableDataModel *m_model;
    int m_section;
    QString m_oldValue;
    QString m_newValue;
};

class InsertRowsCommand final : public QUndoCommand
{
public:
    InsertRowsCommand(TableDataModel *model, int row, int count)
        : m_model(model)
        , m_row(row)
        , m_count(count)
    {
        setText(QObject::tr("插入行"));
    }

    void undo() override
    {
        m_model->removeRowsDirect(m_row, m_count);
    }

    void redo() override
    {
        m_model->insertRowsDirect(m_row, m_count);
    }

private:
    TableDataModel *m_model;
    int m_row;
    int m_count;
};

class InsertColumnsCommand final : public QUndoCommand
{
public:
    InsertColumnsCommand(TableDataModel *model, int column, int count)
        : m_model(model)
        , m_column(column)
        , m_count(count)
    {
        setText(QObject::tr("插入列"));
    }

    void undo() override
    {
        m_model->removeColumnsDirect(m_column, m_count);
    }

    void redo() override
    {
        m_model->insertColumnsDirect(m_column, m_count);
    }

private:
    TableDataModel *m_model;
    int m_column;
    int m_count;
};

class RemoveRowsCommand final : public QUndoCommand
{
public:
    RemoveRowsCommand(TableDataModel *model, int row, int count)
        : m_model(model)
        , m_row(row)
        , m_rows(model->rowsSnapshot(row, count))
    {
        setText(QObject::tr("删除行"));
    }

    void undo() override
    {
        m_model->restoreRowsDirect(m_row, m_rows);
    }

    void redo() override
    {
        m_model->removeRowsDirect(m_row, m_rows.size());
    }

private:
    TableDataModel *m_model;
    int m_row;
    QVector<QVector<QVariant>> m_rows;
};

class RemoveColumnsCommand final : public QUndoCommand
{
public:
    RemoveColumnsCommand(TableDataModel *model, int column, int count)
        : m_model(model)
        , m_column(column)
    {
        const auto tableData = model->tableData();
        const int columnCount = tableData ? tableData->columnCount() : 0;
        const int lastColumn = qMin(column + count, columnCount);
        for (int currentColumn = column; currentColumn < lastColumn; ++currentColumn) {
            m_headers.append(model->headerValue(currentColumn));
            m_columnValues.append(tableData->getColumn(currentColumn));
        }
        setText(QObject::tr("删除列"));
    }

    void undo() override
    {
        m_model->restoreColumnsDirect(m_column, m_headers, m_columnValues);
    }

    void redo() override
    {
        m_model->removeColumnsDirect(m_column, m_headers.size());
    }

private:
    TableDataModel *m_model;
    int m_column;
    QStringList m_headers;
    QVector<QVector<QVariant>> m_columnValues;
};

} // namespace

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

    const QVariant oldValue = m_tableData->at(index.row(), index.column());
    if (oldValue == value) {
        return true;
    }

    if (m_undoStack) {
        m_undoStack->push(new EditCellCommand(this, index.row(), index.column(), oldValue, value));
        return true;
    }

    return setCellValueDirect(index.row(), index.column(), value);
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

    const QString newHeader = value.toString();
    const QString oldHeader = m_tableData->header(section);
    if (newHeader == oldHeader) {
        return true;
    }

    if (m_undoStack) {
        m_undoStack->push(new EditHeaderCommand(this, section, oldHeader, newHeader));
        return true;
    }

    return setHeaderValueDirect(section, newHeader);
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

    if (m_undoStack) {
        m_undoStack->push(new InsertRowsCommand(this, row, count));
        return true;
    }

    return insertRowsDirect(row, count);
}

bool TableDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (parent.isValid() || count <= 0) {
        return false;
    }

    if (m_undoStack) {
        m_undoStack->push(new InsertColumnsCommand(this, column, count));
        return true;
    }

    return insertColumnsDirect(column, count);
}

bool TableDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!m_tableData || parent.isValid() || count <= 0 || row < 0 || row >= m_tableData->rowCount()) {
        return false;
    }

    if (m_undoStack) {
        m_undoStack->push(new RemoveRowsCommand(this, row, count));
        return true;
    }

    return removeRowsDirect(row, count);
}

bool TableDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (!m_tableData || parent.isValid() || count <= 0 || column < 0 || column >= m_tableData->columnCount()) {
        return false;
    }

    if (m_undoStack) {
        m_undoStack->push(new RemoveColumnsCommand(this, column, count));
        return true;
    }

    return removeColumnsDirect(column, count);
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

void TableDataModel::setUndoStack(QUndoStack *undoStack)
{
    m_undoStack = undoStack;
}

QUndoStack *TableDataModel::undoStack() const
{
    return m_undoStack;
}

QVariant TableDataModel::cellValue(int row, int column) const
{
    if (!m_tableData || row < 0 || column < 0 ||
        row >= m_tableData->rowCount() || column >= m_tableData->columnCount()) {
        return {};
    }

    return m_tableData->at(row, column);
}

QString TableDataModel::headerValue(int section) const
{
    if (!m_tableData || section < 0 || section >= m_tableData->columnCount()) {
        return {};
    }

    return m_tableData->header(section);
}

QVector<QVector<QVariant>> TableDataModel::rowsSnapshot(int row, int count) const
{
    QVector<QVector<QVariant>> rows;
    if (!m_tableData || count <= 0 || row < 0 || row >= m_tableData->rowCount()) {
        return rows;
    }

    const int lastRow = qMin(row + count, m_tableData->rowCount());
    rows.reserve(lastRow - row);
    for (int currentRow = row; currentRow < lastRow; ++currentRow) {
        rows.append(m_tableData->getRow(currentRow));
    }
    return rows;
}

bool TableDataModel::setCellValueDirect(int row, int column, const QVariant &value)
{
    if (!m_tableData || row < 0 || column < 0 ||
        row >= m_tableData->rowCount() || column >= m_tableData->columnCount()) {
        return false;
    }

    const QModelIndex modelIndex = index(row, column);
    m_tableData->set(row, column, value);
    emit dataChanged(modelIndex, modelIndex, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

bool TableDataModel::setHeaderValueDirect(int section, const QString &value)
{
    if (!m_tableData || section < 0 || section >= m_tableData->columnCount()) {
        return false;
    }

    m_tableData->setHeader(section, value);
    emit headerDataChanged(Qt::Horizontal, section, section);
    return true;
}

bool TableDataModel::insertRowsDirect(int row, int count)
{
    if (count <= 0) {
        return false;
    }

    ensureData();

    const int oldRowCount = m_tableData->rowCount();
    const int columnCount = m_tableData->columnCount();
    row = qBound(0, row, oldRowCount);

    beginInsertRows(QModelIndex(), row, row + count - 1);

    QVector<QVector<QVariant>> trailingRows;
    trailingRows.reserve(oldRowCount - row);
    for (int currentRow = row; currentRow < oldRowCount; ++currentRow) {
        trailingRows.append(m_tableData->getRow(currentRow));
    }

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

bool TableDataModel::insertColumnsDirect(int column, int count)
{
    if (count <= 0) {
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

bool TableDataModel::removeRowsDirect(int row, int count)
{
    if (!m_tableData || count <= 0 || row < 0 || row >= m_tableData->rowCount()) {
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

bool TableDataModel::removeColumnsDirect(int column, int count)
{
    if (!m_tableData || count <= 0 || column < 0 || column >= m_tableData->columnCount()) {
        return false;
    }

    const int rowCount = m_tableData->rowCount();
    const int oldColumnCount = m_tableData->columnCount();
    const int lastColumn = qMin(column + count, oldColumnCount);
    count = lastColumn - column;

    beginRemoveColumns(QModelIndex(), column, column + count - 1);

    const QStringList oldHeaders = m_tableData->headers();
    QVector<QVector<QVariant>> oldRows;
    oldRows.reserve(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        oldRows.append(m_tableData->getRow(row));
    }

    m_tableData->resize(rowCount, oldColumnCount - count);

    for (int currentColumn = 0; currentColumn < column; ++currentColumn) {
        m_tableData->setHeader(currentColumn, oldHeaders.value(currentColumn));
    }
    for (int currentColumn = column + count; currentColumn < oldColumnCount; ++currentColumn) {
        m_tableData->setHeader(currentColumn - count, oldHeaders.value(currentColumn));
    }

    for (int row = 0; row < rowCount; ++row) {
        QVector<QVariant> newRow(oldColumnCount - count);
        const QVector<QVariant> &oldRow = oldRows[row];

        for (int currentColumn = 0; currentColumn < column; ++currentColumn) {
            newRow[currentColumn] = oldRow.value(currentColumn);
        }
        for (int currentColumn = column + count; currentColumn < oldColumnCount; ++currentColumn) {
            newRow[currentColumn - count] = oldRow.value(currentColumn);
        }

        m_tableData->setRow(row, newRow);
    }

    endRemoveColumns();
    return true;
}

bool TableDataModel::restoreRowsDirect(int row, const QVector<QVector<QVariant>> &rows)
{
    if (rows.isEmpty()) {
        return false;
    }

    if (!insertRowsDirect(row, rows.size())) {
        return false;
    }

    for (int offset = 0; offset < rows.size(); ++offset) {
        m_tableData->setRow(row + offset, rows.at(offset));
    }

    if (columnCount() > 0) {
        emit dataChanged(index(row, 0),
                         index(row + rows.size() - 1, columnCount() - 1),
                         {Qt::DisplayRole, Qt::EditRole});
    }
    return true;
}

bool TableDataModel::restoreColumnsDirect(int column,
                                          const QStringList &headers,
                                          const QVector<QVector<QVariant>> &columnValues)
{
    if (headers.isEmpty() || headers.size() != columnValues.size()) {
        return false;
    }

    if (!insertColumnsDirect(column, headers.size())) {
        return false;
    }

    for (int offset = 0; offset < headers.size(); ++offset) {
        m_tableData->setHeader(column + offset, headers.at(offset));
        const QVector<QVariant> &values = columnValues.at(offset);
        for (int row = 0; row < values.size() && row < rowCount(); ++row) {
            m_tableData->set(row, column + offset, values.at(row));
        }
    }

    emit headerDataChanged(Qt::Horizontal, column, column + headers.size() - 1);
    if (rowCount() > 0) {
        emit dataChanged(index(0, column),
                         index(rowCount() - 1, column + headers.size() - 1),
                         {Qt::DisplayRole, Qt::EditRole});
    }
    return true;
}

void TableDataModel::ensureData()
{
    if (m_tableData.isNull()) {
        m_tableData = QSharedPointer<Core::TableData>::create();
    }
}

#include "TableData.h"
#include <QDebug>
#include <QMetaType>

namespace Core {

// PIMPL 实现
struct TableData::Impl
{
    QVector<QVector<QVariant>> m_data;
    QStringList m_headers;
    int m_rowCount = 0;
    int m_columnCount = 0;
};

TableData::TableData()
    : m_impl(std::make_unique<Impl>())
{
}

TableData::TableData(int rows, int columns)
    : TableData()
{
    resize(rows, columns);
}

TableData::~TableData() = default;

// === 数据访问 ===
QVariant TableData::at(int row, int column) const
{
    if (row < 0 || row >= m_impl->m_rowCount ||
        column < 0 || column >= m_impl->m_columnCount) {
        qWarning() << "TableData::at: Index out of range:" << row << column;
        return QVariant();
    }

    return m_impl->m_data[row][column];
}

void TableData::set(int row, int column, const QVariant& value)
{
    if (row < 0 || row >= m_impl->m_rowCount ||
        column < 0 || column >= m_impl->m_columnCount) {
        qWarning() << "TableData::set: Index out of range:" << row << column;
        return;
    }

    m_impl->m_data[row][column] = value;
}

// === 维度信息 ===
int TableData::rowCount() const
{
    return m_impl->m_rowCount;
}

int TableData::columnCount() const
{
    return m_impl->m_columnCount;
}

bool TableData::isEmpty() const
{
    return m_impl->m_rowCount == 0 || m_impl->m_columnCount == 0;
}

// === 维度调整 ===
void TableData::resize(int rows, int columns)
{
    if (rows < 0 || columns < 0) {
        qWarning() << "TableData::resize: Invalid dimensions:" << rows << columns;
        return;
    }

    m_impl->m_data.resize(rows);
    for (int i = 0; i < rows; ++i) {
        m_impl->m_data[i].resize(columns);
    }

    m_impl->m_rowCount = rows;
    m_impl->m_columnCount = columns;

    // 调整表头
    while (m_impl->m_headers.size() < columns) {
        m_impl->m_headers.append(QString("Column %1").arg(m_impl->m_headers.size() + 1));
    }
    m_impl->m_headers.resize(columns);
}

void TableData::clear()
{
    m_impl->m_data.clear();
    m_impl->m_headers.clear();
    m_impl->m_rowCount = 0;
    m_impl->m_columnCount = 0;
}

// === 表头 ===
void TableData::setHeader(int column, const QString& name)
{
    if (column < 0 || column >= m_impl->m_columnCount) {
        qWarning() << "TableData::setHeader: Column index out of range:" << column;
        return;
    }

    m_impl->m_headers[column] = name;
}

QString TableData::header(int column) const
{
    if (column < 0 || column >= m_impl->m_columnCount) {
        qWarning() << "TableData::header: Column index out of range:" << column;
        return QString();
    }

    return m_impl->m_headers[column];
}

QStringList TableData::headers() const
{
    return m_impl->m_headers;
}

// === 行列访问（批量）===
QVector<QVariant> TableData::getRow(int row) const
{
    if (row < 0 || row >= m_impl->m_rowCount) {
        qWarning() << "TableData::getRow: Row index out of range:" << row;
        return QVector<QVariant>();
    }

    return m_impl->m_data[row];
}

QVector<QVariant> TableData::getColumn(int column) const
{
    if (column < 0 || column >= m_impl->m_columnCount) {
        qWarning() << "TableData::getColumn: Column index out of range:" << column;
        return QVector<QVariant>();
    }

    QVector<QVariant> result;
    result.reserve(m_impl->m_rowCount);

    for (int row = 0; row < m_impl->m_rowCount; ++row) {
        result.append(m_impl->m_data[row][column]);
    }

    return result;
}

void TableData::setRow(int row, const QVector<QVariant>& values)
{
    if (row < 0 || row >= m_impl->m_rowCount) {
        qWarning() << "TableData::setRow: Row index out of range:" << row;
        return;
    }

    if (values.size() != m_impl->m_columnCount) {
        qWarning() << "TableData::setRow: Values size mismatch:" << values.size();
        return;
    }

    m_impl->m_data[row] = values;
}

void TableData::setColumn(int column, const QVector<QVariant>& values)
{
    if (column < 0 || column >= m_impl->m_columnCount) {
        qWarning() << "TableData::setColumn: Column index out of range:" << column;
        return;
    }

    if (values.size() != m_impl->m_rowCount) {
        qWarning() << "TableData::setColumn: Values size mismatch:" << values.size();
        return;
    }

    for (int row = 0; row < m_impl->m_rowCount; ++row) {
        m_impl->m_data[row][column] = values[row];
    }
}

// === 数据类型处理 ===
bool TableData::isNumeric(int row, int column) const
{
    QVariant value = at(row, column);
    return !value.isNull() && (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Int || value.canConvert<double>());
}

bool TableData::isNumeric(int column) const
{
    for (int row = 0; row < m_impl->m_rowCount; ++row) {
        if (!isNumeric(row, column)) {
            return false;
        }
    }
    return true;
}

double TableData::toDouble(int row, int column) const
{
    QVariant value = at(row, column);
    return value.toDouble();
}

QVector<double> TableData::toDoubleVector(int column) const
{
    QVector<QVariant> columnData = getColumn(column);
    QVector<double> result;

    for (const QVariant& value : columnData) {
        if (value.canConvert<double>()) {
            result.append(value.toDouble());
        } else {
            result.append(std::numeric_limits<double>::quiet_NaN());
        }
    }

    return result;
}

// === 克隆 ===
TableData* TableData::clone() const
{
    TableData* copy = new TableData(m_impl->m_rowCount, m_impl->m_columnCount);

    copy->m_impl->m_data = m_impl->m_data;
    copy->m_impl->m_headers = m_impl->m_headers;

    return copy;
}

} // namespace Core

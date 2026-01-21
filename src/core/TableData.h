#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QVector>
#include <QStringList>
#include <QVariant>
#include <memory>

namespace Core {

/**
 * @brief 表格数据模型
 *
 * 存储二维表格数据，支持高效的行列访问
 */
class TableData
{
public:
    explicit TableData();
    explicit TableData(int rows, int columns);
    ~TableData();

    // === 数据访问 ===
    QVariant at(int row, int column) const;
    void set(int row, int column, const QVariant& value);

    // === 维度信息 ===
    int rowCount() const;
    int columnCount() const;
    bool isEmpty() const;

    // === 维度调整 ===
    void resize(int rows, int columns);
    void clear();

    // === 表头 ===
    void setHeader(int column, const QString& name);
    QString header(int column) const;
    QStringList headers() const;

    // === 行列访问（批量）===
    QVector<QVariant> getRow(int row) const;
    QVector<QVariant> getColumn(int column) const;
    void setRow(int row, const QVector<QVariant>& values);
    void setColumn(int column, const QVector<QVariant>& values);

    // === 数据类型处理 ===
    bool isNumeric(int row, int column) const;
    bool isNumeric(int column) const;
    double toDouble(int row, int column) const;
    QVector<double> toDoubleVector(int column) const;

    // === 克隆 ===
    TableData* clone() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Core

#endif // TABLEDATA_H

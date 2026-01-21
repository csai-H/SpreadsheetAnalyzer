#include "CsvLoader.h"
#include "TableData.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

struct CsvLoader::Impl
{
    QChar delimiter = ',';
    bool hasHeader = true;
    bool trimWhitespace = true;
    QString encoding = "UTF-8";
};

CsvLoader::CsvLoader(QObject* parent)
{
    Q_UNUSED(parent);
    m_impl = new Impl();
}

CsvLoader::~CsvLoader()
{
    delete m_impl;
}

void CsvLoader::setDelimiter(const QChar& delimiter)
{
    m_impl->delimiter = delimiter;
}

void CsvLoader::setHasHeader(bool hasHeader)
{
    m_impl->hasHeader = hasHeader;
}

void CsvLoader::setTrimWhitespace(bool trim)
{
    m_impl->trimWhitespace = trim;
}

void CsvLoader::setEncoding(const QString& encoding)
{
    m_impl->encoding = encoding;
}

LoadResult CsvLoader::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return LoadResult::errorResult("无法打开文件: " + filePath);
    }

    QTextStream stream(&file);
    // Qt6: setEncoding expects QStringConverter::Encoding, use setAutoEncoding or skip
    // stream.setEncoding(m_impl->encoding.toUtf8());

    // 读取所有行
    QVector<QStringList> allRows;
    int maxColumns = 0;

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        if (m_impl->trimWhitespace) {
            line = line.trimmed();
        }

        if (line.isEmpty()) {
            continue;  // 跳过空行
        }

        QStringList values = parseCsvLine(line);
        allRows.append(values);

        if (values.size() > maxColumns) {
            maxColumns = values.size();
        }
    }

    file.close();

    if (allRows.isEmpty()) {
        return LoadResult::errorResult("文件为空或格式无效");
    }

    // 设置表头
    QStringList headers;
    if (m_impl->hasHeader && !allRows.isEmpty()) {
        headers = allRows.first();
        // 移除表头行，只保留数据
        allRows.removeFirst();
    }

    // 创建 TableData（使用移除表头后的行数）
    auto* tableData = new Core::TableData(allRows.size(), maxColumns);

    // 设置表头
    if (m_impl->hasHeader && !headers.isEmpty()) {
        for (int col = 0; col < headers.size() && col < maxColumns; ++col) {
            QString header = headers[col];
            if (header.isEmpty()) {
                header = QString("Column %1").arg(col + 1);
            }
            tableData->setHeader(col, header);
        }
    } else {
        // 如果没有表头，设置默认列名
        for (int col = 0; col < maxColumns; ++col) {
            tableData->setHeader(col, QString("Column %1").arg(col + 1));
        }
    }

    // 填充数据
    for (int row = 0; row < allRows.size(); ++row) {
        QStringList values = allRows[row];
        for (int col = 0; col < maxColumns; ++col) {
            QString value;
            if (col < values.size()) {
                value = values[col];
            }
            // 强制存储为字符串类型，防止自动转换为数值
            tableData->set(row, col, QVariant(value));
        }
    }

    return LoadResult::successResult(tableData);
}

QStringList CsvLoader::parseCsvLine(const QString& line)
{
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); ++i) {
        QChar ch = line[i];

        if (ch == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                // 转义的引号
                current += '"';
                ++i;
            } else {
                // 切换引号状态
                inQuotes = !inQuotes;
            }
        } else if (ch == m_impl->delimiter && !inQuotes) {
            // 分隔符
            result.append(current);
            current.clear();
        } else {
            current += ch;
        }
    }

    // 添加最后一个字段
    result.append(current);

    // 去除空白
    if (m_impl->trimWhitespace) {
        for (auto& value : result) {
            value = value.trimmed();
        }
    }

    return result;
}

bool CsvLoader::supports(const QString& filePath) const
{
    return filePath.endsWith(".csv", Qt::CaseInsensitive);
}

QStringList CsvLoader::supportedExtensions() const
{
    return QStringList() << "csv";
}

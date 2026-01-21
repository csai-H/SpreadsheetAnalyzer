#ifndef DATAEXPORTER_H
#define DATAEXPORTER_H

#include <QString>
#include "TableData.h"

/**
 * @brief 数据导出结果
 */
struct ExportResult
{
    bool success = false;
    QString errorMessage;
    QString exportedPath;

    static ExportResult successResult(const QString& path);
    static ExportResult errorResult(const QString& error);
};

/**
 * @brief 数据导出器
 */
class DataExporter
{
public:
    explicit DataExporter(QObject* parent = nullptr);
    ~DataExporter();

    // 导出为 CSV
    ExportResult exportToCsv(const Core::TableData* model, const QString& filePath,
                             const QChar& delimiter = ',',
                             const QString& encoding = "UTF-8");

    // 导出为图片（用于表格截图）
    ExportResult exportToImage(const Core::TableData* model, const QString& filePath,
                               int maxWidth = 1920);

private:
    struct Impl;
    Impl* m_impl;
};

#endif // DATAEXPORTER_H

#include "DataExporter.h"
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QPainter>
#include <QTableView>
#include <QHeaderView>

struct DataExporter::Impl
{
    QChar delimiter = ',';
    QString encoding = "UTF-8";
};

DataExporter::DataExporter(QObject* parent)
{
    Q_UNUSED(parent);
    m_impl = new Impl();
}

DataExporter::~DataExporter()
{
    delete m_impl;
}

ExportResult ExportResult::successResult(const QString& path)
{
    ExportResult result;
    result.success = true;
    result.exportedPath = path;
    return result;
}

ExportResult ExportResult::errorResult(const QString& error)
{
    ExportResult result;
    result.success = false;
    result.errorMessage = error;
    return result;
}

ExportResult DataExporter::exportToCsv(const Core::TableData* model, const QString& filePath,
                                        const QChar& delimiter,
                                        const QString& encoding)
{
    Q_UNUSED(encoding);  // TODO: 实现编码支持
    if (!model) {
        return ExportResult::errorResult("无效的数据模型");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return ExportResult::errorResult("无法创建文件: " + filePath);
    }

    QTextStream out(&file);
    // Qt6: setEncoding expects QStringConverter::Encoding
    // out.setEncoding(encoding.toUtf8());

    // 写入表头
    QStringList headers = model->headers();
    out << headers.join(delimiter) << "\n";

    // 写入数据
    for (int row = 0; row < model->rowCount(); ++row) {
        QStringList values;
        for (int col = 0; col < model->columnCount(); ++col) {
            QVariant value = model->at(row, col);
            values << value.toString();
        }
        out << values.join(delimiter) << "\n";
    }

    file.close();
    return ExportResult::successResult(filePath);
}

ExportResult DataExporter::exportToImage(const Core::TableData* model, const QString& filePath,
                                          int maxWidth)
{
    Q_UNUSED(model);
    Q_UNUSED(filePath);
    Q_UNUSED(maxWidth);

    // TODO: 实现表格导出为图片
    return ExportResult::errorResult("导出图片功能待实现");
}

#include "ExcelExporter.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>

bool ExcelExporter::exportToHtmlExcel(const Core::TableData* tableData, const QString& filePath)
{
    if (!tableData || tableData->isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    // Qt6: setEncoding expects QStringConverter::Encoding
    // out.setEncoding("UTF-8");

    // 写入HTML头部（Excel兼容格式）
    out << "<html xmlns:o='urn:schemas-microsoft-com:office:office'\n";
    out << "xmlns:x='urn:schemas-microsoft-com:office:excel'\n";
    out << "xmlns='http://www.w3.org/TR/REC-html40'>\n";
    out << "<head>\n";
    out << "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\n";
    out << "<style>\n";
    out << "table { border-collapse: collapse; }\n";
    out << "td, th { border: 1px solid #ccc; padding: 8px; text-align: center; }\n";
    out << "th { background-color: #4CAF50; color: white; }\n";
    out << "tr:nth-child(even) { background-color: #f2f2f2; }\n";
    out << "</style>\n";
    out << "</head>\n";
    out << "<body>\n";

    // 写入表格标题
    out << "<h2>" << (filePath.isEmpty() ? "数据导出" : filePath) << "</h2>\n";
    out << "<p>导出时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "</p>\n";

    // 写入表格
    out << "<table>\n";

    // 写入表头
    out << "<tr>\n";
    for (int col = 0; col < tableData->columnCount(); ++col) {
        QString header = tableData->header(col);
        out << "<th>" << header << "</th>\n";
    }
    out << "</tr>\n";

    // 写入数据行
    for (int row = 0; row < tableData->rowCount(); ++row) {
        out << "<tr>\n";
        for (int col = 0; col < tableData->columnCount(); ++col) {
            QVariant value = tableData->at(row, col);
            out << "<td>" << value.toString() << "</td>\n";
        }
        out << "</tr>\n";
    }

    out << "</table>\n";
    out << "</body>\n";
    out << "</html>\n";

    file.close();
    return true;
}

bool ExcelExporter::exportToExcel(const Core::TableData* tableData, const QString& filePath)
{
    // 使用HTML格式的Excel导出
    return exportToHtmlExcel(tableData, filePath);
}

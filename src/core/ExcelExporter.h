#ifndef EXCELEXPORTER_H
#define EXCELEXPORTER_H

#include "../core/TableData.h"
#include <QString>

/**
 * @brief Excel导出器
 *
 * 将数据导出为Excel文件
 */
class ExcelExporter
{
public:
    /**
     * @brief 将TableData导出为Excel文件
     * @param tableData 数据表
     * @param filePath 文件路径
     * @return 是否成功
     */
    static bool exportToExcel(const Core::TableData* tableData, const QString& filePath);

    /**
     * @brief 将TableData导出为Excel-compatible HTML文件
     * @param tableData 数据表
     * @param filePath 文件路径
     * @return 是否成功
     */
    static bool exportToHtmlExcel(const Core::TableData* tableData, const QString& filePath);
};

#endif // EXCELEXPORTER_H

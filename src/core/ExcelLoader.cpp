#include "ExcelLoader.h"
#include "TableData.h"
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <OpenXLSX.hpp>
#include <vector>

struct ExcelLoader::Impl
{
    int sheetIndex = 0;
    bool hasHeader = true;
    QString sheetName;
};

ExcelLoader::ExcelLoader(QObject* parent)
{
    Q_UNUSED(parent);
    m_impl = new Impl();
}

ExcelLoader::~ExcelLoader()
{
    delete m_impl;
}

void ExcelLoader::setSheetName(const QString& sheetName)
{
    m_impl->sheetName = sheetName;
}

void ExcelLoader::setSheetIndex(int index)
{
    m_impl->sheetIndex = index;
}

void ExcelLoader::setHasHeader(bool hasHeader)
{
    m_impl->hasHeader = hasHeader;
}

LoadResult ExcelLoader::load(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    // OpenXLSX 只支持 .xlsx 格式
    if (extension != "xlsx") {
        return LoadResult::errorResult("不支持的文件格式: " + extension + "\n请使用 .xlsx 格式的 Excel 文件。");
    }

    try {
        // 将 QString 转换为 std::string
        std::string filePathStd = fileInfo.absoluteFilePath().toStdString();

        // 打开 Excel 文件
        OpenXLSX::XLDocument doc;
        doc.open(filePathStd);
        auto wb = doc.workbook();

        // 获取工作表
        OpenXLSX::XLWorksheet worksheet;

        // 如果没有指定工作表名称，使用索引获取
        if (m_impl->sheetName.isEmpty()) {
            auto sheetNames = wb.worksheetNames();
            if (sheetNames.empty()) {
                doc.close();
                return LoadResult::errorResult("Excel 文件中没有工作表");
            }

            if (m_impl->sheetIndex >= 0 && m_impl->sheetIndex < static_cast<int>(sheetNames.size())) {
                worksheet = wb.worksheet(sheetNames[m_impl->sheetIndex]);
            } else {
                worksheet = wb.worksheet(sheetNames[0]);
            }
        } else {
            worksheet = wb.worksheet(m_impl->sheetName.toStdString());
        }

        // 获取工作表的数据范围
        auto range = worksheet.range();
        int maxRow = static_cast<int>(range.numRows());
        int maxCol = static_cast<int>(range.numColumns());

        if (maxRow == 0 || maxCol == 0) {
            doc.close();
            return LoadResult::errorResult("Excel 工作表为空");
        }

        // 首先扫描所有数据，找出实际有数据的行和列
        std::vector<std::vector<QString>> allData(maxRow, std::vector<QString>(maxCol));
        std::vector<bool> colHasData(maxCol, false);
        std::vector<bool> rowHasData(maxRow, false);

        // 读取所有单元格数据
        for (int row = 1; row <= maxRow; ++row) {
            for (int col = 1; col <= maxCol; ++col) {
                auto cell = worksheet.cell(row, col);
                QString cellValue;

                try {
                    auto& cellValueProxy = cell.value();
                    auto valueType = cellValueProxy.type();

                    if (valueType == OpenXLSX::XLValueType::String) {
                        cellValue = QString::fromStdString(cellValueProxy.get<std::string>());
                    } else if (valueType == OpenXLSX::XLValueType::Integer) {
                        cellValue = QString::number(cellValueProxy.get<int64_t>());
                    } else if (valueType == OpenXLSX::XLValueType::Float) {
                        cellValue = QString::number(cellValueProxy.get<double>());
                    } else if (valueType == OpenXLSX::XLValueType::Boolean) {
                        cellValue = cellValueProxy.get<bool>() ? "true" : "false";
                    }
                } catch (...) {
                    cellValue = "";
                }

                allData[row - 1][col - 1] = cellValue;

                // 记录哪些行和列有数据
                if (!cellValue.isEmpty()) {
                    colHasData[col - 1] = true;
                    rowHasData[row - 1] = true;
                }
            }
        }

        // 如果有表头，只包含表头不为空的列；否则包含所有有数据的列
        std::vector<int> validColumns; // 记录有效的列索引
        if (m_impl->hasHeader) {
            // 只包含表头不为空的列
            for (int col = 0; col < maxCol; ++col) {
                if (!allData[0][col].isEmpty()) {
                    validColumns.push_back(col);
                }
            }
        } else {
            // 包含所有有数据的列
            for (int col = 0; col < maxCol; ++col) {
                if (colHasData[col]) {
                    validColumns.push_back(col);
                }
            }
        }

        int actualDataCols = static_cast<int>(validColumns.size());
        if (actualDataCols == 0) {
            doc.close();
            return LoadResult::errorResult("Excel 工作表没有有效数据");
        }

        // 找出实际有数据的行数
        int actualDataRows = 0;
        for (int row = 0; row < maxRow; ++row) {
            if (rowHasData[row]) {
                actualDataRows = row + 1;
            }
        }

        if (actualDataCols == 0 || actualDataRows == 0) {
            doc.close();
            return LoadResult::errorResult("Excel 工作表没有有效数据");
        }

        // 计算数据行数（如果有表头，减去1行）
        int dataRowCount = m_impl->hasHeader ? (actualDataRows - 1) : actualDataRows;
        if (dataRowCount <= 0) {
            doc.close();
            return LoadResult::errorResult("Excel 文件没有数据行");
        }

        // 创建 TableData 对象（只包含实际有数据的列）
        auto* tableData = new Core::TableData(dataRowCount, actualDataCols);

        // 读取表头
        if (m_impl->hasHeader) {
            for (int i = 0; i < actualDataCols; ++i) {
                int col = validColumns[i];
                QString headerText = allData[0][col];
                tableData->setHeader(i, headerText);
            }
        } else {
            // 如果没有表头，生成默认列名
            for (int i = 0; i < actualDataCols; ++i) {
                tableData->setHeader(i, "Column" + QString::number(i + 1));
            }
        }

        // 读取数据行
        int startRow = m_impl->hasHeader ? 1 : 0;
        int dataRow = 0;  // TableData 中的行索引

        for (int row = startRow; row < actualDataRows; ++row) {
            for (int i = 0; i < actualDataCols; ++i) {
                int col = validColumns[i];
                QString value = allData[row][col];
                tableData->set(dataRow, i, QVariant(value));
            }
            dataRow++;
        }

        doc.close();

        return LoadResult::successResult(tableData);

    } catch (const std::exception& e) {
        return LoadResult::errorResult(
            QString("读取 Excel 文件失败: %1").arg(e.what())
        );
    } catch (...) {
        return LoadResult::errorResult("读取 Excel 文件时发生未知错误");
    }
}

bool ExcelLoader::supports(const QString& filePath) const
{
    return filePath.endsWith(".xlsx", Qt::CaseInsensitive);
}

QStringList ExcelLoader::supportedExtensions() const
{
    return QStringList() << "xlsx";
}

#include "DataTableView.h"
#include "../core/CsvLoader.h"
#include "../core/ExcelLoader.h"
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QLineEdit>

// ==================== TableItemDelegate 实现 ====================

QWidget *TableItemDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(true);
    editor->setAutoFillBackground(true);

    // 设置最小高度，避免文字被压缩
    editor->setMinimumHeight(30);

    return editor;
}

void TableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(text);
}

void TableItemDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    Q_UNUSED(index);

    // 设置编辑器的几何形状，确保有足够的高度
    QRect rect = option.rect;
    editor->setGeometry(rect);
}

// ==================== DataTableView 实现 ====================

DataTableView::DataTableView(QWidget *parent)
    : QTableView(parent)
    , m_model(new QStandardItemModel(this))
    , m_tableData(new Core::TableData())
{
    setModel(m_model);

    // 设置自定义委托，修复编辑器高度问题
    setItemDelegate(new TableItemDelegate(this));

    // 设置视图属性
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ContiguousSelection);

    // 设置表头
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSectionsClickable(true);
    horizontalHeader()->setFixedHeight(50);
    verticalHeader()->setDefaultSectionSize(30);
    verticalHeader()->setMinimumWidth(80);

    // 连接表头点击信号
    connect(horizontalHeader(), &QHeaderView::sectionClicked,
            this, &DataTableView::onHeaderClicked);

    // 连接选择变化信号（用于快速统计面板）
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() {
                emit selectionChanged();
            });

    // 设置排序指示器
    horizontalHeader()->setSortIndicatorShown(true);

    // 设置网格
    setShowGrid(true);
    setWordWrap(false);

    // 设置编辑器高度，避免文字被压缩
    verticalHeader()->setDefaultSectionSize(30);

    setupContextMenu();
}

DataTableView::~DataTableView()
{
    delete m_tableData;
}

void DataTableView::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    m_contextMenu->addAction("复制", this, [this]() {
        copySelection();
    });

    m_contextMenu->addSeparator();

    m_contextMenu->addAction("插入行", this, [this]() {
        QModelIndex current = currentIndex();
        m_model->insertRow(current.row());
        emit dataChanged();
    });

    m_contextMenu->addAction("删除行", this, [this]() {
        QModelIndex current = currentIndex();
        m_model->removeRow(current.row());
        emit dataChanged();
    });

    m_contextMenu->addSeparator();

    // 列宽调整菜单
    QMenu* columnMenu = m_contextMenu->addMenu("列宽调整");
    columnMenu->addAction("自动调整列宽", this, [this]() {
        autoResizeColumns();
    });
    columnMenu->addAction("根据内容调整", this, [this]() {
        resizeColumnsToContents();
    });
    columnMenu->addAction("重置为默认宽度", this, [this]() {
        for (int col = 0; col < m_model->columnCount(); ++col) {
            setColumnWidth(col, 100);
        }
    });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableView::customContextMenuRequested,
            this, &DataTableView::onContextMenuRequested);
}

bool DataTableView::loadFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    if (suffix == "csv") {
        // 使用 CsvLoader 加载
        CsvLoader loader;
        LoadResult result = loader.load(filePath);

        if (!result.success) {
            QMessageBox::warning(this, "错误", "加载CSV文件失败: " + result.errorMessage);
            return false;
        }

        // 更新 TableData
        delete m_tableData;
        m_tableData = result.data;

        // 更新 QStandardItemModel
        m_model->clear();
        m_model->setRowCount(m_tableData->rowCount());
        m_model->setColumnCount(m_tableData->columnCount());

        // 设置表头（使用 TableData 中的表头）
        for (int col = 0; col < m_tableData->columnCount(); ++col) {
            m_model->setHeaderData(col, Qt::Horizontal, m_tableData->header(col));
        }

        // 填充数据
        for (int row = 0; row < m_tableData->rowCount(); ++row) {
            for (int col = 0; col < m_tableData->columnCount(); ++col) {
                QVariant value = m_tableData->at(row, col);
                QStandardItem *item = new QStandardItem(value.toString());
                m_model->setItem(row, col, item);
            }
        }

        // 自动调整列宽
        autoResizeColumns();

        emit fileLoaded(filePath);
        return true;

    } else if (suffix == "xlsx" || suffix == "xls") {
        // 使用 ExcelLoader 加载
        ExcelLoader loader;
        LoadResult result = loader.load(filePath);

        if (!result.success) {
            QMessageBox::warning(this, "错误", "加载Excel文件失败: " + result.errorMessage);
            return false;
        }

        // 更新 TableData
        delete m_tableData;
        m_tableData = result.data;

        // 更新 QStandardItemModel
        m_model->clear();
        m_model->setRowCount(m_tableData->rowCount());
        m_model->setColumnCount(m_tableData->columnCount());

        // 设置表头（使用 TableData 中的表头）
        for (int col = 0; col < m_tableData->columnCount(); ++col) {
            m_model->setHeaderData(col, Qt::Horizontal, m_tableData->header(col));
        }

        // 填充数据
        for (int row = 0; row < m_tableData->rowCount(); ++row) {
            for (int col = 0; col < m_tableData->columnCount(); ++col) {
                QVariant value = m_tableData->at(row, col);
                QStandardItem *item = new QStandardItem(value.toString());
                m_model->setItem(row, col, item);
            }
        }

        // 自动调整列宽
        autoResizeColumns();

        emit fileLoaded(filePath);
        return true;
    }

    return false;
}

bool DataTableView::saveFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    if (suffix == "csv") {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream out(&file);

        // 写入表头
        QStringList headers;
        for (int col = 0; col < m_model->columnCount(); ++col) {
            headers << m_model->horizontalHeaderItem(col)->text();
        }
        out << headers.join(',') << "\n";

        // 写入数据
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QStringList values;
            for (int col = 0; col < m_model->columnCount(); ++col) {
                QStandardItem *item = m_model->item(row, col);
                values << (item ? item->text() : "");
            }
            out << values.join(',') << "\n";
        }

        file.close();
        return true;
    }

    return false;
}

void DataTableView::clearData()
{
    m_model->clear();
    delete m_tableData;
    m_tableData = new Core::TableData();
}

Core::TableData *DataTableView::tableData() const
{
    return m_tableData;
}

QString DataTableView::selectedRangeInfo() const
{
    QModelIndexList indexes = selectedIndexes();

    if (indexes.isEmpty()) {
        return "无选择";
    }

    int minRow = indexes.first().row();
    int maxRow = indexes.first().row();
    int minCol = indexes.first().column();
    int maxCol = indexes.first().column();

    for (const QModelIndex &index : indexes) {
        minRow = qMin(minRow, index.row());
        maxRow = qMax(maxRow, index.row());
        minCol = qMin(minCol, index.column());
        maxCol = qMax(maxCol, index.column());
    }

    int rows = maxRow - minRow + 1;
    int cols = maxCol - minCol + 1;

    return QString("%1 行 x %2 列").arg(rows).arg(cols);
}

// ==================== 快速统计 ====================

DataTableView::SelectionStats DataTableView::calculateSelectionStats() const
{
    SelectionStats stats;

    QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty()) {
        return stats;
    }

    QVector<double> numericValues;

    // 遍历选中的单元格，收集数值数据
    for (const QModelIndex &index : indexes) {
        if (!index.isValid()) continue;

        QStandardItem* item = m_model->item(index.row(), index.column());
        if (!item) continue;

        QString text = item->text().trimmed();
        if (text.isEmpty()) continue;

        // 尝试转换为数值
        bool ok = false;
        double value = text.toDouble(&ok);
        if (ok) {
            numericValues.append(value);
        }
    }

    if (numericValues.isEmpty()) {
        stats.count = indexes.size();
        return stats;
    }

    // 计算统计量
    stats.count = indexes.size();
    stats.hasNumericData = true;
    stats.sum = 0.0;
    stats.min = numericValues.first();
    stats.max = numericValues.first();

    for (double value : numericValues) {
        stats.sum += value;
        stats.min = qMin(stats.min, value);
        stats.max = qMax(stats.max, value);
    }

    stats.mean = stats.sum / numericValues.size();

    return stats;
}

QString DataTableView::getSelectionStatsText() const
{
    SelectionStats stats = calculateSelectionStats();

    if (stats.count == 0) {
        return "";
    }

    // 智能格式化数字（添加千分位分隔符）
    auto formatNumber = [](double value, int decimals = 2) -> QString {
        if (qAbs(value) >= 1000000) {
            return QString::number(value / 1000000.0, 'f', 1) + "M";
        } else if (qAbs(value) >= 1000) {
            return QString::number(value / 1000.0, 'f', 1) + "K";
        } else if (qFuzzyCompare(value, qFloor(value))) {
            return QString::number(qint64(value));
        } else {
            return QString::number(value, 'f', decimals);
        }
    };

    QString result = QString("选中: %1 格").arg(stats.count);

    if (stats.hasNumericData) {
        result += QString(" | Σ: %1 | 均值: %2 | 最小: %3 | 最大: %4")
            .arg(formatNumber(stats.sum))
            .arg(formatNumber(stats.mean))
            .arg(formatNumber(stats.min))
            .arg(formatNumber(stats.max));
    }

    return result;
}


void DataTableView::onContextMenuRequested(const QPoint &pos)
{
    m_contextMenu->exec(viewport()->mapToGlobal(pos));
}

void DataTableView::onHeaderClicked(int column)
{
    // 判断是否点击同一列
    if (m_sortColumn == column) {
        // 切换排序顺序
        m_sortOrder = (m_sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        // 新列，默认升序
        m_sortColumn = column;
        m_sortOrder = Qt::AscendingOrder;
    }

    // 执行排序
    sortColumn(column, m_sortOrder);

    // 更新排序指示器
    horizontalHeader()->setSortIndicator(column, m_sortOrder);
}

bool DataTableView::isNumericColumn(int column) const
{
    if (!m_tableData || column < 0 || column >= m_tableData->columnCount()) {
        return false;
    }

    return m_tableData->isNumeric(column);
}

void DataTableView::sortColumn(int column, Qt::SortOrder order)
{
    if (!m_model || column < 0 || column >= m_model->columnCount()) {
        return;
    }

    // 判断是否为数值列
    bool numeric = isNumericColumn(column);

    // 如果是数值列，需要设置正确的数据类型以便数值排序
    if (numeric) {
        for (int row = 0; row < m_model->rowCount(); ++row) {
            QStandardItem* item = m_model->item(row, column);
            if (item) {
                QString text = item->text();
                bool ok;
                double value = text.trimmed().toDouble(&ok);
                if (ok) {
                    // 设置为数值类型，这样 sort() 会按数值排序
                    // 同时保持 DisplayRole 为原始文本，保证显示格式
                    item->setData(value, Qt::EditRole);
                }
            }
        }
    }

    // 使用 QStandardItemModel 的内置排序功能
    m_model->sort(column, order);

    emit dataChanged();
}

// ==================== 列宽调整 ====================

void DataTableView::resizeColumnsToContents()
{
    // 使用 Qt 内置的列宽自适应功能
    QTableView::resizeColumnsToContents();
}

void DataTableView::autoResizeColumns()
{
    if (!m_model || m_model->columnCount() == 0) {
        return;
    }

    // 智能自适应列宽算法
    for (int col = 0; col < m_model->columnCount(); ++col) {
        // 获取表头宽度
        QString headerText = m_model->headerData(col, Qt::Horizontal).toString();
        int headerWidth = fontMetrics().horizontalAdvance(headerText) + 20;  // 加padding

        // 采样计算内容宽度（最多检查前100行，避免大文件卡顿）
        int maxContentWidth = headerWidth;
        int sampleRows = qMin(100, m_model->rowCount());

        for (int row = 0; row < sampleRows; ++row) {
            QStandardItem* item = m_model->item(row, col);
            if (item) {
                QString text = item->text();
                int textWidth = fontMetrics().horizontalAdvance(text);
                maxContentWidth = qMax(maxContentWidth, textWidth);
            }
        }

        // 设置列宽，限制在合理范围内
        int columnWidth = qBound(80, maxContentWidth + 20, 500);  // 最小80，最大500
        setColumnWidth(col, columnWidth);
    }

    // 如果总宽度小于视口宽度，拉伸最后一列填充
    if (horizontalHeader()->length() < viewport()->width()) {
        horizontalHeader()->setStretchLastSection(true);
    } else {
        horizontalHeader()->setStretchLastSection(false);
    }
}

// ==================== 复制粘贴 ====================

void DataTableView::copySelection()
{
    // 获取选中的单元格
    QModelIndexList indexes = selectedIndexes();

    if (indexes.isEmpty()) {
        return;
    }

    // 确定选中范围的边界
    int minRow = indexes.first().row();
    int maxRow = indexes.first().row();
    int minCol = indexes.first().column();
    int maxCol = indexes.first().column();

    for (const QModelIndex &index : indexes) {
        minRow = qMin(minRow, index.row());
        maxRow = qMax(maxRow, index.row());
        minCol = qMin(minCol, index.column());
        maxCol = qMax(maxCol, index.column());
    }

    // 按行组织数据（制表符分隔列，换行符分隔行）
    QString text;
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            QStandardItem* item = m_model->item(row, col);
            QString cellText = item ? item->text() : "";

            // 如果文本包含制表符、换行符或引号，需要用引号包裹并转义
            if (cellText.contains('\t') || cellText.contains('\n') || cellText.contains('"')) {
                cellText = "\"" + cellText.replace("\"", "\"\"") + "\"";
            }

            text += cellText;

            // 列之间用制表符分隔
            if (col < maxCol) {
                text += '\t';
            }
        }
        // 行之间用换行符分隔
        if (row < maxRow) {
            text += '\n';
        }
    }

    // 复制到剪贴板
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);

    // 给用户反馈（状态栏或短暂提示）
    qDebug() << "已复制" << (maxRow - minRow + 1) << "行 x" << (maxCol - minCol + 1) << "列到剪贴板";
}

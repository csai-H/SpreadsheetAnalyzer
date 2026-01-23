#include "DataTableView.h"
#include "../core/CsvLoader.h"
#include "../core/ExcelLoader.h"
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>

DataTableView::DataTableView(QWidget *parent)
    : QTableView(parent)
    , m_model(new QStandardItemModel(this))
    , m_tableData(new Core::TableData())
{
    setModel(m_model);

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

    // 设置排序指示器
    horizontalHeader()->setSortIndicatorShown(true);

    // 设置网格
    setShowGrid(true);
    setWordWrap(false);

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
        // TODO: 实现复制功能
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

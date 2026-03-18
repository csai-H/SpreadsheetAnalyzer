#include "DataTableView.h"

#include "TableFilterProxyModel.h"
#include "../core/CsvLoader.h"
#include "../core/ExcelExporter.h"
#include "../core/ExcelLoader.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextStream>

QWidget *TableItemDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto *editor = new QLineEdit(parent);
    editor->setFrame(true);
    editor->setAutoFillBackground(true);
    editor->setMinimumHeight(30);
    return editor;
}

void TableItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void TableItemDelegate::updateEditorGeometry(QWidget *editor,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

DataTableView::DataTableView(QWidget *parent)
    : QTableView(parent)
    , m_sourceModel(new QStandardItemModel(this))
    , m_proxyModel(new TableFilterProxyModel(this))
{
    m_proxyModel->setSourceModel(m_sourceModel);
    setModel(m_proxyModel);
    connectSourceModelSignals();

    setItemDelegate(new TableItemDelegate(this));
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setShowGrid(true);
    setWordWrap(false);

    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSectionsClickable(true);
    horizontalHeader()->setFixedHeight(50);
    horizontalHeader()->setSortIndicatorShown(true);
    verticalHeader()->setDefaultSectionSize(30);
    verticalHeader()->setMinimumWidth(80);

    connect(horizontalHeader(), &QHeaderView::sectionClicked,
            this, &DataTableView::onHeaderClicked);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() { emit selectionChanged(); });

    setupContextMenu();
}

DataTableView::~DataTableView() = default;

bool DataTableView::loadFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();
    LoadResult result;

    if (suffix == "csv" || suffix == "txt") {
        CsvLoader loader;
        result = loader.load(filePath);
    } else if (suffix == "xlsx" || suffix == "xls") {
        ExcelLoader loader;
        result = loader.load(filePath);
    } else {
        return false;
    }

    if (!result.success || !result.data) {
        QMessageBox::warning(this, tr("错误"), tr("加载文件失败: %1").arg(result.errorMessage));
        return false;
    }

    setTableData(QSharedPointer<Core::TableData>(result.data));
    autoResizeColumns();
    emit fileLoaded(filePath);
    return true;
}

bool DataTableView::saveFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();
    const auto data = snapshotData(false);
    if (!data || data->isEmpty()) {
        return false;
    }

    if (suffix == "xlsx") {
        return ExcelExporter::exportToExcel(data.data(), filePath);
    }

    if (suffix != "csv" && suffix != "txt") {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    auto escapeCsv = [](QString cellText) {
        if (cellText.contains(',') || cellText.contains('\n') || cellText.contains('"')) {
            cellText.replace("\"", "\"\"");
            return "\"" + cellText + "\"";
        }
        return cellText;
    };

    QStringList headers;
    for (int col = 0; col < data->columnCount(); ++col) {
        headers << escapeCsv(data->header(col));
    }
    out << headers.join(',') << '\n';

    for (int row = 0; row < data->rowCount(); ++row) {
        QStringList values;
        for (int col = 0; col < data->columnCount(); ++col) {
            values << escapeCsv(data->at(row, col).toString());
        }
        out << values.join(',') << '\n';
    }

    return true;
}

void DataTableView::clearData()
{
    m_bulkUpdating = true;
    m_sourceModel->clear();
    m_bulkUpdating = false;

    clearFilter();
    m_sortColumn = -1;
    m_sortOrder = Qt::AscendingOrder;
    invalidateSnapshots();
}

void DataTableView::setTableData(const QSharedPointer<Core::TableData> &data)
{
    populateModelFromTableData(data.data());
    clearFilter();
    m_sortColumn = -1;
    m_sortOrder = Qt::AscendingOrder;
    horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    autoResizeColumns();
}

bool DataTableView::hasData() const
{
    return totalRowCount() > 0 && dataColumnCount() > 0;
}

int DataTableView::totalRowCount() const
{
    return m_sourceModel->rowCount();
}

int DataTableView::visibleRowCount() const
{
    return m_proxyModel->rowCount();
}

int DataTableView::dataColumnCount() const
{
    return m_sourceModel->columnCount();
}

Core::TableData *DataTableView::tableData() const
{
    return snapshotData(false).data();
}

QSharedPointer<Core::TableData> DataTableView::snapshotData(bool visibleOnly) const
{
    if (m_snapshotCacheDirty) {
        m_fullSnapshotCache.clear();
        m_visibleSnapshotCache.clear();
        m_snapshotCacheDirty = false;
    }

    if (visibleOnly) {
        if (m_visibleSnapshotCache.isNull()) {
            m_visibleSnapshotCache = buildSnapshot(m_proxyModel);
        }
        return m_visibleSnapshotCache;
    }

    if (m_fullSnapshotCache.isNull()) {
        m_fullSnapshotCache = buildSnapshot(m_sourceModel);
    }
    return m_fullSnapshotCache;
}

void DataTableView::applyFilter(int column, int condition, const QString &value)
{
    m_proxyModel->setFilterRule(
        column,
        static_cast<TableFilterProxyModel::Condition>(condition),
        value);
    invalidateSnapshots();
    emit viewChanged();
}

void DataTableView::clearFilter()
{
    const bool hadFilter = m_proxyModel->hasActiveFilter();
    m_proxyModel->clearFilterRule();
    invalidateSnapshots();
    if (hadFilter) {
        emit viewChanged();
    }
}

bool DataTableView::hasActiveFilter() const
{
    return m_proxyModel->hasActiveFilter();
}

QString DataTableView::selectedRangeInfo() const
{
    const QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty()) {
        return tr("无选择");
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

    return tr("%1 行 x %2 列").arg(maxRow - minRow + 1).arg(maxCol - minCol + 1);
}

DataTableView::SelectionStats DataTableView::calculateSelectionStats() const
{
    SelectionStats stats;
    const QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty()) {
        return stats;
    }

    QVector<double> numericValues;
    numericValues.reserve(indexes.size());

    for (const QModelIndex &index : indexes) {
        if (!index.isValid()) {
            continue;
        }

        const QString text = model()->data(index, Qt::DisplayRole).toString().trimmed();
        if (text.isEmpty()) {
            continue;
        }

        bool ok = false;
        const double value = text.toDouble(&ok);
        if (ok) {
            numericValues.append(value);
        }
    }

    stats.count = indexes.size();
    if (numericValues.isEmpty()) {
        return stats;
    }

    stats.hasNumericData = true;
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
    const SelectionStats stats = calculateSelectionStats();
    if (stats.count == 0) {
        return {};
    }

    auto formatNumber = [](double value, int decimals = 2) {
        if (qAbs(value) >= 1000000.0) {
            return QString::number(value / 1000000.0, 'f', 1) + "M";
        }
        if (qAbs(value) >= 1000.0) {
            return QString::number(value / 1000.0, 'f', 1) + "K";
        }
        if (qFuzzyCompare(value, qFloor(value))) {
            return QString::number(static_cast<qint64>(value));
        }
        return QString::number(value, 'f', decimals);
    };

    QString result = tr("选中: %1 格").arg(stats.count);
    if (stats.hasNumericData) {
        result += tr(" | 和: %1 | 均值: %2 | 最小: %3 | 最大: %4")
                      .arg(formatNumber(stats.sum))
                      .arg(formatNumber(stats.mean))
                      .arg(formatNumber(stats.min))
                      .arg(formatNumber(stats.max));
    }

    return result;
}

void DataTableView::resizeColumnsToContents()
{
    QTableView::resizeColumnsToContents();
}

void DataTableView::autoResizeColumns()
{
    if (model()->columnCount() == 0) {
        return;
    }

    for (int col = 0; col < model()->columnCount(); ++col) {
        const QString headerText = model()->headerData(col, Qt::Horizontal).toString();
        int maxContentWidth = fontMetrics().horizontalAdvance(headerText) + 20;
        const int sampleRows = qMin(100, model()->rowCount());

        for (int row = 0; row < sampleRows; ++row) {
            const QString text = model()->data(model()->index(row, col), Qt::DisplayRole).toString();
            maxContentWidth = qMax(maxContentWidth, fontMetrics().horizontalAdvance(text));
        }

        setColumnWidth(col, qBound(80, maxContentWidth + 20, 500));
    }

    horizontalHeader()->setStretchLastSection(horizontalHeader()->length() < viewport()->width());
}

void DataTableView::copySelection()
{
    const QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty()) {
        return;
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

    QString text;
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            QString cellText = model()->data(model()->index(row, col), Qt::DisplayRole).toString();
            if (cellText.contains('\t') || cellText.contains('\n') || cellText.contains('"')) {
                cellText = "\"" + cellText.replace("\"", "\"\"") + "\"";
            }

            text += cellText;
            if (col < maxCol) {
                text += '\t';
            }
        }

        if (row < maxRow) {
            text += '\n';
        }
    }

    QApplication::clipboard()->setText(text);
}

void DataTableView::onContextMenuRequested(const QPoint &pos)
{
    m_contextMenu->exec(viewport()->mapToGlobal(pos));
}

void DataTableView::onHeaderClicked(int column)
{
    if (m_sortColumn == column) {
        m_sortOrder = (m_sortOrder == Qt::AscendingOrder)
                          ? Qt::DescendingOrder
                          : Qt::AscendingOrder;
    } else {
        m_sortColumn = column;
        m_sortOrder = Qt::AscendingOrder;
    }

    sortColumn(column, m_sortOrder);
    horizontalHeader()->setSortIndicator(column, m_sortOrder);
}

void DataTableView::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    m_contextMenu->addAction(tr("复制"), this, &DataTableView::copySelection);
    m_contextMenu->addSeparator();

    m_contextMenu->addAction(tr("插入行"), this, [this]() {
        int insertRow = m_sourceModel->rowCount();
        if (currentIndex().isValid()) {
            const QModelIndex sourceIndex = m_proxyModel->mapToSource(currentIndex());
            if (sourceIndex.isValid()) {
                insertRow = sourceIndex.row();
            }
        }

        m_sourceModel->insertRow(insertRow);
        for (int col = 0; col < m_sourceModel->columnCount(); ++col) {
            auto *item = new QStandardItem();
            item->setEditable(true);
            m_sourceModel->setItem(insertRow, col, item);
        }
    });

    m_contextMenu->addAction(tr("删除行"), this, [this]() {
        if (!currentIndex().isValid()) {
            return;
        }

        const QModelIndex sourceIndex = m_proxyModel->mapToSource(currentIndex());
        if (sourceIndex.isValid()) {
            m_sourceModel->removeRow(sourceIndex.row());
        }
    });

    m_contextMenu->addSeparator();

    QMenu *columnMenu = m_contextMenu->addMenu(tr("列宽调整"));
    columnMenu->addAction(tr("自动调整列宽"), this, &DataTableView::autoResizeColumns);
    columnMenu->addAction(tr("根据内容调整"), this, &DataTableView::resizeColumnsToContents);
    columnMenu->addAction(tr("重置为默认宽度"), this, [this]() {
        for (int col = 0; col < model()->columnCount(); ++col) {
            setColumnWidth(col, 100);
        }
    });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableView::customContextMenuRequested,
            this, &DataTableView::onContextMenuRequested);
}

void DataTableView::populateModelFromTableData(const Core::TableData *data)
{
    m_bulkUpdating = true;
    m_sourceModel->clear();

    if (!data || data->columnCount() == 0) {
        m_bulkUpdating = false;
        invalidateSnapshots();
        return;
    }

    m_sourceModel->setRowCount(data->rowCount());
    m_sourceModel->setColumnCount(data->columnCount());

    QStringList headers;
    headers.reserve(data->columnCount());
    for (int col = 0; col < data->columnCount(); ++col) {
        headers << data->header(col);
    }
    m_sourceModel->setHorizontalHeaderLabels(headers);

    for (int row = 0; row < data->rowCount(); ++row) {
        for (int col = 0; col < data->columnCount(); ++col) {
            auto *item = new QStandardItem(data->at(row, col).toString());
            item->setEditable(true);
            m_sourceModel->setItem(row, col, item);
        }
    }

    m_bulkUpdating = false;
    invalidateSnapshots();
}

QSharedPointer<Core::TableData> DataTableView::buildSnapshot(QAbstractItemModel *model) const
{
    if (!model) {
        return QSharedPointer<Core::TableData>::create();
    }

    auto data = QSharedPointer<Core::TableData>::create(model->rowCount(), model->columnCount());
    for (int col = 0; col < model->columnCount(); ++col) {
        data->setHeader(col, model->headerData(col, Qt::Horizontal).toString());
    }

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            data->set(row, col, model->data(model->index(row, col), Qt::DisplayRole));
        }
    }

    return data;
}

void DataTableView::invalidateSnapshots() const
{
    m_snapshotCacheDirty = true;
}

void DataTableView::connectSourceModelSignals()
{
    connect(m_sourceModel, &QStandardItemModel::dataChanged,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QStandardItemModel::rowsInserted,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QStandardItemModel::rowsRemoved,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QStandardItemModel::modelReset,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QStandardItemModel::headerDataChanged,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QStandardItemModel::layoutChanged,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit viewChanged();
                }
            });
}

void DataTableView::sortColumn(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= m_sourceModel->columnCount()) {
        return;
    }

    if (isNumericColumn(column)) {
        for (int row = 0; row < m_sourceModel->rowCount(); ++row) {
            QStandardItem *item = m_sourceModel->item(row, column);
            if (!item) {
                continue;
            }

            bool ok = false;
            const double value = item->text().trimmed().toDouble(&ok);
            if (ok) {
                item->setData(value, Qt::EditRole);
            }
        }
    }

    m_sourceModel->sort(column, order);
    invalidateSnapshots();
    emit dataChanged();
    emit viewChanged();
}

bool DataTableView::isNumericColumn(int column) const
{
    const auto data = snapshotData(false);
    return data && column >= 0 && column < data->columnCount() && data->isNumeric(column);
}

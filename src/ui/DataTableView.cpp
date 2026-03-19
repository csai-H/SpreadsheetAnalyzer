#include "DataTableView.h"

#include "TableDataModel.h"
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
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextStream>
#include <QUndoStack>

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
    , m_tableModel(QSharedPointer<TableDataModel>::create())
    , m_sourceModel(m_tableModel.data())
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
    setTableModel(QSharedPointer<TableDataModel>::create());
    clearFilter();
    m_sortColumn = -1;
    m_sortOrder = Qt::AscendingOrder;
    horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    invalidateSnapshots();
}

void DataTableView::setTableData(const QSharedPointer<Core::TableData> &data)
{
    setTableModel(QSharedPointer<TableDataModel>::create(data));
    clearFilter();
    m_sortColumn = -1;
    m_sortOrder = Qt::AscendingOrder;
    horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    autoResizeColumns();
}

void DataTableView::setTableModel(const QSharedPointer<TableDataModel> &model)
{
    if (m_sourceModel) {
        disconnect(m_sourceModel, nullptr, this, nullptr);
    }

    m_tableModel = model ? model : QSharedPointer<TableDataModel>::create();
    m_sourceModel = m_tableModel.data();
    if (m_sourceModel) {
        m_sourceModel->setUndoStack(m_undoStack);
    }
    m_proxyModel->setSourceModel(m_sourceModel);
    connectSourceModelSignals();
    invalidateSnapshots();
}

void DataTableView::setUndoStack(QUndoStack *undoStack)
{
    m_undoStack = undoStack;
    if (m_sourceModel) {
        m_sourceModel->setUndoStack(m_undoStack);
    }
}

bool DataTableView::hasData() const
{
    return totalRowCount() > 0 && dataColumnCount() > 0;
}

int DataTableView::totalRowCount() const
{
    return m_sourceModel ? m_sourceModel->rowCount() : 0;
}

int DataTableView::visibleRowCount() const
{
    return m_proxyModel->rowCount();
}

int DataTableView::dataColumnCount() const
{
    return m_sourceModel ? m_sourceModel->columnCount() : 0;
}

QSharedPointer<TableDataModel> DataTableView::tableModel() const
{
    return m_tableModel;
}

Core::TableData *DataTableView::tableData() const
{
    return m_sourceModel ? m_sourceModel->tableData().data() : nullptr;
}

QSharedPointer<Core::TableData> DataTableView::snapshotData(bool visibleOnly) const
{
    if (m_snapshotCacheDirty) {
        m_visibleSnapshotCache.clear();
        m_snapshotCacheDirty = false;
    }

    if (visibleOnly) {
        if (m_visibleSnapshotCache.isNull()) {
            m_visibleSnapshotCache = buildSnapshot(m_proxyModel);
        }
        return m_visibleSnapshotCache;
    }

    return m_sourceModel ? m_sourceModel->tableData() : QSharedPointer<Core::TableData>::create();
}

QStringList DataTableView::columnHeaders() const
{
    QStringList headers;
    if (!m_sourceModel) {
        return headers;
    }

    headers.reserve(m_sourceModel->columnCount());
    for (int col = 0; col < m_sourceModel->columnCount(); ++col) {
        headers << m_sourceModel->headerData(col, Qt::Horizontal).toString();
    }
    return headers;
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

int DataTableView::activeFilterColumn() const
{
    return m_proxyModel->filterColumn();
}

int DataTableView::activeFilterCondition() const
{
    return static_cast<int>(m_proxyModel->filterCondition());
}

QString DataTableView::activeFilterValue() const
{
    return m_proxyModel->filterValue();
}

int DataTableView::sortColumnIndex() const
{
    return m_sortColumn;
}

Qt::SortOrder DataTableView::currentSortOrder() const
{
    return m_sortOrder;
}

void DataTableView::setSortState(int column, Qt::SortOrder order)
{
    m_sortColumn = column;
    m_sortOrder = order;
    horizontalHeader()->setSortIndicator(column, order);
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
            return QString::number(value / 1000000.0, 'f', 1) + "百万";
        }
        if (qAbs(value) >= 1000.0) {
            return QString::number(value / 1000.0, 'f', 1) + "千";
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

bool DataTableView::pasteFromClipboard(QString *errorMessage)
{
    if (!m_sourceModel) {
        if (errorMessage) {
            *errorMessage = tr("当前表格没有可编辑的数据模型");
        }
        return false;
    }

    const QString clipboardText = QApplication::clipboard()->text();
    if (clipboardText.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("剪贴板中没有可粘贴的数据");
        }
        return false;
    }

    QString normalizedText = clipboardText;
    normalizedText.replace("\r\n", "\n");
    normalizedText.replace('\r', '\n');
    if (normalizedText.endsWith('\n')) {
        normalizedText.chop(1);
    }

    const QStringList rowTexts = normalizedText.split('\n');
    QVector<QStringList> rows;
    rows.reserve(rowTexts.size());

    int maxColumns = 0;
    for (const QString &rowText : rowTexts) {
        const QStringList columns = rowText.split('\t');
        if (columns.isEmpty()) {
            continue;
        }
        maxColumns = qMax(maxColumns, columns.size());
        rows.append(columns);
    }

    if (rows.isEmpty() || maxColumns <= 0) {
        if (errorMessage) {
            *errorMessage = tr("剪贴板内容格式不支持");
        }
        return false;
    }

    int startRow = 0;
    int startColumn = 0;

    const QModelIndex proxyIndex = currentIndex();
    if (proxyIndex.isValid()) {
        const QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        if (!sourceIndex.isValid()) {
            if (errorMessage) {
                *errorMessage = tr("无法确定粘贴位置");
            }
            return false;
        }

        startRow = sourceIndex.row();
        startColumn = sourceIndex.column();
    }

    const int requiredRows = startRow + rows.size();
    const int requiredColumns = startColumn + maxColumns;
    const bool useUndoMacro = m_undoStack != nullptr;
    if (useUndoMacro) {
        m_undoStack->beginMacro(tr("粘贴数据"));
    }

    if (requiredRows > m_sourceModel->rowCount()) {
        m_sourceModel->insertRows(m_sourceModel->rowCount(), requiredRows - m_sourceModel->rowCount());
    }
    if (requiredColumns > m_sourceModel->columnCount()) {
        m_sourceModel->insertColumns(m_sourceModel->columnCount(), requiredColumns - m_sourceModel->columnCount());
    }

    for (int rowOffset = 0; rowOffset < rows.size(); ++rowOffset) {
        const QStringList &columns = rows[rowOffset];
        for (int columnOffset = 0; columnOffset < columns.size(); ++columnOffset) {
            const QModelIndex target = m_sourceModel->index(startRow + rowOffset, startColumn + columnOffset);
            m_sourceModel->setData(target, columns[columnOffset], Qt::EditRole);
        }
    }

    const QModelIndex topLeft = m_proxyModel->mapFromSource(m_sourceModel->index(startRow, startColumn));
    const QModelIndex bottomRight = m_proxyModel->mapFromSource(
        m_sourceModel->index(startRow + rows.size() - 1, startColumn + maxColumns - 1));
    if (topLeft.isValid() && bottomRight.isValid()) {
        selectionModel()->select(QItemSelection(topLeft, bottomRight),
                                 QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(topLeft);
    }

    if (useUndoMacro) {
        m_undoStack->endMacro();
    }

    return true;
}

bool DataTableView::findText(const QString &text, Qt::CaseSensitivity caseSensitivity)
{
    if (text.isEmpty() || !model() || model()->rowCount() == 0 || model()->columnCount() == 0) {
        return false;
    }

    const int rowCount = model()->rowCount();
    const int columnCount = model()->columnCount();
    const int totalCells = rowCount * columnCount;

    int startOffset = 0;
    if (currentIndex().isValid()) {
        startOffset = currentIndex().row() * columnCount + currentIndex().column() + 1;
        if (startOffset >= totalCells) {
            startOffset = 0;
        }
    }

    auto selectMatch = [this](const QModelIndex &index) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(index);
        scrollTo(index, QAbstractItemView::PositionAtCenter);
    };

    for (int offset = 0; offset < totalCells; ++offset) {
        const int linearIndex = (startOffset + offset) % totalCells;
        const int row = linearIndex / columnCount;
        const int column = linearIndex % columnCount;
        const QModelIndex index = model()->index(row, column);
        const QString cellText = model()->data(index, Qt::DisplayRole).toString();
        if (cellText.contains(text, caseSensitivity)) {
            selectMatch(index);
            return true;
        }
    }

    return false;
}

bool DataTableView::goToCell(int row, int column, QString *errorMessage)
{
    if (!m_sourceModel) {
        if (errorMessage) {
            *errorMessage = tr("当前表格没有数据");
        }
        return false;
    }

    if (row < 0 || row >= m_sourceModel->rowCount() || column < 0 || column >= m_sourceModel->columnCount()) {
        if (errorMessage) {
            *errorMessage = tr("目标单元格超出当前数据范围");
        }
        return false;
    }

    const QModelIndex sourceIndex = m_sourceModel->index(row, column);
    const QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
    if (!proxyIndex.isValid()) {
        if (errorMessage) {
            *errorMessage = tr("目标单元格被当前筛选隐藏");
        }
        return false;
    }

    selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect);
    setCurrentIndex(proxyIndex);
    scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
    return true;
}

bool DataTableView::appendColumn(const QString &header, const QVector<QVariant> &values, QString *errorMessage)
{
    if (!m_sourceModel) {
        if (errorMessage) {
            *errorMessage = tr("当前表格没有可编辑的数据模型");
        }
        return false;
    }

    const QString trimmedHeader = header.trimmed();
    if (trimmedHeader.isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("新列名称不能为空");
        }
        return false;
    }

    if (m_sourceModel->rowCount() > 0 && values.size() != m_sourceModel->rowCount()) {
        if (errorMessage) {
            *errorMessage = tr("新列数据行数与当前表格不一致");
        }
        return false;
    }

    const int newColumn = m_sourceModel->columnCount();
    const bool useUndoMacro = m_undoStack != nullptr;
    if (useUndoMacro) {
        m_undoStack->beginMacro(tr("新增列 %1").arg(trimmedHeader));
    }
    if (!m_sourceModel->insertColumns(newColumn, 1)) {
        if (useUndoMacro) {
            m_undoStack->endMacro();
        }
        if (errorMessage) {
            *errorMessage = tr("无法插入新列");
        }
        return false;
    }

    m_sourceModel->setHeaderData(newColumn, Qt::Horizontal, trimmedHeader, Qt::EditRole);
    for (int row = 0; row < values.size(); ++row) {
        m_sourceModel->setData(m_sourceModel->index(row, newColumn), values[row], Qt::EditRole);
    }

    autoResizeColumns();
    if (useUndoMacro) {
        m_undoStack->endMacro();
    }
    return true;
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
    m_contextMenu->addAction(tr("粘贴"), this, [this]() {
        pasteFromClipboard();
    });
    m_contextMenu->addSeparator();

    m_contextMenu->addAction(tr("插入行"), this, [this]() {
        if (!m_sourceModel) {
            return;
        }

        int insertRow = m_sourceModel->rowCount();
        if (currentIndex().isValid()) {
            const QModelIndex sourceIndex = m_proxyModel->mapToSource(currentIndex());
            if (sourceIndex.isValid()) {
                insertRow = sourceIndex.row();
            }
        }

        m_sourceModel->insertRows(insertRow, 1);
    });

    m_contextMenu->addAction(tr("删除行"), this, [this]() {
        if (!m_sourceModel || !currentIndex().isValid()) {
            return;
        }

        const QModelIndex sourceIndex = m_proxyModel->mapToSource(currentIndex());
        if (sourceIndex.isValid()) {
            m_sourceModel->removeRows(sourceIndex.row(), 1);
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
    if (!m_sourceModel) {
        return;
    }

    connect(m_sourceModel, &QAbstractItemModel::dataChanged,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::rowsInserted,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::rowsRemoved,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::columnsInserted,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::columnsRemoved,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::modelReset,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::headerDataChanged,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit dataChanged();
                }
            });
    connect(m_sourceModel, &QAbstractItemModel::layoutChanged,
            this, [this]() {
                invalidateSnapshots();
                if (!m_bulkUpdating) {
                    emit viewChanged();
                }
            });
}

void DataTableView::sortColumn(int column, Qt::SortOrder order)
{
    if (!m_sourceModel || column < 0 || column >= m_sourceModel->columnCount()) {
        return;
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

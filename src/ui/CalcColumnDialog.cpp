#include "CalcColumnDialog.h"

#include "DataTableView.h"

#include <QAbstractItemView>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

QString fallbackColumnName(int index)
{
    return QStringLiteral("列%1").arg(index + 1);
}

bool nearlyZero(double value)
{
    return std::abs(value) <= std::numeric_limits<double>::epsilon();
}

} // namespace

CalcColumnDialog::CalcColumnDialog(QTableView* tableView, QWidget* parent)
    : QDialog(parent)
    , m_tableView(tableView)
    , m_sourceColumnsList(nullptr)
    , m_calculatedColumnsList(nullptr)
    , m_operationCombo(nullptr)
    , m_columnNameEdit(nullptr)
    , m_addButton(nullptr)
    , m_removeButton(nullptr)
    , m_applyButton(nullptr)
{
    setWindowTitle(QStringLiteral("计算列"));
    resize(560, 460);

    setupUI();
    updateAvailableColumns();
    onOperationChanged(0);
}

CalcColumnDialog::~CalcColumnDialog() = default;

void CalcColumnDialog::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    auto* sourceGroup = new QGroupBox(QStringLiteral("源列"));
    auto* sourceLayout = new QVBoxLayout();
    m_sourceColumnsList = new QListWidget();
    m_sourceColumnsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    sourceLayout->addWidget(m_sourceColumnsList);
    sourceGroup->setLayout(sourceLayout);
    layout->addWidget(sourceGroup);

    auto* calculatedGroup = new QGroupBox(QStringLiteral("待应用的计算列"));
    auto* calculatedLayout = new QVBoxLayout();
    m_calculatedColumnsList = new QListWidget();
    calculatedLayout->addWidget(m_calculatedColumnsList);
    calculatedGroup->setLayout(calculatedLayout);
    layout->addWidget(calculatedGroup);

    auto* calcGroup = new QGroupBox(QStringLiteral("计算参数"));
    auto* calcLayout = new QFormLayout();

    m_operationCombo = new QComboBox();
    m_operationCombo->addItem(QStringLiteral("加法"));
    m_operationCombo->addItem(QStringLiteral("减法"));
    m_operationCombo->addItem(QStringLiteral("乘法"));
    m_operationCombo->addItem(QStringLiteral("除法"));
    m_operationCombo->addItem(QStringLiteral("百分比"));
    m_operationCombo->addItem(QStringLiteral("差分"));
    m_operationCombo->addItem(QStringLiteral("增长率"));

    connect(m_operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CalcColumnDialog::onOperationChanged);

    m_columnNameEdit = new QLineEdit();

    calcLayout->addRow(QStringLiteral("计算类型:"), m_operationCombo);
    calcLayout->addRow(QStringLiteral("新列名称:"), m_columnNameEdit);

    calcGroup->setLayout(calcLayout);
    layout->addWidget(calcGroup);

    auto* buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton(QStringLiteral("添加到列表"));
    m_removeButton = new QPushButton(QStringLiteral("移除选中项"));
    m_applyButton = new QPushButton(QStringLiteral("应用"));

    connect(m_addButton, &QPushButton::clicked, this, &CalcColumnDialog::onAddColumn);
    connect(m_removeButton, &QPushButton::clicked, this, &CalcColumnDialog::onRemoveColumn);
    connect(m_applyButton, &QPushButton::clicked, this, &CalcColumnDialog::onApply);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyButton);

    layout->addLayout(buttonLayout);
}

void CalcColumnDialog::updateAvailableColumns()
{
    m_availableColumns.clear();
    if (m_sourceColumnsList) {
        m_sourceColumnsList->clear();
    }

    if (!m_tableView || !m_sourceColumnsList) {
        return;
    }

    if (auto* dataView = qobject_cast<DataTableView*>(m_tableView)) {
        const QStringList headers = dataView->columnHeaders();
        m_availableColumns.reserve(headers.size());
        for (int column = 0; column < headers.size(); ++column) {
            const QString header = headers.at(column).trimmed().isEmpty()
                                       ? fallbackColumnName(column)
                                       : headers.at(column);
            m_availableColumns.append(header);
            auto* item = new QListWidgetItem(header, m_sourceColumnsList);
            item->setData(Qt::UserRole, column);
        }
        return;
    }

    auto* model = m_tableView->model();
    if (!model) {
        return;
    }

    m_availableColumns.reserve(model->columnCount());
    for (int column = 0; column < model->columnCount(); ++column) {
        QString header = model->headerData(column, Qt::Horizontal).toString().trimmed();
        if (header.isEmpty()) {
            header = fallbackColumnName(column);
        }
        m_availableColumns.append(header);
        auto* item = new QListWidgetItem(header, m_sourceColumnsList);
        item->setData(Qt::UserRole, column);
    }
}

QString CalcColumnDialog::operationName(CalculationType type) const
{
    switch (type) {
    case Add:
        return QStringLiteral("加法");
    case Subtract:
        return QStringLiteral("减法");
    case Multiply:
        return QStringLiteral("乘法");
    case Divide:
        return QStringLiteral("除法");
    case Percentage:
        return QStringLiteral("百分比");
    case Difference:
        return QStringLiteral("差分");
    case GrowthRate:
        return QStringLiteral("增长率");
    }

    return QStringLiteral("未知");
}

QString CalcColumnDialog::buildCalculationDescription(const CalculationSpec& spec) const
{
    auto nameForColumn = [this](int index) {
        if (index >= 0 && index < m_availableColumns.size()) {
            return m_availableColumns.at(index);
        }
        return fallbackColumnName(index);
    };

    QString expression;
    switch (spec.type) {
    case Add:
        expression = QStringLiteral("%1 + %2")
                         .arg(nameForColumn(spec.sourceColumns.value(0)),
                              nameForColumn(spec.sourceColumns.value(1)));
        break;
    case Subtract:
        expression = QStringLiteral("%1 - %2")
                         .arg(nameForColumn(spec.sourceColumns.value(0)),
                              nameForColumn(spec.sourceColumns.value(1)));
        break;
    case Multiply:
        expression = QStringLiteral("%1 × %2")
                         .arg(nameForColumn(spec.sourceColumns.value(0)),
                              nameForColumn(spec.sourceColumns.value(1)));
        break;
    case Divide:
        expression = QStringLiteral("%1 ÷ %2")
                         .arg(nameForColumn(spec.sourceColumns.value(0)),
                              nameForColumn(spec.sourceColumns.value(1)));
        break;
    case Percentage:
        expression = QStringLiteral("%1 / %2 × 100")
                         .arg(nameForColumn(spec.sourceColumns.value(0)),
                              nameForColumn(spec.sourceColumns.value(1)));
        break;
    case Difference:
        expression = QStringLiteral("%1 的相邻差分")
                         .arg(nameForColumn(spec.sourceColumns.value(0)));
        break;
    case GrowthRate:
        expression = QStringLiteral("%1 的相邻增长率")
                         .arg(nameForColumn(spec.sourceColumns.value(0)));
        break;
    }

    return QStringLiteral("%1 = %2").arg(spec.columnName, expression);
}

int CalcColumnDialog::requiredColumnCount(CalculationType type) const
{
    switch (type) {
    case Difference:
    case GrowthRate:
        return 1;
    case Add:
    case Subtract:
    case Multiply:
    case Divide:
    case Percentage:
        return 2;
    }

    return 0;
}

bool CalcColumnDialog::parseNumber(const QVariant& value, double* number, bool* isEmpty) const
{
    const QString text = value.toString().trimmed();
    if (text.isEmpty()) {
        if (number) {
            *number = 0.0;
        }
        if (isEmpty) {
            *isEmpty = true;
        }
        return true;
    }

    bool ok = false;
    const double parsed = text.toDouble(&ok);
    if (!ok) {
        return false;
    }

    if (number) {
        *number = parsed;
    }
    if (isEmpty) {
        *isEmpty = false;
    }
    return true;
}

bool CalcColumnDialog::calculateValues(const CalculationSpec& spec,
                                       QVector<QVariant>* values,
                                       QString* errorMessage) const
{
    auto* dataView = qobject_cast<DataTableView*>(m_tableView);
    const auto sourceData = dataView ? dataView->snapshotData(false)
                                     : QSharedPointer<Core::TableData>();
    if (!sourceData) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("当前表格没有可用于计算的数据。");
        }
        return false;
    }

    values->clear();
    values->reserve(sourceData->rowCount());

    auto invalidValueError = [this, errorMessage, &spec](int row, int column) {
        if (!errorMessage) {
            return;
        }
        const QString header = (column >= 0 && column < m_availableColumns.size())
                                   ? m_availableColumns.at(column)
                                   : fallbackColumnName(column);
        *errorMessage = QStringLiteral("第 %1 行的“%2”不是有效数字，无法执行“%3”。")
                            .arg(row + 2)
                            .arg(header, operationName(spec.type));
    };

    bool hasComputedValue = false;

    for (int row = 0; row < sourceData->rowCount(); ++row) {
        QVariant result;

        if (spec.type == Difference || spec.type == GrowthRate) {
            if (row == 0) {
                values->append(result);
                continue;
            }

            const int column = spec.sourceColumns.value(0, -1);
            double currentValue = 0.0;
            double previousValue = 0.0;
            bool currentEmpty = false;
            bool previousEmpty = false;

            if (!parseNumber(sourceData->at(row, column), &currentValue, &currentEmpty)) {
                invalidValueError(row, column);
                return false;
            }
            if (!parseNumber(sourceData->at(row - 1, column), &previousValue, &previousEmpty)) {
                invalidValueError(row - 1, column);
                return false;
            }

            if (currentEmpty || previousEmpty) {
                values->append(result);
                continue;
            }

            if (spec.type == Difference) {
                result = currentValue - previousValue;
                hasComputedValue = true;
            } else if (!nearlyZero(previousValue)) {
                result = (currentValue - previousValue) / previousValue * 100.0;
                hasComputedValue = true;
            }

            values->append(result);
            continue;
        }

        const int leftColumn = spec.sourceColumns.value(0, -1);
        const int rightColumn = spec.sourceColumns.value(1, -1);
        double leftValue = 0.0;
        double rightValue = 0.0;
        bool leftEmpty = false;
        bool rightEmpty = false;

        if (!parseNumber(sourceData->at(row, leftColumn), &leftValue, &leftEmpty)) {
            invalidValueError(row, leftColumn);
            return false;
        }
        if (!parseNumber(sourceData->at(row, rightColumn), &rightValue, &rightEmpty)) {
            invalidValueError(row, rightColumn);
            return false;
        }

        if (leftEmpty || rightEmpty) {
            values->append(result);
            continue;
        }

        switch (spec.type) {
        case Add:
            result = leftValue + rightValue;
            hasComputedValue = true;
            break;
        case Subtract:
            result = leftValue - rightValue;
            hasComputedValue = true;
            break;
        case Multiply:
            result = leftValue * rightValue;
            hasComputedValue = true;
            break;
        case Divide:
            if (!nearlyZero(rightValue)) {
                result = leftValue / rightValue;
                hasComputedValue = true;
            }
            break;
        case Percentage:
            if (!nearlyZero(rightValue)) {
                result = leftValue / rightValue * 100.0;
                hasComputedValue = true;
            }
            break;
        case Difference:
        case GrowthRate:
            break;
        }

        values->append(result);
    }

    if (!hasComputedValue && sourceData->rowCount() > 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("所选列没有可用于计算的有效数值。");
        }
        return false;
    }

    return true;
}

void CalcColumnDialog::onOperationChanged(int index)
{
    const auto type = static_cast<CalculationType>(index);
    const int requiredColumns = requiredColumnCount(type);

    if (m_sourceColumnsList) {
        m_sourceColumnsList->setSelectionMode(
            requiredColumns == 1 ? QAbstractItemView::SingleSelection
                                 : QAbstractItemView::ExtendedSelection);
        m_sourceColumnsList->clearSelection();
    }

    if (m_columnNameEdit) {
        switch (type) {
        case Add:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：收入+成本"));
            break;
        case Subtract:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：毛利"));
            break;
        case Multiply:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：金额"));
            break;
        case Divide:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：单价"));
            break;
        case Percentage:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：完成率(%)"));
            break;
        case Difference:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：成绩差分"));
            break;
        case GrowthRate:
            m_columnNameEdit->setPlaceholderText(QStringLiteral("例如：成绩增长率(%)"));
            break;
        }
    }
}

void CalcColumnDialog::onAddColumn()
{
    const QString columnName = m_columnNameEdit->text().trimmed();
    if (columnName.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("请输入新列名称。"));
        return;
    }

    const auto type = static_cast<CalculationType>(m_operationCombo->currentIndex());
    const int expectedColumns = requiredColumnCount(type);
    const QList<QListWidgetItem*> selectedItems = m_sourceColumnsList->selectedItems();
    if (selectedItems.size() != expectedColumns) {
        QMessageBox::warning(
            this,
            QStringLiteral("错误"),
            expectedColumns == 1
                ? QStringLiteral("当前计算类型需要且只能选择 1 列源数据。")
                : QStringLiteral("当前计算类型需要且只能选择 2 列源数据。"));
        return;
    }

    auto matchesName = [&columnName](const QString& existingName) {
        return existingName.compare(columnName, Qt::CaseInsensitive) == 0;
    };

    for (const QString& existingColumn : m_availableColumns) {
        if (matchesName(existingColumn)) {
            QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("新列名称与现有列重复。"));
            return;
        }
    }
    for (const CalculationSpec& spec : m_pendingCalculations) {
        if (matchesName(spec.columnName)) {
            QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("该列名已在待应用列表中。"));
            return;
        }
    }

    CalculationSpec spec;
    spec.columnName = columnName;
    spec.type = type;

    QVector<int> sourceColumns;
    sourceColumns.reserve(selectedItems.size());
    for (QListWidgetItem* item : selectedItems) {
        sourceColumns.append(item->data(Qt::UserRole).toInt());
    }
    std::sort(sourceColumns.begin(), sourceColumns.end());
    spec.sourceColumns = sourceColumns;

    m_pendingCalculations.append(spec);
    m_calculatedColumnsList->addItem(buildCalculationDescription(spec));
    m_calculatedColumnsList->setCurrentRow(m_calculatedColumnsList->count() - 1);
    m_columnNameEdit->clear();
}

void CalcColumnDialog::onRemoveColumn()
{
    const int row = m_calculatedColumnsList ? m_calculatedColumnsList->currentRow() : -1;
    if (row < 0 || row >= m_pendingCalculations.size()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择一个待应用的计算列。"));
        return;
    }

    m_pendingCalculations.removeAt(row);
    delete m_calculatedColumnsList->takeItem(row);
}

void CalcColumnDialog::onApply()
{
    auto* dataView = qobject_cast<DataTableView*>(m_tableView);
    if (!dataView) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("当前表格视图不支持计算列。"));
        return;
    }

    if (m_pendingCalculations.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先添加至少一个计算列。"));
        return;
    }

    QVector<QVector<QVariant>> calculatedValues;
    calculatedValues.reserve(m_pendingCalculations.size());

    for (const CalculationSpec& spec : m_pendingCalculations) {
        QVector<QVariant> values;
        QString errorMessage;
        if (!calculateValues(spec, &values, &errorMessage)) {
            QMessageBox::warning(this, QStringLiteral("计算失败"), errorMessage);
            return;
        }
        calculatedValues.append(values);
    }

    for (int index = 0; index < m_pendingCalculations.size(); ++index) {
        QString errorMessage;
        if (!dataView->appendColumn(m_pendingCalculations.at(index).columnName,
                                    calculatedValues.at(index),
                                    &errorMessage)) {
            QMessageBox::warning(this, QStringLiteral("应用失败"), errorMessage);
            return;
        }
    }

    accept();
}

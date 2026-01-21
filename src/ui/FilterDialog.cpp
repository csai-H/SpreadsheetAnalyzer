#include "FilterDialog.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QHeaderView>

FilterDialog::FilterDialog(QTableView* tableView, QWidget* parent)
    : QDialog(parent)
    , m_tableView(tableView)
{
    setWindowTitle("数据筛选");
    resize(400, 200);

    m_model = qobject_cast<QStandardItemModel*>(m_tableView->model());

    setupUI();
}

FilterDialog::~FilterDialog() = default;

void FilterDialog::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // 筛选条件组
    auto* filterGroup = new QGroupBox("筛选条件");
    auto* formLayout = new QFormLayout();

    // 列选择
    m_columnCombo = new QComboBox();
    for (int col = 0; col < m_model->columnCount(); ++col) {
        QString header = m_model->headerData(col, Qt::Horizontal).toString();
        m_columnCombo->addItem(header, col);
    }

    // 条件选择
    m_conditionCombo = new QComboBox();
    m_conditionCombo->addItem("等于", static_cast<int>(FilterCondition::Equals));
    m_conditionCombo->addItem("不等于", static_cast<int>(FilterCondition::NotEquals));
    m_conditionCombo->addItem("大于", static_cast<int>(FilterCondition::GreaterThan));
    m_conditionCombo->addItem("小于", static_cast<int>(FilterCondition::LessThan));
    m_conditionCombo->addItem("大于等于", static_cast<int>(FilterCondition::GreaterOrEqual));
    m_conditionCombo->addItem("小于等于", static_cast<int>(FilterCondition::LessOrEqual));
    m_conditionCombo->addItem("包含", static_cast<int>(FilterCondition::Contains));
    m_conditionCombo->addItem("不包含", static_cast<int>(FilterCondition::NotContains));
    m_conditionCombo->addItem("开始于", static_cast<int>(FilterCondition::StartsWith));
    m_conditionCombo->addItem("结束于", static_cast<int>(FilterCondition::EndsWith));
    m_conditionCombo->addItem("为空", static_cast<int>(FilterCondition::IsEmpty));
    m_conditionCombo->addItem("不为空", static_cast<int>(FilterCondition::IsNotEmpty));

    connect(m_conditionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FilterDialog::onFilterTypeChanged);

    // 值输入
    m_valueEdit = new QLineEdit();

    formLayout->addRow("列:", m_columnCombo);
    formLayout->addRow("条件:", m_conditionCombo);
    formLayout->addRow("值:", m_valueEdit);

    filterGroup->setLayout(formLayout);
    layout->addWidget(filterGroup);

    // 按钮
    auto* buttonLayout = new QHBoxLayout();
    auto* applyButton = new QPushButton("应用筛选");
    auto* clearButton = new QPushButton("清除筛选");
    auto* closeButton = new QPushButton("关闭");

    connect(applyButton, &QPushButton::clicked, this, &FilterDialog::onApplyFilter);
    connect(clearButton, &QPushButton::clicked, this, &FilterDialog::onClearFilter);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);
}

void FilterDialog::onFilterTypeChanged(int index)
{
    Q_UNUSED(index);
    // 根据筛选类型启用/禁用值输入
    auto condition = static_cast<FilterDialog::FilterCondition::Type>(m_conditionCombo->currentData().toInt());

    if (condition == FilterCondition::IsEmpty ||
        condition == FilterCondition::IsNotEmpty) {
        m_valueEdit->setEnabled(false);
    } else {
        m_valueEdit->setEnabled(true);
    }
}

void FilterDialog::onApplyFilter()
{
    applyFilter();
    accept();
}

void FilterDialog::onClearFilter()
{
    // 清除筛选
    for (int row = 0; row < m_model->rowCount(); ++row) {
        m_tableView->setRowHidden(row, false);
    }

    accept();
}

void FilterDialog::applyFilter()
{
    int column = currentColumn();
    auto condition = static_cast<FilterDialog::FilterCondition::Type>(m_conditionCombo->currentData().toInt());
    QString value = filterValue();

    for (int row = 0; row < m_model->rowCount(); ++row) {
        QStandardItem* item = m_model->item(row, column);
        if (!item) continue;

        QString itemText = item->text();
        bool match = false;

        // 判断是否匹配
        switch (condition) {
            case FilterCondition::Equals:
                match = (itemText == value);
                break;
            case FilterCondition::NotEquals:
                match = (itemText != value);
                break;
            case FilterCondition::GreaterThan:
                match = (itemText.toDouble() > value.toDouble());
                break;
            case FilterCondition::LessThan:
                match = (itemText.toDouble() < value.toDouble());
                break;
            case FilterCondition::GreaterOrEqual:
                match = (itemText.toDouble() >= value.toDouble());
                break;
            case FilterCondition::LessOrEqual:
                match = (itemText.toDouble() <= value.toDouble());
                break;
            case FilterCondition::Contains:
                match = itemText.contains(value);
                break;
            case FilterCondition::NotContains:
                match = !itemText.contains(value);
                break;
            case FilterCondition::StartsWith:
                match = itemText.startsWith(value);
                break;
            case FilterCondition::EndsWith:
                match = itemText.endsWith(value);
                break;
            case FilterCondition::IsEmpty:
                match = itemText.isEmpty();
                break;
            case FilterCondition::IsNotEmpty:
                match = !itemText.isEmpty();
                break;
        }

        // 显示/隐藏行
        m_tableView->setRowHidden(row, !match);
    }
}

int FilterDialog::currentColumn() const
{
    return m_columnCombo->currentData().toInt();
}

FilterDialog::FilterCondition::Type FilterDialog::currentCondition() const
{
    return static_cast<FilterDialog::FilterCondition::Type>(m_conditionCombo->currentData().toInt());
}

QString FilterDialog::filterValue() const
{
    return m_valueEdit->text();
}

#include "FilterDialog.h"

#include "DataTableView.h"

#include <QAbstractItemModel>

FilterDialog::FilterDialog(QTableView *tableView, QWidget *parent)
    : QDialog(parent)
    , m_tableView(tableView)
    , m_dataTableView(qobject_cast<DataTableView *>(tableView))
{
    setWindowTitle(tr("数据筛选"));
    resize(400, 200);

    setupUI();
}

FilterDialog::~FilterDialog() = default;

void FilterDialog::setupUI()
{
    auto *layout = new QVBoxLayout(this);

    auto *filterGroup = new QGroupBox(tr("筛选条件"));
    auto *formLayout = new QFormLayout();

    m_columnCombo = new QComboBox();
    if (m_tableView && m_tableView->model()) {
        for (int col = 0; col < m_tableView->model()->columnCount(); ++col) {
            const QString header = m_tableView->model()->headerData(col, Qt::Horizontal).toString();
            m_columnCombo->addItem(header, col);
        }
    }

    m_conditionCombo = new QComboBox();
    m_conditionCombo->addItem(tr("等于"), static_cast<int>(FilterCondition::Equals));
    m_conditionCombo->addItem(tr("不等于"), static_cast<int>(FilterCondition::NotEquals));
    m_conditionCombo->addItem(tr("大于"), static_cast<int>(FilterCondition::GreaterThan));
    m_conditionCombo->addItem(tr("小于"), static_cast<int>(FilterCondition::LessThan));
    m_conditionCombo->addItem(tr("大于等于"), static_cast<int>(FilterCondition::GreaterOrEqual));
    m_conditionCombo->addItem(tr("小于等于"), static_cast<int>(FilterCondition::LessOrEqual));
    m_conditionCombo->addItem(tr("包含"), static_cast<int>(FilterCondition::Contains));
    m_conditionCombo->addItem(tr("不包含"), static_cast<int>(FilterCondition::NotContains));
    m_conditionCombo->addItem(tr("开始于"), static_cast<int>(FilterCondition::StartsWith));
    m_conditionCombo->addItem(tr("结束于"), static_cast<int>(FilterCondition::EndsWith));
    m_conditionCombo->addItem(tr("为空"), static_cast<int>(FilterCondition::IsEmpty));
    m_conditionCombo->addItem(tr("不为空"), static_cast<int>(FilterCondition::IsNotEmpty));

    connect(m_conditionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FilterDialog::onFilterTypeChanged);

    m_valueEdit = new QLineEdit();

    formLayout->addRow(tr("列"), m_columnCombo);
    formLayout->addRow(tr("条件"), m_conditionCombo);
    formLayout->addRow(tr("值"), m_valueEdit);

    filterGroup->setLayout(formLayout);
    layout->addWidget(filterGroup);

    auto *buttonLayout = new QHBoxLayout();
    auto *applyButton = new QPushButton(tr("应用筛选"));
    auto *clearButton = new QPushButton(tr("清除筛选"));
    auto *closeButton = new QPushButton(tr("关闭"));

    connect(applyButton, &QPushButton::clicked, this, &FilterDialog::onApplyFilter);
    connect(clearButton, &QPushButton::clicked, this, &FilterDialog::onClearFilter);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    onFilterTypeChanged(m_conditionCombo->currentIndex());
}

void FilterDialog::onFilterTypeChanged(int index)
{
    Q_UNUSED(index);

    const auto condition = currentCondition();
    const bool needsValue =
        condition != FilterCondition::IsEmpty &&
        condition != FilterCondition::IsNotEmpty;
    m_valueEdit->setEnabled(needsValue);
    if (!needsValue) {
        m_valueEdit->clear();
    }
}

void FilterDialog::onApplyFilter()
{
    if (m_dataTableView) {
        m_dataTableView->applyFilter(
            currentColumn(),
            static_cast<int>(currentCondition()),
            filterValue());
    }
    accept();
}

void FilterDialog::onClearFilter()
{
    if (m_dataTableView) {
        m_dataTableView->clearFilter();
    }
    accept();
}

int FilterDialog::currentColumn() const
{
    return m_columnCombo->currentData().toInt();
}

FilterDialog::FilterCondition::Type FilterDialog::currentCondition() const
{
    return static_cast<FilterCondition::Type>(m_conditionCombo->currentData().toInt());
}

QString FilterDialog::filterValue() const
{
    return m_valueEdit->text();
}

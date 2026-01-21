#include "CalcColumnDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStandardItemModel>

CalcColumnDialog::CalcColumnDialog(QTableView* tableView, QWidget* parent)
    : QDialog(parent)
    , m_tableView(tableView)
{
    setWindowTitle("计算列");
    resize(500, 400);

    updateAvailableColumns();
    setupUI();
}

CalcColumnDialog::~CalcColumnDialog() = default;

void CalcColumnDialog::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // 源列选择
    auto* sourceGroup = new QGroupBox("源列");
    auto* sourceLayout = new QVBoxLayout();
    m_sourceColumnsList = new QListWidget();
    sourceLayout->addWidget(m_sourceColumnsList);
    sourceGroup->setLayout(sourceLayout);
    layout->addWidget(sourceGroup);

    // 计算参数
    auto* calcGroup = new QGroupBox("计算参数");
    auto* calcLayout = new QFormLayout();

    m_operationCombo = new QComboBox();
    m_operationCombo->addItem("加法");
    m_operationCombo->addItem("减法");
    m_operationCombo->addItem("乘法");
    m_operationCombo->addItem("除法");
    m_operationCombo->addItem("百分比");
    m_operationCombo->addItem("差分");
    m_operationCombo->addItem("增长率");

    connect(m_operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CalcColumnDialog::onOperationChanged);

    m_columnNameEdit = new QLineEdit();
    m_columnNameEdit->setPlaceholderText("新列名称");

    calcLayout->addRow("计算类型:", m_operationCombo);
    calcLayout->addRow("新列名称:", m_columnNameEdit);

    calcGroup->setLayout(calcLayout);
    layout->addWidget(calcGroup);

    // 按钮
    auto* buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("添加列");
    m_removeButton = new QPushButton("移除列");
    m_applyButton = new QPushButton("应用到数据");

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
    m_sourceColumnsList->clear();

    if (!m_tableView) return;

    auto* model = qobject_cast<QStandardItemModel*>(m_tableView->model());
    if (!model) return;

    for (int col = 0; col < model->columnCount(); ++col) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        m_availableColumns.append(header);
        m_sourceColumnsList->addItem(header);
    }
}

void CalcColumnDialog::onOperationChanged(int index)
{
    Q_UNUSED(index);
}

void CalcColumnDialog::onAddColumn()
{
    QString columnName = m_columnNameEdit->text().trimmed();

    if (columnName.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入新列名称");
        return;
    }

    QString operation = m_operationCombo->currentText();
    QString itemText = QString("%1: %2").arg(columnName, operation);

    m_calculatedColumnsList->addItem(itemText);
}

void CalcColumnDialog::onRemoveColumn()
{
    auto* selectedItem = m_calculatedColumnsList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "提示", "请先选择一个计算列");
        return;
    }

    delete selectedItem;
}

void CalcColumnDialog::onApply()
{
    QMessageBox::information(this, "提示", "计算列功能将在下一个版本中完整实现");
    accept();
}

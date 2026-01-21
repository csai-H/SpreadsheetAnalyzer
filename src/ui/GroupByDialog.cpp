#include "GroupByDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QStandardItemModel>
#include <QTableWidgetItem>
#include <algorithm>
#include <numeric>
#include <map>

GroupByDialog::GroupByDialog(QTableView* tableView, QWidget *parent)
    : QDialog(parent)
    , m_tableView(tableView)
{
    setWindowTitle("数据分组汇总");
    resize(800, 600);

    setupUI();
    populateColumns();
}

GroupByDialog::~GroupByDialog()
{
}

void GroupByDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 分组参数组
    auto* paramGroup = new QGroupBox("分组参数");
    auto* paramLayout = new QFormLayout();

    m_groupByCombo = new QComboBox();
    m_aggregateCombo = new QComboBox();
    m_aggregateCombo->addItem("计数", static_cast<int>(0));
    m_aggregateCombo->addItem("求和", static_cast<int>(1));
    m_aggregateCombo->addItem("均值", static_cast<int>(2));
    m_aggregateCombo->addItem("最小值", static_cast<int>(3));
    m_aggregateCombo->addItem("最大值", static_cast<int>(4));
    m_aggregateCombo->addItem("标准差", static_cast<int>(5));

    paramLayout->addRow("分组列:", m_groupByCombo);
    paramLayout->addRow("汇总方式:", m_aggregateCombo);

    paramGroup->setLayout(paramLayout);
    mainLayout->addWidget(paramGroup);

    // 分组结果组
    auto* resultGroup = new QGroupBox("分组结果");
    auto* resultLayout = new QVBoxLayout();

    m_groupTreeWidget = new QTreeWidget();
    m_resultsTable = new QTableWidget();
    m_resultsTable->setEditTriggers(QTableView::NoEditTriggers);
    m_resultsTable->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_resultsTable->setAlternatingRowColors(true);

    resultLayout->addWidget(m_groupTreeWidget);
    resultLayout->addWidget(m_resultsTable);

    resultGroup->setLayout(resultLayout);
    mainLayout->addWidget(resultGroup);

    // 按钮
    auto* buttonLayout = new QHBoxLayout();
    m_groupByButton = new QPushButton("执行分组");
    m_clearButton = new QPushButton("清除分组");
    m_exportButton = new QPushButton("导出结果");

    connect(m_groupByButton, &QPushButton::clicked, this, &GroupByDialog::onGroupByClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &GroupByDialog::onClearGrouping);
    connect(m_exportButton, &QPushButton::clicked, this, &GroupByDialog::onExportResults);

    buttonLayout->addWidget(m_groupByButton);
    m_clearButton->setEnabled(false);
    m_exportButton->setEnabled(false);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_exportButton);

    mainLayout->addLayout(buttonLayout);
}

void GroupByDialog::populateColumns()
{
    if (!m_tableView) return;

    auto* model = qobject_cast<QStandardItemModel*>(m_tableView->model());
    if (!model) return;

    m_groupByCombo->clear();

    for (int col = 0; col < model->columnCount(); ++col) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        m_groupByCombo->addItem(header, col);
    }
}

void GroupByDialog::onGroupByClicked()
{
    m_groupTreeWidget->clear();
    m_resultsTable->setRowCount(0);
    m_resultsTable->setColumnCount(0);

    if (!m_tableView) return;

    auto* model = qobject_cast<QStandardItemModel*>(m_tableView->model());
    if (!model) return;

    int groupColumn = m_groupByCombo->currentData().toInt();

    // 分组数据收集
    QMap<QString, QList<QList<double>>> groupedData;

    // 行索引映射
    QMap<QString, QList<int>> groupRowIndices;

    for (int row = 0; row < model->rowCount(); ++row) {
        QVariant value = model->data(model->index(row, groupColumn));
        QString groupKey = value.toString();

        if (!groupRowIndices.contains(groupKey)) {
            groupRowIndices[groupKey] = QList<int>();
        }
        groupRowIndices[groupKey].append(row);
    }

    // 获取每个组的所有数值列
    int numColumns = model->columnCount();

    for (const QString& groupKey : groupRowIndices.keys()) {
        QList<QList<double>> columnData;

        for (int col = 0; col < numColumns; ++col) {
            QList<double> values;

            for (int row : groupRowIndices[groupKey]) {
                QVariant value = model->data(model->index(row, col));
                bool ok;
                double numValue = value.toDouble(&ok);
                if (ok) {
                    values.append(numValue);
                } else {
                    values.append(0.0);
                }
            }

            columnData.append(values);
        }

        groupedData[groupKey] = columnData;
    }

    // 计算统计
    auto statistics = calculateStatistics(groupedData);

    // 显示分组树
    for (const QString& groupKey : groupedData.keys()) {
        auto* groupItem = new QTreeWidgetItem();
        groupItem->setText(0, groupKey);
        groupItem->setExpanded(true);

        for (const QString& statName : statistics[groupKey].keys()) {
            auto* statItem = new QTreeWidgetItem();
            statItem->setText(0, statName);
            statItem->setText(1, QString::number(statistics[groupKey][statName]));
            groupItem->addChild(statItem);
        }

        m_groupTreeWidget->addTopLevelItem(groupItem);
    }

    // 显示统计表格
    QStringList headers;
    headers << "分组";

    QStringList rowHeaders;
    for (const QString& groupKey : groupedData.keys()) {
        rowHeaders << groupKey;
    }

    // 获取第一个组的键用于遍历统计信息
    QString firstKey;
    if (!groupedData.isEmpty()) {
        firstKey = groupedData.keys().first();
    }

    for (const QString& statName : statistics[firstKey].keys()) {
        if (rowHeaders.contains(statName)) {
            // 已经添加过，跳过
            continue;
        }
        rowHeaders << statName;
        break;
    }

    m_resultsTable->setColumnCount(rowHeaders.size());
    m_resultsTable->setRowCount(groupedData.size());

    // 设置表头
    m_resultsTable->setHorizontalHeaderLabels(rowHeaders);

    // 填充数据
    int row = 0;
    for (const QString& groupKey : groupedData.keys()) {
        int col = 0;

        m_resultsTable->setItem(row, col++, new QTableWidgetItem(groupKey));

        for (const QString& statName : statistics[groupKey].keys()) {
            double value = statistics[groupKey][statName];
            m_resultsTable->setItem(row, col++, new QTableWidgetItem(QString::number(value, 'f', 2)));
        }

        row++;
    }

    m_clearButton->setEnabled(true);
    m_exportButton->setEnabled(true);
}

QMap<QString, QMap<QString, double>> GroupByDialog::calculateStatistics(const QMap<QString, QList<QList<double>>>& groupedData)
{
    QMap<QString, QMap<QString, double>> results;

    for (const QString& groupKey : groupedData.keys()) {
        QMap<QString, double> stats;
        const QList<QList<double>>& columnData = groupedData[groupKey];

        // 对每个数值列进行统计
        for (int col = 0; col < columnData.size(); ++col) {
            const QList<double>& values = columnData[col];
            QString statName = QString("列%1").arg(col + 1);

            // 计算各种统计量
            if (m_aggregateCombo->currentData().toInt() == 0) {
                // 计数
                stats[statName + "(计数)"] = values.size();
            } else if (m_aggregateCombo->currentData().toInt() == 1) {
                // 求和
                stats[statName + "(求和)"] = std::accumulate(values.begin(), values.end(), 0.0);
            } else if (m_aggregateCombo->currentData().toInt() == 2) {
                // 均值
                if (values.isEmpty()) {
                    stats[statName + "(均值)"] = 0.0;
                } else {
                    double sum = std::accumulate(values.begin(), values.end(), 0.0);
                    stats[statName + "(均值)"] = sum / values.size();
                }
            } else if (m_aggregateCombo->currentData().toInt() == 3) {
                // 最小值
                auto it = std::min_element(values.begin(), values.end());
                stats[statName + "(最小值)"] = *it;
            } else if (m_aggregateCombo->currentData().toInt() == 4) {
                // 最大值
                auto it = std::max_element(values.begin(), values.end());
                stats[statName + "(最大值)"] = *it;
            } else if (m_aggregateCombo->currentData().toInt() == 5) {
                // 标准差
                if (values.size() > 1) {
                    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
                    double sqSum = 0.0;
                    for (double v : values) {
                        sqSum += (v - mean) * (v - mean);
                    }
                    stats[statName + "(标准差)"] = qSqrt(sqSum / (values.size() - 1));
                }
            }
        }

        results[groupKey] = stats;
    }

    return results;
}

void GroupByDialog::onClearGrouping()
{
    m_groupTreeWidget->clear();
    m_resultsTable->setRowCount(0);
    m_clearButton->setEnabled(false);
    m_exportButton->setEnabled(false);
}

void GroupByDialog::onExportResults()
{
    // TODO: 实现结果导出功能
    QMessageBox::information(this, "导出", "结果将导出为CSV文件");
}

#include "StatisticsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent)
    , m_tableData(nullptr)
{
    setWindowTitle("统计分析");
    resize(800, 600);

    setupUI();
}

StatisticsDialog::~StatisticsDialog() = default;

void StatisticsDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // 标签页
    m_tabWidget = new QTabWidget(this);

    // === 描述性统计标签页 ===
    m_descriptiveTab = new QWidget();
    auto *descriptiveLayout = new QVBoxLayout(m_descriptiveTab);

    // 列选择
    auto *columnLayout = new QHBoxLayout();
    columnLayout->addWidget(new QLabel("选择列:"));
    m_columnCombo = new QComboBox();
    columnLayout->addWidget(m_columnCombo);
    descriptiveLayout->addLayout(columnLayout);

    // 计算按钮
    m_calculateButton = new QPushButton("计算统计量");
    descriptiveLayout->addWidget(m_calculateButton);

    // 结果显示
    m_resultsTextEdit = new QTextEdit();
    m_resultsTextEdit->setReadOnly(true);
    m_resultsTextEdit->setFont(QFont("Courier", 10));
    descriptiveLayout->addWidget(m_resultsTextEdit);

    m_tabWidget->addTab(m_descriptiveTab, "描述性统计");

    // === 预测分析标签页 ===
    m_forecastingTab = new QWidget();
    auto *forecastingLayout = new QVBoxLayout(m_forecastingTab);

    // 参数设置组
    auto *paramGroup = new QGroupBox("预测参数");
    auto *paramLayout = new QFormLayout();

    m_forecastColumnCombo = new QComboBox();
    paramLayout->addRow("选择列:", m_forecastColumnCombo);

    m_forecastMethodCombo = new QComboBox();
    m_forecastMethodCombo->addItem("简单移动平均");
    m_forecastMethodCombo->addItem("加权移动平均");
    m_forecastMethodCombo->addItem("指数平滑");
    m_forecastMethodCombo->addItem("线性回归");
    paramLayout->addRow("预测方法:", m_forecastMethodCombo);

    m_windowSpinBox = new QSpinBox();
    m_windowSpinBox->setRange(2, 100);
    m_windowSpinBox->setValue(5);
    paramLayout->addRow("窗口大小:", m_windowSpinBox);

    m_alphaSpinBox = new QDoubleSpinBox();
    m_alphaSpinBox->setRange(0.01, 0.99);
    m_alphaSpinBox->setSingleStep(0.1);
    m_alphaSpinBox->setValue(0.3);
    paramLayout->addRow("平滑系数(α):", m_alphaSpinBox);

    m_forecastPeriodsSpinBox = new QSpinBox();
    m_forecastPeriodsSpinBox->setRange(1, 100);
    m_forecastPeriodsSpinBox->setValue(5);
    paramLayout->addRow("预测期数:", m_forecastPeriodsSpinBox);

    paramGroup->setLayout(paramLayout);
    forecastingLayout->addWidget(paramGroup);

    // 预测按钮
    m_forecastButton = new QPushButton("执行预测");
    forecastingLayout->addWidget(m_forecastButton);

    // 预测结果
    m_forecastResultsTextEdit = new QTextEdit();
    m_forecastResultsTextEdit->setReadOnly(true);
    m_forecastResultsTextEdit->setFont(QFont("Courier", 10));
    forecastingLayout->addWidget(m_forecastResultsTextEdit);

    m_tabWidget->addTab(m_forecastingTab, "预测分析");

    mainLayout->addWidget(m_tabWidget);

    // 连接信号
    connect(m_calculateButton, &QPushButton::clicked,
            this, &StatisticsDialog::onCalculateClicked);
    connect(m_forecastButton, &QPushButton::clicked,
            this, &StatisticsDialog::onCalculateClicked);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &StatisticsDialog::onTabChanged);
}

void StatisticsDialog::setTableData(Core::TableData *data)
{
    m_tableData = data;

    if (!m_tableData) return;

    // 更新列选择下拉框
    m_columnCombo->clear();
    m_forecastColumnCombo->clear();

    for (int col = 0; col < m_tableData->columnCount(); ++col) {
        QString header = m_tableData->header(col);
        m_columnCombo->addItem(header, col);
        m_forecastColumnCombo->addItem(header, col);
    }
}

void StatisticsDialog::onColumnChanged(int index)
{
    Q_UNUSED(index);
    // 可以在这里添加列变化时的处理逻辑
}

void StatisticsDialog::onTabChanged(int index)
{
    Q_UNUSED(index);
    // 切换标签页时的处理
}

void StatisticsDialog::onCalculateClicked()
{
    if (!m_tableData) {
        QMessageBox::warning(this, "错误", "没有数据");
        return;
    }

    int currentTab = m_tabWidget->currentIndex();

    if (currentTab == 0) {
        calculateDescriptiveStats();
    } else if (currentTab == 1) {
        calculateForecasting();
    }
}

void StatisticsDialog::calculateDescriptiveStats()
{
    int column = m_columnCombo->currentData().toInt();
    QVector<double> data = m_tableData->toDoubleVector(column);

    if (data.isEmpty()) {
        QMessageBox::warning(this, "错误", "所选列没有有效数据");
        return;
    }

    // 计算描述性统计
    Statistics::DescriptiveSummary summary = Statistics::DescriptiveStats::summarize(data);

    // 显示结果
    displaySummary(summary);
}

void StatisticsDialog::displaySummary(const Statistics::DescriptiveSummary &summary)
{
    QString result;

    result += "========== 描述性统计结果 ==========\n\n";

    result += "【集中趋势】\n";
    result += QString("  样本量: %1\n").arg(summary.count);
    result += QString("  均值: %1\n").arg(summary.mean, 0, 'f', 4);
    result += QString("  中位数: %1\n").arg(summary.median, 0, 'f', 4);
    result += QString("  众数: %1\n\n").arg(summary.mode, 0, 'f', 4);

    result += "【离散程度】\n";
    result += QString("  最小值: %1\n").arg(summary.min, 0, 'f', 4);
    result += QString("  最大值: %1\n").arg(summary.max, 0, 'f', 4);
    result += QString("  极差: %1\n").arg(summary.range, 0, 'f', 4);
    result += QString("  方差: %1\n").arg(summary.variance, 0, 'f', 4);
    result += QString("  标准差: %1\n").arg(summary.stdDev, 0, 'f', 4);
    result += QString("  第一四分位数(Q1): %1\n").arg(summary.q1, 0, 'f', 4);
    result += QString("  第三四分位数(Q3): %1\n").arg(summary.q3, 0, 'f', 4);
    result += QString("  四分位距(IQR): %1\n\n").arg(summary.iqr, 0, 'f', 4);

    result += "【分布形状】\n";
    result += QString("  偏度: %1\n").arg(summary.skewness, 0, 'f', 4);
    result += QString("  峰度: %1\n\n").arg(summary.kurtosis, 0, 'f', 4);

    result += "========== 说明 ==========\n";
    result += "偏度 > 0: 右偏（正偏）\n";
    result += "偏度 < 0: 左偏（负偏）\n";
    result += "偏度 ≈ 0: 对称分布\n\n";
    result += "峰度 > 3: 尖峰分布\n";
    result += "峰度 < 3: 平峰分布\n";
    result += "峰度 = 3: 正态分布";

    m_resultsTextEdit->setText(result);
}

void StatisticsDialog::calculateForecasting()
{
    int column = m_forecastColumnCombo->currentData().toInt();
    QVector<double> data = m_tableData->toDoubleVector(column);

    if (data.isEmpty()) {
        QMessageBox::warning(this, "错误", "所选列没有有效数据");
        return;
    }

    QString method = m_forecastMethodCombo->currentText();
    Statistics::ForecastResult result;

    if (method == "简单移动平均") {
        int window = m_windowSpinBox->value();
        int periods = m_forecastPeriodsSpinBox->value();
        result = Statistics::Forecasting::simpleMovingAverage(data, window, periods);

    } else if (method == "指数平滑") {
        double alpha = m_alphaSpinBox->value();
        int periods = m_forecastPeriodsSpinBox->value();
        result = Statistics::Forecasting::exponentialSmoothing(data, alpha, periods);

    } else if (method == "线性回归") {
        int periods = m_forecastPeriodsSpinBox->value();
        result = Statistics::Forecasting::linearRegression(data, periods);

    } else {
        QMessageBox::information(this, "提示", "该方法待实现");
        return;
    }

    if (!result.isValid) {
        QMessageBox::warning(this, "错误", result.errorMessage);
        return;
    }

    // 显示预测结果
    QString output;
    output += "========== 预测结果 ==========\n\n";
    output += QString("方法: %1\n").arg(method);
    output += QString("预测期数: %1\n\n").arg(result.predicted.size());

    output += "预测值:\n";
    for (int i = 0; i < result.predicted.size(); ++i) {
        output += QString("  期 %1: %2\n").arg(i + 1).arg(result.predicted[i], 0, 'f', 4);
    }

    output += QString("\n误差指标 (MSE): %1\n").arg(result.errorMetric, 0, 'f', 6);

    m_forecastResultsTextEdit->setText(output);
}

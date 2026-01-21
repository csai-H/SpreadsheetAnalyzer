#include "ChartView.h"
#include "visualization/ChartHelper.h"
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ChartView::ChartView(QWidget *parent)
    : QWidget(parent)
    , m_chartView(new QChartView(this))
    , m_currentChart(nullptr)
    , m_tableData(nullptr)
{
    auto *mainLayout = new QVBoxLayout(this);

    setupToolbar();

    // 设置图表视图
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView);

    // 创建示例图表
    createSampleChart();
}

ChartView::~ChartView() = default;

void ChartView::setupToolbar()
{
    auto *toolbarLayout = new QHBoxLayout();

    // 图表类型选择
    auto *typeLabel = new QLabel("图表类型:");
    toolbarLayout->addWidget(typeLabel);

    m_chartTypeCombo = new QComboBox();
    m_chartTypeCombo->addItem("柱状图");
    m_chartTypeCombo->addItem("折线图");
    m_chartTypeCombo->addItem("饼图");
    m_chartTypeCombo->addItem("散点图");
    m_chartTypeCombo->addItem("箱型图");
    m_chartTypeCombo->addItem("水平柱状图");
    toolbarLayout->addWidget(m_chartTypeCombo);

    // 标题编辑
    auto *titleLabel = new QLabel("标题:");
    toolbarLayout->addWidget(titleLabel);

    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("输入图表标题...");
    toolbarLayout->addWidget(m_titleEdit);

    // 刷新按钮
    m_refreshButton = new QPushButton("刷新");
    toolbarLayout->addWidget(m_refreshButton);

    // 保存按钮
    m_saveButton = new QPushButton("保存图片");
    toolbarLayout->addWidget(m_saveButton);

    toolbarLayout->addStretch();

    layout()->addItem(toolbarLayout);

    // 连接信号
    connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChartView::onChartTypeChanged);
    connect(m_titleEdit, &QLineEdit::textChanged,
            this, &ChartView::onTitleChanged);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &ChartView::refreshChart);
    connect(m_saveButton, &QPushButton::clicked,
            this, &ChartView::onSaveImage);
}

void ChartView::createSampleChart()
{
    // 创建示例柱状图
    auto *chart = new QChart();
    chart->setTitle("示例图表");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto *series = new QBarSeries();
    auto *set1 = new QBarSet("数据1");
    auto *set2 = new QBarSet("数据2");

    *set1 << 10 << 20 << 30 << 40 << 50;
    *set2 << 15 << 25 << 35 << 45 << 55;

    series->append(set1);
    series->append(set2);

    chart->addSeries(series);

    auto *axisX = new QBarCategoryAxis();
    axisX->append({"A", "B", "C", "D", "E"});
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto *axisY = new QValueAxis();
    axisY->setRange(0, 60);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView->setChart(chart);
    m_currentChart = chart;
}

void ChartView::setChartType(const QString &typeName)
{
    int index = m_chartTypeCombo->findText(typeName);
    if (index >= 0) {
        m_chartTypeCombo->setCurrentIndex(index);
    }
}

void ChartView::refreshChart()
{
    updateChart();
}

void ChartView::updateChart()
{
    QString chartType = m_chartTypeCombo->currentText();

    Charts::ChartStyle style;
    style.titleFontSize = 14;
    style.animationEnabled = true;
    style.showLegend = true;

    QChart* chart = nullptr;

    if (!m_chartData.isEmpty()) {
        // 使用真实数据
        m_chartData.title = m_titleEdit->text().isEmpty() ? m_chartData.title : m_titleEdit->text();

        if (chartType == "柱状图") {
            chart = Charts::ChartHelper::createBarChart(m_chartData, style);
        } else if (chartType == "折线图") {
            chart = Charts::ChartHelper::createLineChart(m_chartData, style);
        } else if (chartType == "散点图") {
            chart = Charts::ChartHelper::createScatterChart(m_chartData, style);
        } else if (chartType == "饼图") {
            chart = Charts::ChartHelper::createPieChart(m_chartData, style);
        } else if (chartType == "箱型图") {
            chart = Charts::ChartHelper::createBoxPlotChart(m_chartData, style);
        } else if (chartType == "水平柱状图") {
            chart = Charts::ChartHelper::createHorizontalBarChart(m_chartData, style);
        } else {
            // 未实现的图表类型
            createSampleChart();
            return;
        }
    } else {
        // 没有数据时显示示例图表
        createSampleChart();
        return;
    }

    if (chart) {
        m_chartView->setChart(chart);
        m_currentChart = chart;
    }
}

void ChartView::onChartTypeChanged(int index)
{
    Q_UNUSED(index);
    updateChart();
}

void ChartView::onTitleChanged(const QString &title)
{
    if (m_currentChart) {
        m_currentChart->setTitle(title);
    }
}

void ChartView::onSaveImage()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存图表",
        QString(),
        "PNG图片 (*.png);;JPEG图片 (*.jpg);;PDF文件 (*.pdf)"
    );

    if (!fileName.isEmpty()) {
        QPixmap pixmap = m_chartView->grab();
        if (pixmap.save(fileName)) {
            QMessageBox::information(this, "成功", "图表已保存");
        } else {
            QMessageBox::warning(this, "错误", "保存图表失败");
        }
    }
}

void ChartView::setChartData(const Charts::ChartData &data)
{
    m_chartData = data;
    updateChart();
}

void ChartView::setTableData(Core::TableData *data, int column)
{
    m_tableData = data;

    if (!m_tableData || column < 0 || column >= m_tableData->columnCount()) {
        return;
    }

    // 从TableData创建ChartData
    m_chartData.title = m_tableData->header(column);
    m_chartData.xAxisTitle = m_tableData->header(0);  // 使用第一列的表头（如"月份"）
    m_chartData.yAxisTitle = m_tableData->header(column);

    // 使用第一列作为X轴类别（通常是名称/日期列）
    m_chartData.categories.clear();
    if (m_tableData->columnCount() > 0) {
        for (int row = 0; row < m_tableData->rowCount(); ++row) {
            // 直接读取原始字符串，不做任何转换
            QVariant categoryValue = m_tableData->at(row, 0);
            QString categoryText = categoryValue.typeId() == QMetaType::QString
                ? categoryValue.toString()
                : QString::number(categoryValue.toDouble());
            m_chartData.categories.append(categoryText);
        }
    }

    QVector<double> values = m_tableData->toDoubleVector(column);
    Charts::DataSeries series(m_tableData->header(column), values);
    series.color = Charts::ChartHelper::getDefaultColor(0);

    m_chartData.series.clear();
    m_chartData.series.append(series);

    updateChart();
}

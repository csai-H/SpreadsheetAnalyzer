#include "ChartHelper.h"
#include "../core/TableData.h"
#include "../statistics/DescriptiveStats.h"
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QPercentBarSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QBoxSet>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QDebug>
#include <QFont>
#include <algorithm>
#include <map>

namespace Charts {

QVector<QColor> g_defaultColors = {
    QColor(52, 152, 219),    // 蓝色
    QColor(46, 204, 113),    // 绿色
    QColor(155, 89, 182),    // 紫色
    QColor(241, 196, 15),    // 黄色
    QColor(230, 126, 34),    // 橙色
    QColor(231, 76, 60),     // 红色
    QColor(26, 188, 156),    // 青色
    QColor(52, 73, 94)       // 深灰
};

QColor ChartHelper::getDefaultColor(int index)
{
    return g_defaultColors[index % g_defaultColors.size()];
}

QChart* ChartHelper::createBarChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setAnimationDuration(style.animationDuration);

    auto* series = new QBarSeries();

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* barSet = new QBarSet(dataSeries.name);
        barSet->setColor(dataSeries.color);
        barSet->setLabel(dataSeries.name);

        for (int j = 0; j < dataSeries.values.size() && j < data.categories.size(); ++j) {
            barSet->append(dataSeries.values[j]);
        }

        series->append(barSet);
    }

    chart->addSeries(series);
    setupAxes(chart, data, style);
    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createStackedBarChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto* series = new QStackedBarSeries();

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* barSet = new QBarSet(dataSeries.name);
        barSet->setColor(dataSeries.color);

        for (int j = 0; j < dataSeries.values.size() && j < data.categories.size(); ++j) {
            barSet->append(dataSeries.values[j]);
        }

        series->append(barSet);
    }

    chart->addSeries(series);
    setupAxes(chart, data, style);
    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createPercentBarChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto* series = new QPercentBarSeries();

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* barSet = new QBarSet(dataSeries.name);
        barSet->setColor(dataSeries.color);

        for (int j = 0; j < dataSeries.values.size() && j < data.categories.size(); ++j) {
            barSet->append(dataSeries.values[j]);
        }

        series->append(barSet);
    }

    chart->addSeries(series);
    setupAxes(chart, data, style);
    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createLineChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* series = new QLineSeries();
        series->setName(dataSeries.name);

        QPen pen(dataSeries.color);
        pen.setWidth(dataSeries.lineWidth);
        series->setPen(pen);

        for (int j = 0; j < dataSeries.values.size(); ++j) {
            series->append(j, dataSeries.values[j]);
        }

        chart->addSeries(series);
    }

    // 设置坐标轴
    auto* axisX = new QValueAxis();
    axisX->setTitleText(data.xAxisTitle);
    axisX->setGridLineVisible(style.showXAxisGrid);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    axisY->setTitleText(data.yAxisTitle);
    axisY->setGridLineVisible(style.showYAxisGrid);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto* series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createSplineChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* series = new QSplineSeries();
        series->setName(dataSeries.name);

        QPen pen(dataSeries.color);
        pen.setWidth(dataSeries.lineWidth);
        series->setPen(pen);

        for (int j = 0; j < dataSeries.values.size(); ++j) {
            series->append(j, dataSeries.values[j]);
        }

        chart->addSeries(series);
    }

    // 设置坐标轴
    auto* axisX = new QValueAxis();
    axisX->setTitleText(data.xAxisTitle);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    axisY->setTitleText(data.yAxisTitle);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto* series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createAreaChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    if (data.series.isEmpty()) {
        return chart;
    }

    const auto& dataSeries = data.series[0];
    auto* topSeries = new QLineSeries();
    auto* bottomSeries = new QLineSeries();

    for (int j = 0; j < dataSeries.values.size(); ++j) {
        topSeries->append(j, dataSeries.values[j]);
        bottomSeries->append(j, 0);
    }

    QPen pen(dataSeries.color);
    pen.setWidth(2);
    topSeries->setPen(pen);

    auto* areaSeries = new QAreaSeries(topSeries, bottomSeries);
    areaSeries->setName(dataSeries.name);
    areaSeries->setColor(dataSeries.color);

    chart->addSeries(areaSeries);

    // 设置坐标轴
    auto* axisX = new QValueAxis();
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);

    areaSeries->attachAxis(axisX);
    areaSeries->attachAxis(axisY);

    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createScatterChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* series = new QScatterSeries();
        series->setName(dataSeries.name);
        series->setColor(dataSeries.color);
        series->setMarkerSize(dataSeries.markerSize);

        for (int j = 0; j < dataSeries.values.size(); ++j) {
            series->append(j, dataSeries.values[j]);
        }

        chart->addSeries(series);
    }

    // 设置坐标轴
    auto* axisX = new QValueAxis();
    axisX->setTitleText(data.xAxisTitle);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto* axisY = new QValueAxis();
    axisY->setTitleText(data.yAxisTitle);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto* series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createPieChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto* series = new QPieSeries();

    if (!data.series.isEmpty()) {
        const auto& dataSeries = data.series[0];
        for (int i = 0; i < dataSeries.values.size() && i < data.categories.size(); ++i) {
            auto* slice = series->append(data.categories[i], dataSeries.values[i]);
            slice->setColor(getDefaultColor(i));
            slice->setLabelVisible(true);
        }
    }

    chart->addSeries(series);
    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createDonutChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = createPieChart(data, style);

    // 设置环形图的空心大小
    auto* pieSeries = dynamic_cast<QPieSeries*>(chart->series().first());
    if (pieSeries) {
        pieSeries->setHoleSize(0.3);
    }

    return chart;
}

void ChartHelper::setupAxes(QChart* chart, const ChartData& data, const ChartStyle& style)
{
    auto* categoryAxis = new QBarCategoryAxis();
    for (const auto& category : data.categories) {
        categoryAxis->append(category);
    }
    categoryAxis->setTitleText(data.xAxisTitle);

    auto* valueAxis = new QValueAxis();
    valueAxis->setTitleText(data.yAxisTitle);
    valueAxis->setGridLineVisible(style.showYAxisGrid);

    chart->addAxis(categoryAxis, Qt::AlignBottom);
    chart->addAxis(valueAxis, Qt::AlignLeft);

    for (auto* series : chart->series()) {
        series->attachAxis(categoryAxis);
        series->attachAxis(valueAxis);
    }
}

void ChartHelper::applyStyle(QChart* chart, const ChartStyle& style)
{
    chart->setBackgroundBrush(QBrush(style.backgroundColor));
    chart->setTitleFont(QFont("Arial", style.titleFontSize, QFont::Bold));
    chart->legend()->setVisible(style.showLegend);
    chart->legend()->setAlignment(Qt::AlignBottom);

    if (!style.animationEnabled) {
        chart->setAnimationOptions(QChart::NoAnimation);
    }
}

QChart* ChartHelper::createBoxPlotChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto* series = new QBoxPlotSeries();

    // 从第一个数据系列创建箱型图
    if (!data.series.isEmpty()) {
        const auto& dataSeries = data.series[0];

        // 计算五数概括
        auto summary = Statistics::DescriptiveStats::summarize(dataSeries.values);

        auto* boxSet = new QBoxSet(
            summary.min,
            summary.q1,
            summary.median,
            summary.q3,
            summary.max,
            dataSeries.name
        );

        boxSet->setBrush(QBrush(dataSeries.color));
        series->append(boxSet);
    }

    chart->addSeries(series);

    // 设置坐标轴
    auto* categoryAxis = new QBarCategoryAxis();
    if (!data.series.isEmpty()) {
        categoryAxis->append(data.series[0].name);
    }
    categoryAxis->setTitleText(data.xAxisTitle.isEmpty() ? "类别" : data.xAxisTitle);
    chart->addAxis(categoryAxis, Qt::AlignBottom);

    auto* valueAxis = new QValueAxis();
    valueAxis->setTitleText(data.yAxisTitle.isEmpty() ? "数值" : data.yAxisTitle);
    valueAxis->setGridLineVisible(style.showYAxisGrid);
    chart->addAxis(valueAxis, Qt::AlignLeft);

    series->attachAxis(categoryAxis);
    series->attachAxis(valueAxis);

    applyStyle(chart, style);

    return chart;
}

QChart* ChartHelper::createHorizontalBarChart(const ChartData& data, const ChartStyle& style)
{
    auto* chart = new QChart();
    chart->setTitle(data.title);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto* series = new QHorizontalBarSeries();

    for (int i = 0; i < data.series.size(); ++i) {
        const auto& dataSeries = data.series[i];
        if (!dataSeries.visible) continue;

        auto* barSet = new QBarSet(dataSeries.name);
        barSet->setColor(dataSeries.color);
        barSet->setLabel(dataSeries.name);

        for (int j = 0; j < dataSeries.values.size() && j < data.categories.size(); ++j) {
            barSet->append(dataSeries.values[j]);
        }

        series->append(barSet);
    }

    chart->addSeries(series);

    // 水平柱状图的坐标轴是反的
    auto* categoryAxis = new QBarCategoryAxis();
    for (const auto& category : data.categories) {
        categoryAxis->append(category);
    }
    categoryAxis->setTitleText(data.yAxisTitle);
    chart->addAxis(categoryAxis, Qt::AlignLeft);

    auto* valueAxis = new QValueAxis();
    valueAxis->setTitleText(data.xAxisTitle);
    valueAxis->setGridLineVisible(style.showYAxisGrid);
    chart->addAxis(valueAxis, Qt::AlignBottom);

    for (auto* seriesItem : chart->series()) {
        seriesItem->attachAxis(categoryAxis);
        seriesItem->attachAxis(valueAxis);
    }

    applyStyle(chart, style);

    return chart;
}

ChartData ChartHelper::createChartDataFromColumn(Core::TableData* tableData, int column)
{
    ChartData chartData;

    if (!tableData || column < 0 || column >= tableData->columnCount()) {
        return chartData;
    }

    chartData.title = tableData->header(column);
    chartData.xAxisTitle = "索引";
    chartData.yAxisTitle = tableData->header(column);

    QVector<double> values = tableData->toDoubleVector(column);
    DataSeries series(tableData->header(column), values);
    series.color = getDefaultColor(0);

    chartData.series.append(series);

    // 创建类别标签（使用行号）
    for (int i = 0; i < values.size(); ++i) {
        chartData.categories.append(QString::number(i + 1));
    }

    return chartData;
}

} // namespace Charts

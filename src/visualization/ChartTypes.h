#ifndef CHARTTYPES_H
#define CHARTTYPES_H

#include <QString>
#include <QColor>
#include <QVector>
#include <QMap>
#include <QSize>

namespace Charts {

/**
 * @brief 图表类型枚举
 */
enum class ChartType
{
    BarChart,                // 柱状图
    StackedBarChart,         // 堆叠柱状图
    PercentBarChart,         // 百分比柱状图
    HorizontalBarChart,      // 水平柱状图
    LineChart,               // 折线图
    SplineChart,             // 平滑曲线图
    AreaChart,               // 面积图
    ScatterChart,            // 散点图
    PieChart,                // 饼图
    DonutChart,              // 环形图
    BoxPlotChart,            // 箱型图
    None
};

/**
 * @brief 数据系列
 */
struct DataSeries
{
    QString name;                    // 系列名称
    QVector<double> values;          // 数值
    QVector<QString> labels;         // 标签
    QColor color;                    // 颜色
    bool visible = true;             // 是否可见
    int lineWidth = 2;               // 线宽
    int markerSize = 8;              // 标记大小

    DataSeries() = default;
    DataSeries(const QString& n, const QVector<double>& v)
        : name(n), values(v) {}
};

/**
 * @brief 图表数据
 */
struct ChartData
{
    QString title;                          // 图表标题
    QString xAxisTitle;                     // X轴标题
    QString yAxisTitle;                     // Y轴标题
    QVector<QString> categories;            // 类别标签（X轴）
    QVector<DataSeries> series;             // 数据系列

    bool isEmpty() const { return series.isEmpty(); }
    int seriesCount() const { return series.size(); }
};

/**
 * @brief 图表样式配置
 */
struct ChartStyle
{
    QColor backgroundColor = Qt::white;
    QColor axisColor = Qt::black;
    QColor gridColor = Qt::lightGray;
    QColor textColor = Qt::black;

    int titleFontSize = 16;
    int axisFontSize = 12;
    int labelFontSize = 10;

    bool showXAxisGrid = true;
    bool showYAxisGrid = true;

    bool showLegend = true;

    bool animationEnabled = true;
    int animationDuration = 500;

    bool tooltipsEnabled = true;
};

} // namespace Charts

#endif // CHARTTYPES_H

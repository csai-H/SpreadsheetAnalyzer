#ifndef CHARTHELPER_H
#define CHARTHELPER_H

#include "ChartTypes.h"
#include "../core/TableData.h"
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

namespace Charts {

/**
 * @brief 图表辅助工具类
 *
 * 提供快速创建Qt Charts的辅助函数
 */
class ChartHelper
{
public:
    // 创建柱状图
    static QChart* createBarChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建堆叠柱状图
    static QChart* createStackedBarChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建百分比柱状图
    static QChart* createPercentBarChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建折线图
    static QChart* createLineChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建平滑曲线图
    static QChart* createSplineChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建面积图
    static QChart* createAreaChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建散点图
    static QChart* createScatterChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建饼图
    static QChart* createPieChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建环形图
    static QChart* createDonutChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建箱型图
    static QChart* createBoxPlotChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 创建水平柱状图
    static QChart* createHorizontalBarChart(const ChartData& data, const ChartStyle& style = ChartStyle());

    // 获取默认颜色
    static QColor getDefaultColor(int index);

    // 从TableData创建ChartData
    static ChartData createChartDataFromColumn(Core::TableData* tableData, int column);

private:
    // 设置坐标轴
    static void setupAxes(QChart* chart, const ChartData& data, const ChartStyle& style);

    // 应用样式
    static void applyStyle(QChart* chart, const ChartStyle& style);
};

} // namespace Charts

#endif // CHARTHELPER_H

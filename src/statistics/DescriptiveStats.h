#ifndef DESCRIPTIVESTATS_H
#define DESCRIPTIVESTATS_H

#include "StatisticTypes.h"
#include <QVector>
#include <algorithm>
#include <cmath>

namespace Statistics {

/**
 * @brief 描述性统计计算器
 *
 * 提供各种描述性统计函数
 */
class DescriptiveStats
{
public:
    // === 集中趋势 ===

    /**
     * @brief 计算均值 (算术平均)
     */
    static StatisticResult mean(const QVector<double>& data);

    /**
     * @brief 计算加权均值
     */
    static StatisticResult weightedMean(const QVector<double>& data,
                                        const QVector<double>& weights);

    /**
     * @brief 计算中位数
     */
    static StatisticResult median(const QVector<double>& data);

    /**
     * @brief 计算众数
     */
    static StatisticResult mode(const QVector<double>& data);

    /**
     * @brief 计算几何均值
     */
    static StatisticResult geometricMean(const QVector<double>& data);

    /**
     * @brief 计算调和均值
     */
    static StatisticResult harmonicMean(const QVector<double>& data);

    // === 离散程度 ===

    /**
     * @brief 计算方差
     */
    static StatisticResult variance(const QVector<double>& data, bool sample = true);

    /**
     * @brief 计算标准差
     */
    static StatisticResult standardDeviation(const QVector<double>& data, bool sample = true);

    /**
     * @brief 计算变异系数 (CV = std/mean * 100%)
     */
    static StatisticResult coefficientOfVariation(const QVector<double>& data);

    /**
     * @brief 计算极差
     */
    static StatisticResult range(const QVector<double>& data);

    /**
     * @brief 计算四分位距 (IQR = Q3 - Q1)
     */
    static StatisticResult interquartileRange(const QVector<double>& data);

    // === 分位数 ===

    /**
     * @brief 计算分位数
     */
    static StatisticResult quantile(const QVector<double>& data, double percentile);

    /**
     * @brief 计算第一四分位数 (25%)
     */
    static StatisticResult q1(const QVector<double>& data);

    /**
     * @brief 计算第三四分位数 (75%)
     */
    static StatisticResult q3(const QVector<double>& data);

    // === 分布形状 ===

    /**
     * @brief 计算偏度 (Skewness)
     */
    static StatisticResult skewness(const QVector<double>& data);

    /**
     * @brief 计算峰度 (Kurtosis)
     */
    static StatisticResult kurtosis(const QVector<double>& data);

    // === 其他统计量 ===

    /**
     * @brief 计算总和
     */
    static StatisticResult sum(const QVector<double>& data);

    /**
     * @brief 计算乘积
     */
    static StatisticResult product(const QVector<double>& data);

    /**
     * @brief 统计有效值个数
     */
    static int count(const QVector<double>& data);

    /**
     * @brief 计算最小值
     */
    static StatisticResult min(const QVector<double>& data);

    /**
     * @brief 计算最大值
     */
    static StatisticResult max(const QVector<double>& data);

    /**
     * @brief 计算所有描述性统计量
     */
    static DescriptiveSummary summarize(const QVector<double>& data);

private:
    // 辅助函数
    static QVector<double> removeInvalid(const QVector<double>& data);
    static void sortData(QVector<double>& data);
    static double percentileLinear(const QVector<double>& sortedData, double percentile);
};

} // namespace Statistics

#endif // DESCRIPTIVESTATS_H

#ifndef STATISTICTYPES_H
#define STATISTICTYPES_H

#include <QVector>
#include <QString>
#include <QVariant>
#include <optional>

namespace Statistics {

/**
 * @brief 统计结果
 */
struct StatisticResult
{
    double value;
    bool isValid = true;
    QString errorMessage;

    static StatisticResult ok(double value);
    static StatisticResult error(const QString& message);
};

/**
 * @brief 描述性统计汇总
 */
struct DescriptiveSummary
{
    int count = 0;           // 有效数据个数
    double sum = 0.0;        // 总和
    double mean = 0.0;       // 均值
    double median = 0.0;     // 中位数
    double mode = 0.0;       // 众数
    double variance = 0.0;   // 方差
    double stdDev = 0.0;     // 标准差
    double min = 0.0;        // 最小值
    double max = 0.0;        // 最大值
    double range = 0.0;      // 极差
    double q1 = 0.0;         // 第一四分位数 (25%)
    double q3 = 0.0;         // 第三四分位数 (75%)
    double iqr = 0.0;        // 四分位距
    double skewness = 0.0;   // 偏度
    double kurtosis = 0.0;   // 峰度

    // 计算所有统计量
    void calculate(const QVector<double>& data);
};

/**
 * @brief 预测方法枚举
 */
enum class ForecastMethod
{
    SimpleMovingAverage,    // 简单移动平均
    WeightedMovingAverage,  // 加权移动平均
    ExponentialSmoothing,   // 指数平滑
    LinearRegression        // 线性回归
};

/**
 * @brief 预测结果
 */
struct ForecastResult
{
    QVector<double> predicted;     // 预测值
    QVector<double> lowerBound;    // 置信区间下限（可选）
    QVector<double> upperBound;    // 置信区间上限（可选）
    double errorMetric = 0.0;      // 误差指标 (MSE/MAE)
    bool isValid = true;
    QString errorMessage;
};

} // namespace Statistics

#endif // STATISTICTYPES_H

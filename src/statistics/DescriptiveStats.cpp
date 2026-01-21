#include "DescriptiveStats.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <QDebug>

namespace Statistics {

// === 集中趋势 ===

StatisticResult DescriptiveStats::mean(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);

    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    double sum = std::accumulate(validData.begin(), validData.end(), 0.0);
    return StatisticResult::ok(sum / validData.size());
}

StatisticResult DescriptiveStats::weightedMean(const QVector<double>& data,
                                               const QVector<double>& weights)
{
    if (data.size() != weights.size()) {
        return StatisticResult::error("数据和权重维度不匹配");
    }

    double sumWeighted = 0.0;
    double sumWeights = 0.0;

    for (int i = 0; i < data.size(); ++i) {
        if (qIsFinite(data[i]) && qIsFinite(weights[i])) {
            sumWeighted += data[i] * weights[i];
            sumWeights += weights[i];
        }
    }

    if (sumWeights == 0.0) {
        return StatisticResult::error("权重和为零");
    }

    return StatisticResult::ok(sumWeighted / sumWeights);
}

StatisticResult DescriptiveStats::median(const QVector<double>& data)
{
    QVector<double> sortedData = data;
    sortData(sortedData);

    QVector<double> validData = removeInvalid(sortedData);
    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    int n = validData.size();
    if (n % 2 == 0) {
        return StatisticResult::ok((validData[n/2 - 1] + validData[n/2]) / 2.0);
    } else {
        return StatisticResult::ok(validData[n/2]);
    }
}

StatisticResult DescriptiveStats::mode(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    // 简化实现：使用 QMap 统计频次
    QMap<double, int> frequency;
    for (double value : validData) {
        frequency[value]++;
    }

    double modeValue = validData[0];
    int maxCount = 0;
    for (auto it = frequency.begin(); it != frequency.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            modeValue = it.key();
        }
    }

    return StatisticResult::ok(modeValue);
}

StatisticResult DescriptiveStats::geometricMean(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);

    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    // 检查是否所有值都为正数
    for (double value : validData) {
        if (value <= 0.0) {
            return StatisticResult::error("几何均值要求所有数据为正数");
        }
    }

    double logSum = 0.0;
    for (double value : validData) {
        logSum += qLn(value);
    }

    return StatisticResult::ok(qExp(logSum / validData.size()));
}

StatisticResult DescriptiveStats::harmonicMean(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);

    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    // 检查是否所有值都为正数
    for (double value : validData) {
        if (value <= 0.0) {
            return StatisticResult::error("调和均值要求所有数据为正数");
        }
    }

    double sumReciprocal = 0.0;
    for (double value : validData) {
        sumReciprocal += 1.0 / value;
    }

    return StatisticResult::ok(validData.size() / sumReciprocal);
}

// === 离散程度 ===

StatisticResult DescriptiveStats::variance(const QVector<double>& data, bool sample)
{
    auto meanResult = mean(data);
    if (!meanResult.isValid) {
        return meanResult;
    }

    QVector<double> validData = removeInvalid(data);
    double meanValue = meanResult.value;

    double sumSquaredDiff = 0.0;
    for (double value : validData) {
        double diff = value - meanValue;
        sumSquaredDiff += diff * diff;
    }

    int n = validData.size();
    int denominator = sample ? (n - 1) : n;

    if (denominator <= 0) {
        return StatisticResult::error("样本量不足");
    }

    return StatisticResult::ok(sumSquaredDiff / denominator);
}

StatisticResult DescriptiveStats::standardDeviation(const QVector<double>& data, bool sample)
{
    auto varResult = variance(data, sample);
    if (!varResult.isValid) {
        return varResult;
    }

    return StatisticResult::ok(qSqrt(varResult.value));
}

StatisticResult DescriptiveStats::coefficientOfVariation(const QVector<double>& data)
{
    auto meanResult = mean(data);
    auto stdResult = standardDeviation(data, true);

    if (!meanResult.isValid || !stdResult.isValid) {
        return StatisticResult::error("无法计算变异系数");
    }

    if (meanResult.value == 0.0) {
        return StatisticResult::error("均值为零，无法计算变异系数");
    }

    return StatisticResult::ok((stdResult.value / meanResult.value) * 100.0);
}

StatisticResult DescriptiveStats::range(const QVector<double>& data)
{
    auto minResult = min(data);
    auto maxResult = max(data);

    if (!minResult.isValid || !maxResult.isValid) {
        return StatisticResult::error("无法计算极差");
    }

    return StatisticResult::ok(maxResult.value - minResult.value);
}

StatisticResult DescriptiveStats::interquartileRange(const QVector<double>& data)
{
    auto q1Result = q1(data);
    auto q3Result = q3(data);

    if (!q1Result.isValid || !q3Result.isValid) {
        return StatisticResult::error("无法计算四分位距");
    }

    return StatisticResult::ok(q3Result.value - q1Result.value);
}

// === 分位数 ===

StatisticResult DescriptiveStats::quantile(const QVector<double>& data, double percentile)
{
    if (percentile < 0.0 || percentile > 100.0) {
        return StatisticResult::error("分位数必须在0-100之间");
    }

    QVector<double> sortedData = data;
    sortData(sortedData);
    QVector<double> validData = removeInvalid(sortedData);

    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    return StatisticResult::ok(percentileLinear(validData, percentile));
}

StatisticResult DescriptiveStats::q1(const QVector<double>& data)
{
    return quantile(data, 25.0);
}

StatisticResult DescriptiveStats::q3(const QVector<double>& data)
{
    return quantile(data, 75.0);
}

// === 分布形状 ===

StatisticResult DescriptiveStats::skewness(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    if (validData.size() < 3) {
        return StatisticResult::error("样本量至少需要3个");
    }

    auto meanResult = mean(validData);
    auto stdResult = standardDeviation(validData, true);

    double meanValue = meanResult.value;
    double stdValue = stdResult.value;

    if (stdValue == 0.0) {
        return StatisticResult::ok(0.0);
    }

    double sum = 0.0;
    for (double value : validData) {
        double standardized = (value - meanValue) / stdValue;
        sum += standardized * standardized * standardized;
    }

    int n = validData.size();
    return StatisticResult::ok(sum / n);
}

StatisticResult DescriptiveStats::kurtosis(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    if (validData.size() < 4) {
        return StatisticResult::error("样本量至少需要4个");
    }

    auto meanResult = mean(validData);
    auto stdResult = standardDeviation(validData, true);

    double meanValue = meanResult.value;
    double stdValue = stdResult.value;

    if (stdValue == 0.0) {
        return StatisticResult::error("标准差为零");
    }

    double sum = 0.0;
    for (double value : validData) {
        double standardized = (value - meanValue) / stdValue;
        sum += standardized * standardized * standardized * standardized;
    }

    int n = validData.size();
    return StatisticResult::ok(sum / n);
}

// === 其他统计量 ===

StatisticResult DescriptiveStats::sum(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    double total = std::accumulate(validData.begin(), validData.end(), 0.0);
    return StatisticResult::ok(total);
}

StatisticResult DescriptiveStats::product(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    double prod = 1.0;
    for (double value : validData) {
        prod *= value;
    }

    return StatisticResult::ok(prod);
}

int DescriptiveStats::count(const QVector<double>& data)
{
    return removeInvalid(data).size();
}

StatisticResult DescriptiveStats::min(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    return StatisticResult::ok(*std::min_element(validData.begin(), validData.end()));
}

StatisticResult DescriptiveStats::max(const QVector<double>& data)
{
    QVector<double> validData = removeInvalid(data);
    if (validData.isEmpty()) {
        return StatisticResult::error("没有有效数据");
    }

    return StatisticResult::ok(*std::max_element(validData.begin(), validData.end()));
}

DescriptiveSummary DescriptiveStats::summarize(const QVector<double>& data)
{
    DescriptiveSummary summary;

    QVector<double> validData = removeInvalid(data);
    summary.count = validData.size();

    if (summary.count == 0) {
        return summary;
    }

    // 计算各项统计量
    auto sumResult = sum(validData);
    auto meanResult = mean(validData);
    auto medianResult = median(validData);
    auto varResult = variance(validData, true);
    auto stdResult = standardDeviation(validData, true);
    auto minResult = min(validData);
    auto maxResult = max(validData);

    summary.sum = sumResult.value;
    summary.mean = meanResult.value;
    summary.median = medianResult.value;
    summary.variance = varResult.value;
    summary.stdDev = stdResult.value;
    summary.min = minResult.value;
    summary.max = maxResult.value;
    summary.range = summary.max - summary.min;

    // 分位数
    summary.q1 = q1(validData).value;
    summary.q3 = q3(validData).value;
    summary.iqr = summary.q3 - summary.q1;

    // 分布形状
    summary.skewness = skewness(validData).value;
    summary.kurtosis = kurtosis(validData).value;

    return summary;
}

// === 辅助函数 ===

QVector<double> DescriptiveStats::removeInvalid(const QVector<double>& data)
{
    QVector<double> result;
    result.reserve(data.size());

    for (double value : data) {
        if (!qIsNaN(value) && !qIsInf(value)) {
            result.append(value);
        }
    }

    return result;
}

void DescriptiveStats::sortData(QVector<double>& data)
{
    std::sort(data.begin(), data.end());
}

double DescriptiveStats::percentileLinear(const QVector<double>& sortedData, double percentile)
{
    int n = sortedData.size();
    double index = (percentile / 100.0) * (n - 1);

    int lower = static_cast<int>(qFloor(index));
    int upper = static_cast<int>(qCeil(index));
    double weight = index - lower;

    if (upper >= n) {
        return sortedData[n - 1];
    }

    return sortedData[lower] * (1.0 - weight) + sortedData[upper] * weight;
}

} // namespace Statistics

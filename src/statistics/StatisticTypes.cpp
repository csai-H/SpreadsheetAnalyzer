#include "StatisticTypes.h"

namespace Statistics {

StatisticResult StatisticResult::ok(double value)
{
    StatisticResult result;
    result.value = value;
    result.isValid = true;
    return result;
}

StatisticResult StatisticResult::error(const QString& message)
{
    StatisticResult result;
    result.isValid = false;
    result.errorMessage = message;
    return result;
}

void DescriptiveSummary::calculate(const QVector<double>& data)
{
    // TODO: 实现完整的统计计算
    // 这里先提供一个简化版本
    if (data.isEmpty()) {
        return;
    }

    count = data.size();

    // 计算最小最大值
    min = data[0];
    max = data[0];
    sum = 0.0;

    for (double value : data) {
        if (value < min) min = value;
        if (value > max) max = value;
        sum += value;
    }

    range = max - min;
    mean = sum / count;

    // 计算方差
    double sumSquaredDiff = 0.0;
    for (double value : data) {
        double diff = value - mean;
        sumSquaredDiff += diff * diff;
    }

    variance = (count > 1) ? (sumSquaredDiff / (count - 1)) : 0.0;
    stdDev = qSqrt(variance);
}

} // namespace Statistics

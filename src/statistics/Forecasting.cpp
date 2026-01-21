#include "Forecasting.h"
#include <cmath>
#include <algorithm>
#include <QDebug>

namespace Statistics {

// === 简单移动平均 ===

ForecastResult Forecasting::simpleMovingAverage(const QVector<double>& data,
                                                  int window,
                                                  int forecastPeriods)
{
    ForecastResult result;

    if (data.size() < window || window <= 0) {
        result.isValid = false;
        result.errorMessage = "数据量不足或窗口大小无效";
        return result;
    }

    // 计算最后一个窗口的平均值
    double sum = 0.0;
    for (int i = data.size() - window; i < data.size(); ++i) {
        sum += data[i];
    }

    double lastMA = sum / window;

    // 生成预测值
    QVector<double> predictions;
    for (int i = 0; i < forecastPeriods; ++i) {
        predictions.append(lastMA);
    }

    result.predicted = predictions;
    result.isValid = true;

    // 计算拟合误差
    QVector<double> fitted;
    for (int i = window; i < data.size(); ++i) {
        double ma = 0.0;
        for (int j = i - window; j < i; ++j) {
            ma += data[j];
        }
        ma /= window;
        fitted.append(ma);
    }

    QVector<double> actual = data.mid(window);
    result.errorMetric = calculateMSE(actual, fitted);

    return result;
}

ForecastResult Forecasting::weightedMovingAverage(const QVector<double>& data,
                                                   const QVector<double>& weights,
                                                   int forecastPeriods)
{
    ForecastResult result;

    int window = weights.size();
    if (data.size() < window || window <= 0) {
        result.isValid = false;
        result.errorMessage = "数据量不足或权重维度无效";
        return result;
    }

    // 归一化权重
    double weightSum = std::accumulate(weights.begin(), weights.end(), 0.0);
    if (weightSum == 0.0) {
        result.isValid = false;
        result.errorMessage = "权重和为零";
        return result;
    }

    // 计算最后一个窗口的加权平均值
    double weightedSum = 0.0;
    for (int i = 0; i < window; ++i) {
        weightedSum += data[data.size() - window + i] * weights[i];
    }

    double lastWMA = weightedSum / weightSum;

    QVector<double> predictions;
    for (int i = 0; i < forecastPeriods; ++i) {
        predictions.append(lastWMA);
    }

    result.predicted = predictions;
    result.isValid = true;

    return result;
}

QVector<double> Forecasting::centeredMovingAverage(const QVector<double>& data, int window)
{
    QVector<double> result;

    if (window % 2 == 0) {
        qWarning() << "窗口大小应为奇数";
        window += 1;
    }

    int halfWindow = window / 2;

    for (int i = halfWindow; i < data.size() - halfWindow; ++i) {
        double sum = 0.0;
        for (int j = i - halfWindow; j <= i + halfWindow; ++j) {
            sum += data[j];
        }
        result.append(sum / window);
    }

    return result;
}

// === 指数平滑 ===

ForecastResult Forecasting::exponentialSmoothing(const QVector<double>& data,
                                                  double alpha,
                                                  int forecastPeriods)
{
    ForecastResult result;

    if (alpha <= 0.0 || alpha >= 1.0 || data.isEmpty()) {
        result.isValid = false;
        result.errorMessage = "参数无效";
        return result;
    }

    QVector<double> smoothed;

    // 初始化
    double level = data[0];
    smoothed.append(level);

    // 平滑
    for (int i = 1; i < data.size(); ++i) {
        level = alpha * data[i] + (1.0 - alpha) * level;
        smoothed.append(level);
    }

    // 预测
    QVector<double> predictions;
    for (int i = 0; i < forecastPeriods; ++i) {
        predictions.append(level);
    }

    result.predicted = predictions;
    result.isValid = true;
    result.errorMetric = calculateMSE(data, smoothed);

    return result;
}

ForecastResult Forecasting::doubleExponentialSmoothing(const QVector<double>& data,
                                                        double alpha,
                                                        double beta,
                                                        int forecastPeriods)
{
    ForecastResult result;

    if (alpha <= 0.0 || alpha >= 1.0 ||
        beta <= 0.0 || beta >= 1.0 ||
        data.size() < 2) {
        result.isValid = false;
        result.errorMessage = "参数无效或数据量不足";
        return result;
    }

    // 初始化
    double level = data[0];
    double trend = data[1] - data[0];

    QVector<double> fitted;

    // 平滑
    for (int i = 1; i < data.size(); ++i) {
        double lastLevel = level;
        level = alpha * data[i] + (1.0 - alpha) * (level + trend);
        trend = beta * (level - lastLevel) + (1.0 - beta) * trend;
        fitted.append(level + trend);
    }

    // 预测
    QVector<double> predictions;
    for (int i = 1; i <= forecastPeriods; ++i) {
        predictions.append(level + i * trend);
    }

    result.predicted = predictions;
    result.isValid = true;
    result.errorMetric = calculateMSE(data.mid(1), fitted);

    return result;
}

// === 线性回归 ===

ForecastResult Forecasting::linearRegression(const QVector<double>& data,
                                              int forecastPeriods)
{
    ForecastResult result;

    if (data.size() < 2) {
        result.isValid = false;
        result.errorMessage = "数据量不足";
        return result;
    }

    int n = data.size();

    // 构造时间序列
    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = i + 1;
    }

    // 最小二乘法求解 y = a + bx
    QVector<double> coeffs = leastSquares(x, data, 1);

    if (coeffs.size() < 2) {
        result.isValid = false;
        result.errorMessage = "回归失败";
        return result;
    }

    double a = coeffs[0];  // 截距
    double b = coeffs[1];  // 斜率

    QVector<double> predictions;
    QVector<double> fitted;

    // 拟合值
    for (int i = 0; i < n; ++i) {
        fitted.append(a + b * x[i]);
    }

    // 预测值
    for (int i = 0; i < forecastPeriods; ++i) {
        predictions.append(a + b * (n + i));
    }

    result.predicted = predictions;
    result.isValid = true;
    result.errorMetric = calculateMSE(data, fitted);

    return result;
}

// === 误差计算 ===

double Forecasting::calculateMSE(const QVector<double>& actual,
                                  const QVector<double>& predicted)
{
    if (actual.size() != predicted.size() || actual.isEmpty()) {
        return qQNaN();
    }

    double sum = 0.0;
    for (int i = 0; i < actual.size(); ++i) {
        double error = actual[i] - predicted[i];
        sum += error * error;
    }

    return sum / actual.size();
}

double Forecasting::calculateMAE(const QVector<double>& actual,
                                  const QVector<double>& predicted)
{
    if (actual.size() != predicted.size() || actual.isEmpty()) {
        return qQNaN();
    }

    double sum = 0.0;
    for (int i = 0; i < actual.size(); ++i) {
        sum += qAbs(actual[i] - predicted[i]);
    }

    return sum / actual.size();
}

double Forecasting::calculateMAPE(const QVector<double>& actual,
                                   const QVector<double>& predicted)
{
    if (actual.size() != predicted.size() || actual.isEmpty()) {
        return qQNaN();
    }

    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < actual.size(); ++i) {
        if (qAbs(actual[i]) > 1e-9) {
            sum += qAbs((actual[i] - predicted[i]) / actual[i]);
            ++count;
        }
    }

    return count > 0 ? (sum / count) * 100.0 : qQNaN();
}

// === 最小二乘法 ===

QVector<double> Forecasting::leastSquares(const QVector<double>& x,
                                           const QVector<double>& y,
                                           int degree)
{
    int n = x.size();
    int m = degree + 1;

    if (n < m) {
        qWarning() << "数据量不足，无法进行" << degree << "阶多项式拟合";
        return QVector<double>();
    }

    // 构造正规方程矩阵
    QVector<QVector<double>> XTX(m, QVector<double>(m, 0.0));
    QVector<double> XTy(m, 0.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> powers(m);
        powers[0] = 1.0;
        for (int j = 1; j < m; ++j) {
            powers[j] = powers[j-1] * x[i];
        }

        for (int row = 0; row < m; ++row) {
            for (int col = 0; col < m; ++col) {
                XTX[row][col] += powers[row] * powers[col];
            }
            XTy[row] += powers[row] * y[i];
        }
    }

    // 高斯消元法求解
    for (int col = 0; col < m; ++col) {
        // 找主元
        int maxRow = col;
        for (int row = col + 1; row < m; ++row) {
            if (qAbs(XTX[row][col]) > qAbs(XTX[maxRow][col])) {
                maxRow = row;
            }
        }

        // 交换行
        std::swap(XTX[col], XTX[maxRow]);
        std::swap(XTy[col], XTy[maxRow]);

        // 消元
        for (int row = col + 1; row < m; ++row) {
            double factor = XTX[row][col] / XTX[col][col];
            for (int i = col; i < m; ++i) {
                XTX[row][i] -= factor * XTX[col][i];
            }
            XTy[row] -= factor * XTy[col];
        }
    }

    // 回代
    QVector<double> coefficients(m);
    for (int i = m - 1; i >= 0; --i) {
        coefficients[i] = XTy[i];
        for (int j = i + 1; j < m; ++j) {
            coefficients[i] -= XTX[i][j] * coefficients[j];
        }
        coefficients[i] /= XTX[i][i];
    }

    return coefficients;
}

} // namespace Statistics

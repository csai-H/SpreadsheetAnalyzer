#ifndef FORECASTING_H
#define FORECASTING_H

#include "StatisticTypes.h"
#include <QVector>

namespace Statistics {

/**
 * @brief 预测算法类
 *
 * 提供各种时间序列预测方法
 */
class Forecasting
{
public:
    // === 移动平均 ===

    /**
     * @brief 简单移动平均
     * @param data 历史数据
     * @param window 窗口大小
     * @param forecastPeriods 预测期数
     * @return 预测结果
     */
    static ForecastResult simpleMovingAverage(const QVector<double>& data,
                                               int window,
                                               int forecastPeriods = 1);

    /**
     * @brief 加权移动平均
     */
    static ForecastResult weightedMovingAverage(const QVector<double>& data,
                                                const QVector<double>& weights,
                                                int forecastPeriods = 1);

    /**
     * @brief 中心移动平均
     */
    static QVector<double> centeredMovingAverage(const QVector<double>& data, int window);

    // === 指数平滑 ===

    /**
     * @brief 简单指数平滑
     */
    static ForecastResult exponentialSmoothing(const QVector<double>& data,
                                                double alpha,
                                                int forecastPeriods = 1);

    /**
     * @brief 双指数平滑（Holt 线性趋势方法）
     */
    static ForecastResult doubleExponentialSmoothing(const QVector<double>& data,
                                                      double alpha,
                                                      double beta,
                                                      int forecastPeriods = 1);

    // === 线性回归 ===

    /**
     * @brief 简单线性回归
     */
    static ForecastResult linearRegression(const QVector<double>& data,
                                            int forecastPeriods = 1);

    // === 误差计算 ===

    /**
     * @brief 计算均方误差 (MSE)
     */
    static double calculateMSE(const QVector<double>& actual,
                                const QVector<double>& predicted);

    /**
     * @brief 计算平均绝对误差 (MAE)
     */
    static double calculateMAE(const QVector<double>& actual,
                                const QVector<double>& predicted);

    /**
     * @brief 计算平均绝对百分比误差 (MAPE)
     */
    static double calculateMAPE(const QVector<double>& actual,
                                 const QVector<double>& predicted);

private:
    // 最小二乘法求解
    static QVector<double> leastSquares(const QVector<double>& x,
                                         const QVector<double>& y,
                                         int degree);
};

} // namespace Statistics

#endif // FORECASTING_H

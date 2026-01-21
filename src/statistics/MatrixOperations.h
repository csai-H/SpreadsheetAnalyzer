#ifndef MATRIXOPERATIONS_H
#define MATRIXOPERATIONS_H

#include <QVector>
#include <QMap>
#include <optional>

namespace Statistics {

/**
 * @brief 矩阵运算类
 *
 * 提供常用的矩阵运算功能
 */
class MatrixOperations
{
public:
    using Matrix = QVector<QVector<double>>;

    // === 基本运算 ===

    /**
     * @brief 矩阵加法
     */
    static Matrix add(const Matrix& A, const Matrix& B);

    /**
     * @brief 矩阵减法
     */
    static Matrix subtract(const Matrix& A, const Matrix& B);

    /**
     * @brief 矩阵数乘
     */
    static Matrix multiply(const Matrix& A, double scalar);

    /**
     * @brief 矩阵乘法
     */
    static Matrix multiply(const Matrix& A, const Matrix& B);

    /**
     * @brief 矩阵转置
     */
    static Matrix transpose(const Matrix& A);

    /**
     * @brief 矩阵行列式
     */
    static std::optional<double> determinant(const Matrix& A);

    /**
     * @brief 矩阵求逆
     */
    static std::optional<Matrix> inverse(const Matrix& A);

    // === 向量运算 ===

    /**
     * @brief 向量点积
     */
    static double dotProduct(const QVector<double>& a, const QVector<double>& b);

    /**
     * @brief 向量范数
     */
    static double norm(const QVector<double>& v, int p = 2);

    /**
     * @brief 向量归一化
     */
    static QVector<double> normalize(const QVector<double>& v);

    // === 特殊运算 ===

    /**
     * @brief 按行求和
     */
    static QVector<double> rowSum(const Matrix& A);

    /**
     * @brief 按列求和
     */
    static QVector<double> columnSum(const Matrix& A);

    /**
     * @brief 按行求均值
     */
    static QVector<double> rowMean(const Matrix& A);

    /**
     * @brief 按列求均值
     */
    static QVector<double> columnMean(const Matrix& A);

    // === 差分运算 ===

    /**
     * @brief 一阶差分
     */
    static QVector<double> diff(const QVector<double>& data, int periods = 1);

    /**
     * @brief 一阶差分（百分比变化）
     */
    static QVector<double> percentChange(const QVector<double>& data, int periods = 1);

    // === 工具函数 ===

    /**
     * @brief 创建单位矩阵
     */
    static Matrix identity(int size);

    /**
     * @brief 创建零矩阵
     */
    static Matrix zeros(int rows, int cols);

    /**
     * @brief 提取子矩阵
     */
    static Matrix submatrix(const Matrix& A, int rowStart, int rowEnd,
                            int colStart, int colEnd);

    /**
     * @brief 水平拼接矩阵
     */
    static Matrix horzcat(const Matrix& A, const Matrix& B);

    /**
     * @brief 垂直拼接矩阵
     */
    static Matrix vertcat(const Matrix& A, const Matrix& B);
};

} // namespace Statistics

#endif // MATRIXOPERATIONS_H

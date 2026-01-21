#include "MatrixOperations.h"
#include <QDebug>
#include <algorithm>

namespace Statistics {

// === 基本运算 ===

MatrixOperations::Matrix MatrixOperations::add(const Matrix& A, const Matrix& B)
{
    if (A.size() != B.size() || (A.size() > 0 && A[0].size() != B[0].size())) {
        qWarning() << "矩阵维度不匹配";
        return Matrix();
    }

    Matrix result(A.size(), QVector<double>(A[0].size()));

    for (int i = 0; i < A.size(); ++i) {
        for (int j = 0; j < A[i].size(); ++j) {
            result[i][j] = A[i][j] + B[i][j];
        }
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::subtract(const Matrix& A, const Matrix& B)
{
    if (A.size() != B.size() || (A.size() > 0 && A[0].size() != B[0].size())) {
        qWarning() << "矩阵维度不匹配";
        return Matrix();
    }

    Matrix result(A.size(), QVector<double>(A[0].size()));

    for (int i = 0; i < A.size(); ++i) {
        for (int j = 0; j < A[i].size(); ++j) {
            result[i][j] = A[i][j] - B[i][j];
        }
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::multiply(const Matrix& A, double scalar)
{
    Matrix result(A.size(), QVector<double>(A[0].size()));

    for (int i = 0; i < A.size(); ++i) {
        for (int j = 0; j < A[i].size(); ++j) {
            result[i][j] = A[i][j] * scalar;
        }
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::multiply(const Matrix& A, const Matrix& B)
{
    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    if (colsA != rowsB) {
        qWarning() << "矩阵维度不匹配，无法相乘";
        return Matrix();
    }

    Matrix result(rowsA, QVector<double>(colsB, 0.0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::transpose(const Matrix& A)
{
    int rows = A.size();
    int cols = A[0].size();

    Matrix result(cols, QVector<double>(rows));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[j][i] = A[i][j];
        }
    }

    return result;
}

std::optional<double> MatrixOperations::determinant(const Matrix& A)
{
    int n = A.size();

    if (n == 0) return 0.0;
    if (n == 1) return A[0][0];
    if (n == 2) return A[0][0] * A[1][1] - A[0][1] * A[1][0];

    // LU 分解计算行列式（简化版本）
    Matrix LU = A;
    double det = 1.0;

    for (int i = 0; i < n; ++i) {
        // 找主元
        int pivot = i;
        for (int row = i + 1; row < n; ++row) {
            if (qAbs(LU[row][i]) > qAbs(LU[pivot][i])) {
                pivot = row;
            }
        }

        // 交换行
        if (pivot != i) {
            std::swap(LU[i], LU[pivot]);
            det *= -1.0;
        }

        det *= LU[i][i];

        if (qAbs(LU[i][i]) < 1e-10) {
            return 0.0;  // 奇异矩阵
        }

        // 消元
        for (int row = i + 1; row < n; ++row) {
            double factor = LU[row][i] / LU[i][i];
            for (int col = i; col < n; ++col) {
                LU[row][col] -= factor * LU[i][col];
            }
        }
    }

    return det;
}

std::optional<MatrixOperations::Matrix> MatrixOperations::inverse(const Matrix& A)
{
    int n = A.size();

    if (n == 0) return Matrix();

    auto detResult = determinant(A);
    if (!detResult.has_value() || qAbs(detResult.value()) < 1e-10) {
        qWarning() << "矩阵不可逆";
        return std::nullopt;
    }

    // 高斯-约旦消元法求逆
    Matrix augmented = A;
    Matrix result = identity(n);

    for (int col = 0; col < n; ++col) {
        // 找主元
        int pivot = col;
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(augmented[row][col]) > qAbs(augmented[pivot][col])) {
                pivot = row;
            }
        }

        // 交换行
        std::swap(augmented[col], augmented[pivot]);
        std::swap(result[col], result[pivot]);

        // 归一化
        double pivotValue = augmented[col][col];
        if (qAbs(pivotValue) < 1e-10) {
            return std::nullopt;
        }

        for (int j = 0; j < n; ++j) {
            augmented[col][j] /= pivotValue;
            result[col][j] /= pivotValue;
        }

        // 消元
        for (int row = 0; row < n; ++row) {
            if (row != col) {
                double factor = augmented[row][col];
                for (int j = 0; j < n; ++j) {
                    augmented[row][j] -= factor * augmented[col][j];
                    result[row][j] -= factor * result[col][j];
                }
            }
        }
    }

    return result;
}

// === 向量运算 ===

double MatrixOperations::dotProduct(const QVector<double>& a, const QVector<double>& b)
{
    if (a.size() != b.size()) {
        qWarning() << "向量维度不匹配";
        return 0.0;
    }

    double result = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        result += a[i] * b[i];
    }

    return result;
}

double MatrixOperations::norm(const QVector<double>& v, int p)
{
    double sum = 0.0;
    for (double value : v) {
        sum += qPow(qAbs(value), p);
    }
    return qPow(sum, 1.0 / p);
}

QVector<double> MatrixOperations::normalize(const QVector<double>& v)
{
    double n = norm(v, 2);
    if (n == 0.0) {
        return v;
    }

    QVector<double> result(v.size());
    for (int i = 0; i < v.size(); ++i) {
        result[i] = v[i] / n;
    }

    return result;
}

// === 特殊运算 ===

QVector<double> MatrixOperations::rowSum(const Matrix& A)
{
    QVector<double> result(A.size());

    for (int i = 0; i < A.size(); ++i) {
        result[i] = std::accumulate(A[i].begin(), A[i].end(), 0.0);
    }

    return result;
}

QVector<double> MatrixOperations::columnSum(const Matrix& A)
{
    if (A.isEmpty()) return QVector<double>();

    int cols = A[0].size();
    QVector<double> result(cols, 0.0);

    for (int i = 0; i < A.size(); ++i) {
        for (int j = 0; j < cols; ++j) {
            result[j] += A[i][j];
        }
    }

    return result;
}

QVector<double> MatrixOperations::rowMean(const Matrix& A)
{
    QVector<double> sums = rowSum(A);
    for (int i = 0; i < sums.size(); ++i) {
        if (!A[i].isEmpty()) {
            sums[i] /= A[i].size();
        }
    }
    return sums;
}

QVector<double> MatrixOperations::columnMean(const Matrix& A)
{
    QVector<double> sums = columnSum(A);
    int rows = A.size();

    for (int i = 0; i < sums.size(); ++i) {
        sums[i] /= rows;
    }

    return sums;
}

// === 差分运算 ===

QVector<double> MatrixOperations::diff(const QVector<double>& data, int periods)
{
    QVector<double> result;

    for (int i = periods; i < data.size(); ++i) {
        result.append(data[i] - data[i - periods]);
    }

    return result;
}

QVector<double> MatrixOperations::percentChange(const QVector<double>& data, int periods)
{
    QVector<double> result;

    for (int i = periods; i < data.size(); ++i) {
        if (qAbs(data[i - periods]) > 1e-10) {
            double change = (data[i] - data[i - periods]) / data[i - periods] * 100.0;
            result.append(change);
        } else {
            result.append(qQNaN());
        }
    }

    return result;
}

// === 工具函数 ===

MatrixOperations::Matrix MatrixOperations::identity(int size)
{
    Matrix result(size, QVector<double>(size, 0.0));

    for (int i = 0; i < size; ++i) {
        result[i][i] = 1.0;
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::zeros(int rows, int cols)
{
    return Matrix(rows, QVector<double>(cols, 0.0));
}

MatrixOperations::Matrix MatrixOperations::submatrix(const Matrix& A,
                                                      int rowStart, int rowEnd,
                                                      int colStart, int colEnd)
{
    Matrix result;

    for (int i = rowStart; i <= rowEnd && i < A.size(); ++i) {
        QVector<double> row;
        for (int j = colStart; j <= colEnd && j < A[i].size(); ++j) {
            row.append(A[i][j]);
        }
        result.append(row);
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::horzcat(const Matrix& A, const Matrix& B)
{
    if (A.size() != B.size()) {
        qWarning() << "矩阵行数不匹配";
        return Matrix();
    }

    Matrix result;

    for (int i = 0; i < A.size(); ++i) {
        QVector<double> row = A[i];
        row.append(B[i]);
        result.append(row);
    }

    return result;
}

MatrixOperations::Matrix MatrixOperations::vertcat(const Matrix& A, const Matrix& B)
{
    Matrix result = A;
    for (const auto& row : B) {
        result.append(row);
    }
    return result;
}

} // namespace Statistics

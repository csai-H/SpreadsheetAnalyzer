#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "core/TableData.h"
#include "statistics/DescriptiveStats.h"
#include "statistics/Forecasting.h"

/**
 * @brief 统计计算对话框
 *
 * 提供描述性统计、预测分析等功能
 */
class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = nullptr);
    ~StatisticsDialog() override;

    void setTableData(Core::TableData *data);

private slots:
    void onCalculateClicked();
    void onColumnChanged(int index);
    void onTabChanged(int index);

private:
    void setupUI();
    void calculateDescriptiveStats();
    void calculateForecasting();
    void displaySummary(const Statistics::DescriptiveSummary &summary);

    QTabWidget *m_tabWidget;

    // 描述性统计标签页
    QWidget *m_descriptiveTab;
    QComboBox *m_columnCombo;
    QPushButton *m_calculateButton;
    QTextEdit *m_resultsTextEdit;

    // 预测分析标签页
    QWidget *m_forecastingTab;
    QComboBox *m_forecastColumnCombo;
    QComboBox *m_forecastMethodCombo;
    QSpinBox *m_windowSpinBox;
    QDoubleSpinBox *m_alphaSpinBox;
    QSpinBox *m_forecastPeriodsSpinBox;
    QPushButton *m_forecastButton;
    QTextEdit *m_forecastResultsTextEdit;

    Core::TableData *m_tableData;
};

#endif // STATISTICSDIALOG_H

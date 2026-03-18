#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSharedPointer>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>

#include "core/TableData.h"
#include "visualization/ChartTypes.h"

/**
 * @brief 图表视图
 *
 * 显示和操作图表
 */
class ChartView : public QWidget
{
    Q_OBJECT

public:
    explicit ChartView(QWidget *parent = nullptr);
    ~ChartView() override;

    // 图表操作
    void setChartType(const QString &typeName);
    void refreshChart();
    void setChartData(const Charts::ChartData &data);
    void setTableData(const QSharedPointer<Core::TableData> &data, int column);

private slots:
    void onChartTypeChanged(int index);
    void onTitleChanged(const QString &title);
    void onSaveImage();

private:
    void setupToolbar();
    void updateChart();
    void createSampleChart();

    QChartView *m_chartView;
    QChart *m_currentChart;
    Charts::ChartData m_chartData;
    QSharedPointer<Core::TableData> m_tableData;

    // 工具栏
    QComboBox *m_chartTypeCombo;
    QLineEdit *m_titleEdit;
    QPushButton *m_saveButton;
    QPushButton *m_refreshButton;
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
};

#endif // CHARTVIEW_H

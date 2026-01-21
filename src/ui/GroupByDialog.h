#ifndef GROUPBYDIALOG_H
#define GROUPBYDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QComboBox>
#include <QTreeWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>

/**
 * @brief 数据分组对话框
 *
 * 支持按列分组进行统计汇总
 */
class GroupByDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GroupByDialog(QTableView* tableView, QWidget* parent = nullptr);
    ~GroupByDialog() override;

private slots:
    void onGroupByClicked();
    void onClearGrouping();
    void onExportResults();

private:
    void setupUI();
    void populateColumns();
    QMap<QString, QMap<QString, double>> calculateStatistics(const QMap<QString, QList<QList<double>>>& groupedData);

    QTableView* m_tableView;

    // UI组件
    QComboBox* m_groupByCombo;        // 分组列
    QComboBox* m_aggregateCombo;      // 汇总方式
    QTreeWidget* m_groupTreeWidget;   // 分组树形结构
    QTableWidget* m_resultsTable;      // 结果表格
    QPushButton *m_groupByButton;
    QPushButton *m_clearButton;
    QPushButton * m_exportButton;
};

#endif // GROUPBYDIALOG_H

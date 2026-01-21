#ifndef CALCCOLUMNDIALOG_H
#define CALCCOLUMNDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVector>

/**
 * @brief 计算列对话框
 *
 * 用于基于现有列计算新列
 */
class CalcColumnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalcColumnDialog(QTableView* tableView, QWidget* parent = nullptr);
    ~CalcColumnDialog() override;

private slots:
    void onAddColumn();
    void onRemoveColumn();
    void onApply();
    void onOperationChanged(int index);

private:
    void setupUI();
    void updateAvailableColumns();
    void performCalculation();

    QTableView* m_tableView;
    QVector<QString> m_availableColumns;

    // 界面组件
    QListWidget* m_sourceColumnsList;
    QListWidget* m_calculatedColumnsList;
    QComboBox* m_operationCombo;
    QLineEdit* m_columnNameEdit;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_applyButton;

    // 计算类型
    enum CalculationType {
        Add,            // 加法
        Subtract,       // 减法
        Multiply,       // 乘法
        Percentage,     // 百分比
        Difference,      // 差分
        GrowthRate      // 增长率
    };
};

#endif // CALCCOLUMNDIALOG_H

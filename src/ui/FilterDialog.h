#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QStandardItemModel>

/**
 * @brief 数据筛选对话框
 *
 * 用于对表格数据进行条件筛选
 */
class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QTableView* tableView, QWidget* parent = nullptr);
    ~FilterDialog() override;

private slots:
    void onApplyFilter();
    void onClearFilter();
    void onFilterTypeChanged(int index);

private:
    void setupUI();
    void applyFilter();

    QTableView* m_tableView;
    QStandardItemModel* m_model;

    // 筛选控件
    QComboBox* m_columnCombo;
    QComboBox* m_conditionCombo;
    QLineEdit* m_valueEdit;

    // 筛选条件
    struct FilterCondition {
        enum Type {
            Equals,          // 等于
            NotEquals,       // 不等于
            GreaterThan,     // 大于
            LessThan,        // 小于
            GreaterOrEqual,  // 大于等于
            LessOrEqual,     // 小于等于
            Contains,        // 包含
            NotContains,     // 不包含
            StartsWith,      // 开始于
            EndsWith,        // 结束于
            IsEmpty,         // 为空
            IsNotEmpty       // 不为空
        };
    };

    int currentColumn() const;
    FilterCondition::Type currentCondition() const;
    QString filterValue() const;
};

#endif // FILTERDIALOG_H

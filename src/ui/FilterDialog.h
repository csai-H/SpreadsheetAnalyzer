#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

class DataTableView;

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QTableView *tableView, QWidget *parent = nullptr);
    ~FilterDialog() override;

private slots:
    void onApplyFilter();
    void onClearFilter();
    void onFilterTypeChanged(int index);

private:
    struct FilterCondition {
        enum Type {
            Equals,
            NotEquals,
            GreaterThan,
            LessThan,
            GreaterOrEqual,
            LessOrEqual,
            Contains,
            NotContains,
            StartsWith,
            EndsWith,
            IsEmpty,
            IsNotEmpty
        };
    };

    void setupUI();
    int currentColumn() const;
    FilterCondition::Type currentCondition() const;
    QString filterValue() const;

    QTableView *m_tableView;
    DataTableView *m_dataTableView;
    QComboBox *m_columnCombo;
    QComboBox *m_conditionCombo;
    QLineEdit *m_valueEdit;
};

#endif // FILTERDIALOG_H

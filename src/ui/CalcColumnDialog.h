#ifndef CALCCOLUMNDIALOG_H
#define CALCCOLUMNDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QTableView>
#include <QVariant>
#include <QVector>

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
    enum CalculationType {
        Add,
        Subtract,
        Multiply,
        Divide,
        Percentage,
        Difference,
        GrowthRate
    };

    struct CalculationSpec {
        QString columnName;
        CalculationType type = Add;
        QVector<int> sourceColumns;
    };

    void setupUI();
    void updateAvailableColumns();
    QString operationName(CalculationType type) const;
    QString buildCalculationDescription(const CalculationSpec& spec) const;
    int requiredColumnCount(CalculationType type) const;
    bool calculateValues(const CalculationSpec& spec,
                         QVector<QVariant>* values,
                         QString* errorMessage) const;
    bool parseNumber(const QVariant& value, double* number, bool* isEmpty) const;

    QTableView* m_tableView;
    QVector<QString> m_availableColumns;
    QVector<CalculationSpec> m_pendingCalculations;

    QLabel* m_leftOperandLabel;
    QComboBox* m_leftOperandCombo;
    QLabel* m_rightOperandLabel;
    QComboBox* m_rightOperandCombo;
    QListWidget* m_calculatedColumnsList;
    QComboBox* m_operationCombo;
    QLineEdit* m_columnNameEdit;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_applyButton;
};

#endif // CALCCOLUMNDIALOG_H

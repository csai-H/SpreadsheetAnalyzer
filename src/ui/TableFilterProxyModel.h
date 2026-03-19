#ifndef TABLEFILTERPROXYMODEL_H
#define TABLEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class TableFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum class Condition {
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

    explicit TableFilterProxyModel(QObject *parent = nullptr);

    void setFilterRule(int column, Condition condition, const QString &value);
    void clearFilterRule();
    bool hasActiveFilter() const;
    int filterColumn() const;
    Condition filterCondition() const;
    QString filterValue() const;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool matches(const QString &cellText) const;

    int m_filterColumn = -1;
    Condition m_condition = Condition::Equals;
    QString m_filterValue;
    bool m_hasFilter = false;
};

#endif // TABLEFILTERPROXYMODEL_H

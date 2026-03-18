#include "TableFilterProxyModel.h"

#include <QAbstractItemModel>

TableFilterProxyModel::TableFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(false);
}

void TableFilterProxyModel::setFilterRule(int column, Condition condition, const QString &value)
{
    beginFilterChange();
    m_filterColumn = column;
    m_condition = condition;
    m_filterValue = value;
    m_hasFilter = true;
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void TableFilterProxyModel::clearFilterRule()
{
    beginFilterChange();
    m_filterColumn = -1;
    m_filterValue.clear();
    m_hasFilter = false;
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

bool TableFilterProxyModel::hasActiveFilter() const
{
    return m_hasFilter;
}

bool TableFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_hasFilter || m_filterColumn < 0 || !sourceModel()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, m_filterColumn, sourceParent);
    const QString cellText = sourceModel()->data(index, Qt::DisplayRole).toString();
    return matches(cellText);
}

bool TableFilterProxyModel::matches(const QString &cellText) const
{
    switch (m_condition) {
    case Condition::Equals:
        return cellText == m_filterValue;
    case Condition::NotEquals:
        return cellText != m_filterValue;
    case Condition::GreaterThan:
    case Condition::LessThan:
    case Condition::GreaterOrEqual:
    case Condition::LessOrEqual: {
        bool leftOk = false;
        bool rightOk = false;
        const double left = cellText.toDouble(&leftOk);
        const double right = m_filterValue.toDouble(&rightOk);
        if (!leftOk || !rightOk) {
            return false;
        }

        switch (m_condition) {
        case Condition::GreaterThan:
            return left > right;
        case Condition::LessThan:
            return left < right;
        case Condition::GreaterOrEqual:
            return left >= right;
        case Condition::LessOrEqual:
            return left <= right;
        default:
            return false;
        }
    }
    case Condition::Contains:
        return cellText.contains(m_filterValue);
    case Condition::NotContains:
        return !cellText.contains(m_filterValue);
    case Condition::StartsWith:
        return cellText.startsWith(m_filterValue);
    case Condition::EndsWith:
        return cellText.endsWith(m_filterValue);
    case Condition::IsEmpty:
        return cellText.isEmpty();
    case Condition::IsNotEmpty:
        return !cellText.isEmpty();
    }

    return true;
}

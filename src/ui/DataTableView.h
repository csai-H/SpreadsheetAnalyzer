#ifndef DATATABLEVIEW_H
#define DATATABLEVIEW_H

#include <QTableView>
#include <QStandardItemModel>
#include <QMenu>
#include "../core/TableData.h"

/**
 * @brief 数据表格视图
 *
 * 显示和编辑表格数据
 */
class DataTableView : public QTableView
{
    Q_OBJECT

public:
    explicit DataTableView(QWidget *parent = nullptr);
    ~DataTableView() override;

    // 数据操作
    bool loadFile(const QString &filePath);
    bool saveFile(const QString &filePath);
    void clearData();

    // 数据获取
    Core::TableData *tableData() const;
    QString selectedRangeInfo() const;

signals:
    void dataChanged();
    void selectionChanged();
    void fileLoaded(const QString &filePath);

private slots:
    void onContextMenuRequested(const QPoint &pos);
    void onHeaderClicked(int column);

private:
    void setupContextMenu();

    // 排序辅助函数
    void sortColumn(int column, Qt::SortOrder order = Qt::AscendingOrder);
    bool isNumericColumn(int column) const;

    QStandardItemModel *m_model;
    Core::TableData *m_tableData;
    QMenu *m_contextMenu;

    // 排序状态跟踪
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

#endif // DATATABLEVIEW_H

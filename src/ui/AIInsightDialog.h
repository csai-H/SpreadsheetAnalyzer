#ifndef AIINSIGHTDIALOG_H
#define AIINSIGHTDIALOG_H

#include <QDialog>
#include <QSharedPointer>

namespace Core {
class TableData;
}

class QLabel;
class QPushButton;
class QPlainTextEdit;
class QTextEdit;
class AIInsightService;

class AIInsightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AIInsightDialog(const QSharedPointer<Core::TableData> &data,
                             bool filteredView,
                             QWidget *parent = nullptr);
    ~AIInsightDialog() override;

private slots:
    void onGenerateClicked();
    void onCopyClicked();
    void onRequestStarted();
    void onRequestFinished(const QString &insights);
    void onRequestFailed(const QString &errorMessage);

private:
    void updateSummaryLabel();

    QSharedPointer<Core::TableData> m_data;
    bool m_filteredView = false;

    QLabel *m_summaryLabel = nullptr;
    QPlainTextEdit *m_focusEdit = nullptr;
    QTextEdit *m_resultEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_generateButton = nullptr;
    QPushButton *m_copyButton = nullptr;

    AIInsightService *m_service = nullptr;
};

#endif // AIINSIGHTDIALOG_H

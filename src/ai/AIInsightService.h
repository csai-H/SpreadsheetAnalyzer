#ifndef AIINSIGHTSERVICE_H
#define AIINSIGHTSERVICE_H

#include <QObject>
#include <QJsonValue>
#include <QSharedPointer>
#include <QString>

namespace Core {
class TableData;
}

class QNetworkAccessManager;
class QNetworkReply;

class AIInsightService : public QObject
{
    Q_OBJECT

public:
    explicit AIInsightService(QObject *parent = nullptr);

    void requestInsights(const QSharedPointer<Core::TableData> &data,
                         const QString &focusPrompt,
                         bool filteredView,
                         bool useDataContext = true);
    bool isBusy() const;

    static QString defaultEndpoint();
    static QString defaultModel();
    static QString configuredApiKey();
    static QString configuredEndpoint();
    static QString configuredModel();

signals:
    void requestStarted();
    void requestFinished(const QString &insights);
    void requestFailed(const QString &errorMessage);

private:
    QString buildSystemPrompt() const;
    QString buildGeneralPrompt(const QString &focusPrompt) const;
    QString buildPrompt(const Core::TableData &data,
                        const QString &focusPrompt,
                        bool filteredView) const;
    QString buildDatasetSummary(const Core::TableData &data,
                                bool filteredView) const;
    QString objectKeys(const QJsonObject &object) const;
    QString responseDumpPath() const;
    void writeResponseDump(const QByteArray &body) const;
    QString extractTextContent(const QJsonValue &contentValue) const;
    QString sanitizeAssistantText(const QString &text) const;
    void handleReply(QNetworkReply *reply);

    QNetworkAccessManager *m_networkManager = nullptr;
    bool m_busy = false;
};

#endif // AIINSIGHTSERVICE_H

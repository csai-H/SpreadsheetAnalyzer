#ifndef AIASSISTANTPANEL_H
#define AIASSISTANTPANEL_H

#include <QEvent>
#include <QPair>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <QWidget>

namespace Core {
class TableData;
}

class QLabel;
class QPushButton;
class QPlainTextEdit;
class QTextBrowser;
class QTimer;
class QToolButton;
class AIInsightService;

class AIAssistantPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AIAssistantPanel(QWidget *parent = nullptr);

    void setContext(const QSharedPointer<Core::TableData> &data,
                    bool filteredView,
                    const QString &fileName);
    void focusComposer();

private slots:
    void onSendClicked();
    void onClearClicked();
    void onSkillClicked();
    void onRequestStarted();
    void onRequestFinished(const QString &insights);
    void onRequestFailed(const QString &errorMessage);
    void onInputTextChanged();
    void onSuggestionClicked(const QUrl &link);
    void onLoadingTick();
    void onHistoryButtonClicked();

private:
    struct ChatMessage {
        QString role;
        QString text;
        QString intent;
    };
    struct ConversationSnapshot {
        QString title;
        QString fileName;
        QVector<ChatMessage> messages;
        QString lastPrompt;
    };

    void appendMessage(const QString &role, const QString &text);
    void refreshTranscript();
    void updateContextLabel();
    void updateStateVisibility();
    void updateSendButtonStyle();
    QString detectIntent(const QString &promptText) const;
    QString renderThinkingCard(const QString &intent, bool completed) const;
    QString renderAssistantBody(const QString &text, const QString &intent) const;
    QString renderPreviewCard(const QString &intent) const;
    QString renderSuggestionChips(const QString &intent, const QString &assistantText) const;
    QStringList buildDynamicSuggestions(const QString &intent, const QString &assistantText) const;
    QStringList buildDynamicThinkingSteps(const QString &intent) const;
    QString buildDynamicNarrative(const QString &intent) const;
    QString buildContextualPrompt(const QString &intent) const;
    QString composePrompt(const QString &userText) const;
    void sendPrompt(const QString &promptText);
    bool eventFilter(QObject *watched, QEvent *event) override;

    void appendHtmlFragment(const QString &html);
    void scrollToBottom();
    void startLoadingAnimation();
    void stopLoadingAnimation();
    void archiveCurrentConversation();
    QString buildConversationTitle(const QVector<ChatMessage> &messages) const;
    void restoreConversation(int historyIndex);

    QLabel *m_contextLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QWidget *m_emptyStateWidget = nullptr;
    QTextBrowser *m_chatView = nullptr;
    QPlainTextEdit *m_inputEdit = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_skillButton = nullptr;
    QPushButton *m_deepResearchButton = nullptr;
    QToolButton *m_historyButton = nullptr;

    AIInsightService *m_service = nullptr;

    QTimer *m_loadingTimer = nullptr;
    int m_loadingFrame = 0;

    QSharedPointer<Core::TableData> m_currentData;
    bool m_filteredView = false;
    QString m_currentFileName;
    QVector<ChatMessage> m_messages;
    QString m_pendingIntent;
    QString m_lastSentPrompt;

    QStringList m_inputHistory;
    int m_historyIndex = -1;
    QVector<ConversationSnapshot> m_conversationHistory;
};

#endif // AIASSISTANTPANEL_H

#include "AIAssistantPanel.h"

#include "../ai/AIInsightService.h"
#include "../core/TableData.h"

#include <QFrame>
#include <QBuffer>
#include <QColor>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QDateTime>
#include <QGuiApplication>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QKeyEvent>
#include <QShortcut>
#include <QStyle>
#include <QStringList>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QtMath>

namespace {

QString escapeHtml(QString text)
{
    text = text.toHtmlEscaped();
    text.replace('\n', QStringLiteral("<br>"));
    return text;
}

QString markdownToHtmlFragment(const QString &markdown)
{
    QTextDocument document;
    document.setMarkdown(markdown, QTextDocument::MarkdownDialectGitHub);

    QString html = document.toHtml();
    const int bodyStart = html.indexOf(QStringLiteral("<body"));
    if (bodyStart < 0) {
        return html;
    }

    const int contentStart = html.indexOf('>', bodyStart);
    const int bodyEnd = html.lastIndexOf(QStringLiteral("</body>"));
    if (contentStart < 0 || bodyEnd < 0 || bodyEnd <= contentStart) {
        return html;
    }

    html = html.mid(contentStart + 1, bodyEnd - contentStart - 1).trimmed();

    // 去掉 QTextDocument 默认的段落边距，方便嵌入侧栏卡片
    html.replace(QStringLiteral("margin-top:12px; margin-bottom:12px;"),
                 QStringLiteral("margin-top:0; margin-bottom:8px;"));
    html.replace(QStringLiteral("margin-top:0px; margin-bottom:0px;"),
                 QStringLiteral("margin-top:0; margin-bottom:8px;"));
    html.replace(QRegularExpression(QStringLiteral("<h1[^>]*>")),
                 QStringLiteral("<h1 style='font-size:19px; font-weight:800; color:#111827; margin:6px 0 10px 0;'>"));
    html.replace(QRegularExpression(QStringLiteral("<h2[^>]*>")),
                 QStringLiteral("<h2 style='font-size:16px; font-weight:800; color:#111827; margin:14px 0 8px 0;'>"));
    html.replace(QRegularExpression(QStringLiteral("<h3[^>]*>")),
                 QStringLiteral("<h3 style='font-size:14px; font-weight:700; color:#1f2937; margin:12px 0 6px 0;'>"));
    html.replace(QRegularExpression(QStringLiteral("<p[^>]*>")),
                 QStringLiteral("<p style='margin:0 0 6px 0; color:#374151; line-height:1.78;'>"));
    html.replace(QRegularExpression(QStringLiteral("<ul[^>]*>")),
                 QStringLiteral("<ul style='margin:2px 0 8px 0; padding:0 0 0 16px; color:#374151; line-height:1.75;'>"));
    html.replace(QRegularExpression(QStringLiteral("<ol[^>]*>")),
                 QStringLiteral("<ol style='margin:2px 0 8px 0; padding:0 0 0 18px; color:#374151; line-height:1.75;'>"));
    html.replace(QRegularExpression(QStringLiteral("<li[^>]*>")),
                 QStringLiteral("<li style='margin:0 0 4px 0; padding-left:0;'>"));
    html.replace(QRegularExpression(QStringLiteral("<blockquote[^>]*>")),
                 QStringLiteral("<blockquote style='margin:8px 0; padding:8px 12px; border-left:3px solid #d0d5dd; background:#f8fafc; color:#475467;'>"));
    html.replace(QRegularExpression(QStringLiteral("<pre[^>]*>")),
                 QStringLiteral("<pre style='margin:8px 0 10px 0; padding:10px 12px; background:#0f172a; color:#e5e7eb; border-radius:10px; overflow:auto; font-size:12px; line-height:1.65;'>"));
    html.replace(QRegularExpression(QStringLiteral("<code[^>]*>")),
                 QStringLiteral("<code style='background:#f3f4f6; color:#b42318; border-radius:6px; padding:1px 4px; font-size:12px;'>"));
    html.replace(QRegularExpression(QStringLiteral("<table[^>]*>")),
                 QStringLiteral("<table style='width:100%; border-collapse:collapse; margin:8px 0 12px 0;'>"));
    html.replace(QRegularExpression(QStringLiteral("<th[^>]*>")),
                 QStringLiteral("<th style='padding:8px 10px; background:#f8fafc; border:1px solid #eaecf0; text-align:left; color:#344054; font-weight:700; font-size:12px;'>"));
    html.replace(QRegularExpression(QStringLiteral("<td[^>]*>")),
                 QStringLiteral("<td style='padding:8px 10px; border:1px solid #eaecf0; color:#374151; font-size:12px; vertical-align:top;'>"));
    return html;
}

QString buildUserBubbleHtml(const QString &text)
{
    return QStringLiteral(
               "<div style='margin:8px 0 18px 0; text-align:right;'>"
               "<div style='font-size:11px; color:#bcc1cb; margin:0 4px 5px 0;'>你</div>"
               "<div style='display:inline-block; max-width:72%; background:#efede8; color:#4b5563; "
               "border:1px solid #ece7df; padding:10px 15px; border-radius:18px 18px 8px 18px; "
               "font-size:13px; font-weight:700; line-height:1.55; text-align:left;"
               "box-shadow:0 1px 2px #00000008;'>%1</div>"
               "</div>")
        .arg(escapeHtml(text));
}

QString buildAssistantCardHtml(const QString &text, const QString &intent)
{
    const QString body = markdownToHtmlFragment(text);

    const QString intentHint = (intent == QStringLiteral("chart"))
        ? QStringLiteral("已结合当前表格的字段结构和数值特征，整理出更适合展示的图表方向。")
        : QStringLiteral("已围绕当前表格的数据结构、关键字段、分布情况和异常点完成解读。");

    return QStringLiteral(
               "<div style='margin:0 0 12px 0;'>"
               "<div style='font-size:22px; font-weight:800; color:#111827; margin-bottom:10px;'>分析结果</div>"
               "<div style='color:#667085; font-size:12px; line-height:1.75; margin-bottom:10px;'>%2</div>"
               "<div style='color:#1f2937; font-size:13px; line-height:1.88;'>%1</div>"
               "</div>")
        .arg(body, intentHint);
}

QString buildSystemHintHtml(const QString &text, bool hasRetry = false)
{
    QString retryLink;
    if (hasRetry) {
        retryLink = QStringLiteral(" <a href='retry:' style='color:#4c7cf0; font-weight:600; font-size:12px;'>重试</a>");
    }
    return QStringLiteral(
               "<div style='margin:10px 0; text-align:center; color:#667085; font-size:12px;'>%1%2</div>")
        .arg(escapeHtml(text), retryLink);
}

QFrame *createLandingCard(QWidget *parent,
                          const QString &title,
                          const QString &description,
                          const QString &intent,
                          const QString &iconText,
                          const QString &iconColor,
                          const QString &iconBackground)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("landingCard"));
    frame->setProperty("intent", intent);
    frame->setCursor(Qt::PointingHandCursor);
    frame->setMinimumHeight(74);
    frame->setStyleSheet(QStringLiteral(
        "QFrame#landingCard { background:#ffffff; border:1px solid #efebe5; border-radius:16px; }"
        "QFrame#landingCard:hover { background:#fcfbfa; border-color:#e7e1d8; }"
        "QFrame#landingCard QLabel { background:transparent; border:none; padding:0; margin:0; }"));

    auto *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(12);

    auto *iconLabel = new QLabel(frame);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(28, 28);
    iconLabel->setTextFormat(Qt::RichText);
    iconLabel->setText(QStringLiteral("<span style='font-size:13px; font-weight:700; color:%1;'>%2</span>")
                           .arg(iconColor, escapeHtml(iconText)));
    iconLabel->setStyleSheet(QStringLiteral("QLabel { background:%1; border-radius:8px; }").arg(iconBackground));

    auto *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    auto *titleLabel = new QLabel(frame);
    titleLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size:14px; font-weight:700; color:#344054; background:transparent; border:none; }"));
    titleLabel->setText(title);

    auto *descLabel = new QLabel(frame);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size:11px; color:#8b93a1; background:transparent; border:none; line-height:1.6; }"));
    descLabel->setText(description);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);
    layout->addWidget(iconLabel, 0, Qt::AlignTop);
    layout->addLayout(textLayout, 1);

    return frame;
}

QToolButton *createHeaderButton(QWidget *parent, const QString &text, const QString &toolTip)
{
    auto *button = new QToolButton(parent);
    button->setText(text);
    button->setToolTip(toolTip);
    button->setCursor(Qt::PointingHandCursor);
    button->setAutoRaise(true);
    button->setFixedSize(24, 24);
    button->setStyleSheet(QStringLiteral(
        "QToolButton { border:none; border-radius:12px; background:transparent; color:#98a2b3; font-size:14px; }"
        "QToolButton:hover { background:#ede8e2; color:#344054; }"));
    return button;
}

static const QStringList kSpinnerFrames = {
    QStringLiteral("⠋"), QStringLiteral("⠙"), QStringLiteral("⠹"),
    QStringLiteral("⠸"), QStringLiteral("⠼"), QStringLiteral("⠴"),
    QStringLiteral("⠦"), QStringLiteral("⠧"), QStringLiteral("⠇"), QStringLiteral("⠏")
};

QString normalizeHeaderText(QString text)
{
    text = text.trimmed().toLower();
    text.remove(QLatin1Char(' '));
    text.remove(QLatin1Char('_'));
    text.remove(QLatin1Char('-'));
    return text;
}

bool containsAnyKeyword(const QString &text, const QStringList &keywords)
{
    for (const QString &keyword : keywords) {
        if (text.contains(keyword)) {
            return true;
        }
    }
    return false;
}

bool isTimeLikeHeader(const QString &header)
{
    const QString normalized = normalizeHeaderText(header);
    return containsAnyKeyword(normalized,
                              {QStringLiteral("时间"), QStringLiteral("日期"), QStringLiteral("年份"),
                               QStringLiteral("年度"), QStringLiteral("季度"), QStringLiteral("月份"),
                               QStringLiteral("月"), QStringLiteral("周"), QStringLiteral("q1"),
                               QStringLiteral("q2"), QStringLiteral("q3"), QStringLiteral("q4"),
                               QStringLiteral("第一季度"), QStringLiteral("第二季度"),
                               QStringLiteral("第三季度"), QStringLiteral("第四季度")});
}

bool isMetricLikeHeader(const QString &header)
{
    const QString normalized = normalizeHeaderText(header);
    return containsAnyKeyword(normalized,
                              {QStringLiteral("总"), QStringLiteral("合计"), QStringLiteral("销售额"),
                               QStringLiteral("销量"), QStringLiteral("金额"), QStringLiteral("收入"),
                               QStringLiteral("利润"), QStringLiteral("成本"), QStringLiteral("数量"),
                               QStringLiteral("次数"), QStringLiteral("得分"), QStringLiteral("成绩"),
                               QStringLiteral("均值"), QStringLiteral("平均")});
}

QString commonTimeWord(const QStringList &headers)
{
    for (const QString &header : headers) {
        const QString normalized = normalizeHeaderText(header);
        if (containsAnyKeyword(normalized, {QStringLiteral("季度"), QStringLiteral("q1"), QStringLiteral("q2"),
                                            QStringLiteral("q3"), QStringLiteral("q4"),
                                            QStringLiteral("第一季度"), QStringLiteral("第二季度"),
                                            QStringLiteral("第三季度"), QStringLiteral("第四季度")})) {
            return QStringLiteral("季度");
        }
        if (containsAnyKeyword(normalized, {QStringLiteral("月份"), QStringLiteral("月")})) {
            return QStringLiteral("月份");
        }
        if (containsAnyKeyword(normalized, {QStringLiteral("年份"), QStringLiteral("年度"), QStringLiteral("年")})) {
            return QStringLiteral("年度");
        }
        if (containsAnyKeyword(normalized, {QStringLiteral("日期"), QStringLiteral("时间")})) {
            return QStringLiteral("时间");
        }
    }

    return QString();
}

QString formatPreviewValue(double value)
{
    if (qFuzzyCompare(value, qRound64(value))) {
        return QLocale::system().toString(qRound64(value));
    }

    return QLocale::system().toString(value, 'f', 1);
}

QString elidePreviewLabel(const QString &label, const QFont &font, int maxPixelWidth)
{
    const QString simplified = label.simplified();
    const QFontMetrics metrics(font);
    return metrics.elidedText(simplified, Qt::ElideRight, maxPixelWidth);
}

QString previewChartDataUrl(const QVector<QPair<QString, double>> &items)
{
    if (items.isEmpty()) {
        return QString();
    }

    constexpr int imageWidth = 336;
    const int rowHeight = 32;
    const int topPadding = 12;
    const int bottomPadding = 10;
    const int imageHeight = topPadding + items.size() * rowHeight + bottomPadding;

    QImage image(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(QStringLiteral("#ffffff")));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(Qt::NoPen);

    const int leftLabelWidth = 82;
    const int rightValueWidth = 82;
    const int chartLeft = 92;
    const int chartRight = imageWidth - rightValueWidth - 10;
    const int chartWidth = chartRight - chartLeft;

    double maxValue = 0.0;
    for (const auto &item : items) {
        maxValue = qMax(maxValue, item.second);
    }
    if (maxValue <= 0.0) {
        maxValue = 1.0;
    }

    QFont labelFont = painter.font();
    labelFont.setPointSize(8);
    QFont valueFont = painter.font();
    valueFont.setPointSize(8);
    valueFont.setBold(false);

    painter.setPen(QPen(QColor(QStringLiteral("#eef1f5")), 1));
    painter.drawLine(chartLeft, topPadding - 2, chartLeft, imageHeight - bottomPadding + 2);
    painter.drawLine(chartLeft + chartWidth / 2, topPadding - 2, chartLeft + chartWidth / 2, imageHeight - bottomPadding + 2);
    painter.drawLine(chartRight, topPadding - 2, chartRight, imageHeight - bottomPadding + 2);

    for (int index = 0; index < items.size(); ++index) {
        const int top = topPadding + index * rowHeight;
        const int barTop = top + 11;
        const int barHeight = 8;
        const double value = items[index].second;
        const int barWidth = qMax(12, static_cast<int>(qRound((value / maxValue) * chartWidth)));

        painter.setPen(QColor(QStringLiteral("#667085")));
        painter.setFont(labelFont);
        const QString label = elidePreviewLabel(items[index].first, labelFont, leftLabelWidth - 6);
        painter.drawText(QRect(0, top, leftLabelWidth, rowHeight),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         label);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(QStringLiteral("#eef2f6")));
        painter.drawRoundedRect(QRectF(chartLeft, barTop, chartWidth, barHeight), 4, 4);

        painter.setBrush(index == 0 ? QColor(QStringLiteral("#d8b06f"))
                                    : QColor(QStringLiteral("#89a0f7")));
        painter.drawRoundedRect(QRectF(chartLeft, barTop, barWidth, barHeight), 4, 4);

        painter.setPen(QColor(QStringLiteral("#667085")));
        painter.setFont(valueFont);
        painter.drawText(QRect(chartRight + 10, top, rightValueWidth - 10, rowHeight),
                         Qt::AlignVCenter | Qt::AlignRight,
                         formatPreviewValue(value));
    }

    painter.end();

    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    return QStringLiteral("data:image/png;base64,%1")
        .arg(QString::fromLatin1(pngBytes.toBase64()));
}

} // namespace

AIAssistantPanel::AIAssistantPanel(QWidget *parent)
    : QWidget(parent)
    , m_service(new AIInsightService(this))
    , m_loadingTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("aiAssistantPanel"));
    setMinimumWidth(320);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 10, 16, 14);
    mainLayout->setSpacing(10);

    // === 头部 ===
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    m_historyButton = createHeaderButton(this, QStringLiteral("☰"), QStringLiteral("历史对话"));
    connect(m_historyButton, &QToolButton::clicked, this, &AIAssistantPanel::onHistoryButtonClicked);
    headerLayout->addWidget(m_historyButton, 0, Qt::AlignTop);

    auto *brandLayout = new QVBoxLayout();
    brandLayout->setContentsMargins(0, 0, 0, 0);
    brandLayout->setSpacing(1);
    auto *brandLabel = new QLabel(QStringLiteral("WPS 灵思"), this);
    brandLabel->setObjectName(QStringLiteral("assistantBrand"));
    auto *subBrandLabel = new QLabel(QStringLiteral("内嵌 AI 生成"), this);
    subBrandLabel->setObjectName(QStringLiteral("assistantSubBrand"));
    brandLayout->addWidget(brandLabel);
    brandLayout->addWidget(subBrandLabel);
    headerLayout->addLayout(brandLayout);
    headerLayout->addStretch();

    auto *clearHeaderButton = createHeaderButton(this, QStringLiteral("↺"), QStringLiteral("清空对话"));
    auto *skillHeaderButton = createHeaderButton(this, QStringLiteral("⋯"), QStringLiteral("技能"));
    auto *focusHeaderButton = createHeaderButton(this, QStringLiteral("◎"), QStringLiteral("定位到输入框"));
    headerLayout->addWidget(clearHeaderButton);
    headerLayout->addWidget(skillHeaderButton);
    headerLayout->addWidget(focusHeaderButton);
    mainLayout->addLayout(headerLayout);

    // === 上下文标签 ===
    m_contextLabel = new QLabel(this);
    m_contextLabel->setObjectName(QStringLiteral("assistantContext"));
    m_contextLabel->setWordWrap(true);
    mainLayout->addWidget(m_contextLabel);

    // === 空状态（落地卡片） ===
    m_emptyStateWidget = new QWidget(this);
    auto *emptyLayout = new QVBoxLayout(m_emptyStateWidget);
    emptyLayout->setContentsMargins(0, 56, 0, 0);
    emptyLayout->setSpacing(16);

    auto *welcomeLabel = new QLabel(QStringLiteral("HI，今天有什么可以帮忙？"), m_emptyStateWidget);
    welcomeLabel->setObjectName(QStringLiteral("assistantWelcome"));
    welcomeLabel->setWordWrap(true);
    emptyLayout->addWidget(welcomeLabel);

    auto *analysisCard = createLandingCard(
        m_emptyStateWidget,
        QStringLiteral("数据分析"),
        QStringLiteral("智能解读表格数据，快速提炼关键信息"),
        QStringLiteral("analysis"),
        QStringLiteral("▣"),
        QStringLiteral("#65b84d"),
        QStringLiteral("#eef9e8"));

    auto *chartCard = createLandingCard(
        m_emptyStateWidget,
        QStringLiteral("生成图表"),
        QStringLiteral("基于文档数据生成有业务价值的图表"),
        QStringLiteral("chart"),
        QStringLiteral("▤"),
        QStringLiteral("#4d7cf0"),
        QStringLiteral("#ebf1ff"));

    analysisCard->installEventFilter(this);
    chartCard->installEventFilter(this);

    emptyLayout->addWidget(analysisCard);
    emptyLayout->addWidget(chartCard);
    emptyLayout->addStretch();
    mainLayout->addWidget(m_emptyStateWidget, 1);

    // === 聊天记录 ===
    m_chatView = new QTextBrowser(this);
    m_chatView->setObjectName(QStringLiteral("assistantTranscript"));
    m_chatView->setOpenLinks(false);
    m_chatView->setOpenExternalLinks(false);
    connect(m_chatView, &QTextBrowser::anchorClicked, this, &AIAssistantPanel::onSuggestionClicked);
    mainLayout->addWidget(m_chatView, 1);

    // === 输入区域 ===
    auto *composerShell = new QWidget(this);
    composerShell->setObjectName(QStringLiteral("composerShell"));
    auto *composerLayout = new QVBoxLayout(composerShell);
    composerLayout->setContentsMargins(14, 12, 14, 10);
    composerLayout->setSpacing(8);

    m_statusLabel = new QLabel(composerShell);
    m_statusLabel->setObjectName(QStringLiteral("assistantStatus"));
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    composerLayout->addWidget(m_statusLabel);

    m_inputEdit = new QPlainTextEdit(composerShell);
    m_inputEdit->setObjectName(QStringLiteral("assistantComposer"));
    m_inputEdit->setPlaceholderText(QStringLiteral("输入要分析的方向或选择技能"));
    m_inputEdit->setMaximumBlockCount(20);
    m_inputEdit->setFixedHeight(72);
    m_inputEdit->installEventFilter(this);
    composerLayout->addWidget(m_inputEdit);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(8);

    m_clearButton = new QPushButton(QStringLiteral("＋"), composerShell);
    m_clearButton->setObjectName(QStringLiteral("assistantMiniButton"));
    m_clearButton->setToolTip(QStringLiteral("新建对话"));
    m_clearButton->setCursor(Qt::PointingHandCursor);

    m_skillButton = new QPushButton(QStringLiteral("技能"), composerShell);
    m_skillButton->setObjectName(QStringLiteral("assistantSkillButton"));
    m_skillButton->setCursor(Qt::PointingHandCursor);

    m_deepResearchButton = new QPushButton(QStringLiteral("⚡"), composerShell);
    m_deepResearchButton->setObjectName(QStringLiteral("assistantModeButton"));
    m_deepResearchButton->setToolTip(QStringLiteral("快速解读"));
    m_deepResearchButton->setCursor(Qt::PointingHandCursor);

    m_sendButton = new QPushButton(QStringLiteral("➜"), composerShell);
    m_sendButton->setObjectName(QStringLiteral("assistantSendButton"));
    m_sendButton->setToolTip(QStringLiteral("发送"));
    m_sendButton->setCursor(Qt::PointingHandCursor);

    bottomLayout->addWidget(m_clearButton);
    bottomLayout->addWidget(m_skillButton);
    bottomLayout->addWidget(m_deepResearchButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_sendButton);
    composerLayout->addLayout(bottomLayout);
    mainLayout->addWidget(composerShell);

    // === 信号连接 ===
    connect(m_clearButton, &QPushButton::clicked, this, &AIAssistantPanel::onClearClicked);
    connect(m_skillButton, &QPushButton::clicked, this, &AIAssistantPanel::onSkillClicked);
    connect(clearHeaderButton, &QToolButton::clicked, this, &AIAssistantPanel::onClearClicked);
    connect(skillHeaderButton, &QToolButton::clicked, this, &AIAssistantPanel::onSkillClicked);
    connect(focusHeaderButton, &QToolButton::clicked, this, &AIAssistantPanel::focusComposer);
    connect(m_deepResearchButton, &QPushButton::clicked, this, [this]() {
        sendPrompt(buildContextualPrompt(QStringLiteral("analysis")));
    });
    connect(m_sendButton, &QPushButton::clicked, this, &AIAssistantPanel::onSendClicked);
    connect(m_inputEdit, &QPlainTextEdit::textChanged, this, &AIAssistantPanel::onInputTextChanged);

    connect(m_service, &AIInsightService::requestStarted,
            this, &AIAssistantPanel::onRequestStarted);
    connect(m_service, &AIInsightService::requestFinished,
            this, &AIAssistantPanel::onRequestFinished);
    connect(m_service, &AIInsightService::requestFailed,
            this, &AIAssistantPanel::onRequestFailed);

    // 加载动画计时器
    m_loadingTimer->setInterval(80);
    connect(m_loadingTimer, &QTimer::timeout, this, &AIAssistantPanel::onLoadingTick);

    // === 样式表 ===
    setStyleSheet(QStringLiteral(
        "#aiAssistantPanel { background:#f6f4f1; }"
        "QLabel#assistantBrand { font-size:18px; font-weight:800; color:#111827; }"
        "QLabel#assistantSubBrand { font-size:11px; color:#b3b8c3; }"
        "QLabel#assistantContext { font-size:12px; color:#9098a6; padding-left:30px; margin-top:2px; }"
        "QLabel#assistantWelcome { font-size:20px; font-weight:800; color:#101828; margin:8px 0 0 2px; }"
        "QTextBrowser#assistantTranscript { background:transparent; border:none; padding:0; }"
        "QTextBrowser#assistantTranscript QScrollBar:vertical { background:transparent; width:6px; margin:4px 0; }"
        "QTextBrowser#assistantTranscript QScrollBar::handle:vertical { background:#d1d5db; border-radius:3px; min-height:20px; }"
        "QTextBrowser#assistantTranscript QScrollBar::add-line:vertical, "
        "QTextBrowser#assistantTranscript QScrollBar::sub-line:vertical { height:0; }"
        "QTextBrowser#assistantTranscript QScrollBar::add-page:vertical, "
        "QTextBrowser#assistantTranscript QScrollBar::sub-page:vertical { background:none; }"
        "QWidget#composerShell { background:#ffffff; border:1px solid #ebe7e2; border-radius:20px; "
        "box-shadow:0 1px 4px #0000000a; }"
        "QLabel#assistantStatus { font-size:12px; color:#98a2b3; }"
        "QPlainTextEdit#assistantComposer { background:transparent; border:none; color:#101828; font-size:13px; padding:0; selection-background-color:#dbe7ff; }"
        "QPushButton#assistantMiniButton { min-width:26px; max-width:26px; min-height:26px; max-height:26px; "
        "border:none; background:transparent; color:#667085; font-size:20px; font-weight:300; }"
        "QPushButton#assistantMiniButton:hover { color:#101828; }"
        "QPushButton#assistantSkillButton { min-width:46px; min-height:28px; border:none; background:transparent; "
        "color:#667085; font-size:13px; font-weight:700; padding:0 2px; text-align:left; }"
        "QPushButton#assistantSkillButton:hover { color:#101828; }"
        "QPushButton#assistantModeButton { min-width:30px; max-width:30px; min-height:30px; max-height:30px; "
        "border:none; border-radius:15px; background:#e8efff; color:#5b7ef5; font-size:14px; font-weight:700; }"
        "QPushButton#assistantModeButton:hover { background:#dce6ff; }"
        "QPushButton#assistantSendButton { min-width:32px; max-width:32px; min-height:32px; max-height:32px; "
        "border:none; border-radius:14px; background:#e5e7eb; color:#9ca3af; font-size:13px; font-weight:700; }"
        "QPushButton#assistantSendButton:hover { background:#d1d5db; color:#6b7280; }"
        "QPushButton#assistantSendButton[active=\"true\"] { background:#4c7cf0; color:#ffffff; }"
        "QPushButton#assistantSendButton[active=\"true\"]:hover { background:#3b6de0; }"
        "QPushButton#assistantSendButton:disabled { background:#e5e7eb; color:#c0c4cc; }"));

    updateContextLabel();
    refreshTranscript();
}

// ============================================================
// Context
// ============================================================

void AIAssistantPanel::setContext(const QSharedPointer<Core::TableData> &data,
                                  bool filteredView,
                                  const QString &fileName)
{
    const bool fileChanged = m_currentFileName != fileName;
    if (fileChanged && !m_messages.isEmpty() && !m_service->isBusy()) {
        archiveCurrentConversation();
        m_messages.clear();
        m_pendingIntent.clear();
        m_lastSentPrompt.clear();
        m_statusLabel->hide();
        refreshTranscript();
    }

    m_currentData = data;
    m_filteredView = filteredView;
    m_currentFileName = fileName;
    updateContextLabel();
    updateSendButtonStyle();
}

void AIAssistantPanel::focusComposer()
{
    m_inputEdit->setFocus();
}

// ============================================================
// Slots
// ============================================================

void AIAssistantPanel::onSendClicked()
{
    sendPrompt(m_inputEdit->toPlainText());
}

void AIAssistantPanel::onClearClicked()
{
    if (!m_messages.isEmpty()) {
        archiveCurrentConversation();
    }
    m_messages.clear();
    m_statusLabel->hide();
    m_inputHistory.clear();
    m_historyIndex = -1;
    m_lastSentPrompt.clear();
    refreshTranscript();
    updateSendButtonStyle();
}

void AIAssistantPanel::onSkillClicked()
{
    QMenu menu(this);

    auto *analysisAction = menu.addAction(QStringLiteral("📊  数据分析"));
    analysisAction->setToolTip(QStringLiteral("对表格做整体数据分析，提炼关键信息"));

    auto *chartAction = menu.addAction(QStringLiteral("📈  生成图表"));
    chartAction->setToolTip(QStringLiteral("推荐可视化图表，展示字段关系"));

    QAction *selected = menu.exec(m_skillButton->mapToGlobal(QPoint(0, m_skillButton->height() + 6)));
    if (!selected) {
        return;
    }

    const QString intent = (selected == chartAction) ? QStringLiteral("chart") : QStringLiteral("analysis");
    sendPrompt(buildContextualPrompt(intent));
}

void AIAssistantPanel::onRequestStarted()
{
    m_statusLabel->show();
    startLoadingAnimation();
    m_sendButton->setEnabled(false);
}

void AIAssistantPanel::onRequestFinished(const QString &insights)
{
    stopLoadingAnimation();
    m_statusLabel->hide();
    m_messages.append(ChatMessage{QStringLiteral("assistant"), insights.trimmed(), m_pendingIntent});
    m_pendingIntent.clear();
    // 助手消息需要全量重建（建议芯片只显示在最后一条）
    refreshTranscript();
    updateSendButtonStyle();
}

void AIAssistantPanel::onRequestFailed(const QString &errorMessage)
{
    stopLoadingAnimation();
    m_pendingIntent.clear();
    // 带重试链接的系统提示
    const QString hintHtml = buildSystemHintHtml(errorMessage, true);
    m_messages.append(ChatMessage{QStringLiteral("system"), errorMessage, {}});
    appendHtmlFragment(hintHtml);
    updateStateVisibility();
    updateContextLabel();
    m_statusLabel->setText(QStringLiteral("请求失败"));
    m_statusLabel->show();
    updateSendButtonStyle();
}

void AIAssistantPanel::onInputTextChanged()
{
    updateSendButtonStyle();
}

void AIAssistantPanel::onSuggestionClicked(const QUrl &link)
{
    if (link.scheme() == QStringLiteral("suggest")) {
        sendPrompt(link.path().mid(1)); // skip leading '/'
    } else if (link.scheme() == QStringLiteral("retry")) {
        if (!m_lastSentPrompt.isEmpty()) {
            sendPrompt(m_lastSentPrompt);
        }
    }
}

void AIAssistantPanel::onLoadingTick()
{
    m_loadingFrame = (m_loadingFrame + 1) % kSpinnerFrames.size();
    m_statusLabel->setText(kSpinnerFrames[m_loadingFrame] + QStringLiteral(" 正在分析..."));
}

// ============================================================
// Event filter
// ============================================================

bool AIAssistantPanel::eventFilter(QObject *watched, QEvent *event)
{
    // 输入框键盘事件
    if (watched == m_inputEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();

        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                return false; // Shift+Enter 换行
            }
            onSendClicked();
            return true;
        }

        // 上/下方向键翻阅输入历史
        if (key == Qt::Key_Up && !m_inputHistory.isEmpty()) {
            if (m_historyIndex < 0) {
                m_historyIndex = m_inputHistory.size() - 1;
            } else if (m_historyIndex > 0) {
                --m_historyIndex;
            }
            m_inputEdit->setPlainText(m_inputHistory[m_historyIndex]);
            m_inputEdit->moveCursor(QTextCursor::End);
            return true;
        }
        if (key == Qt::Key_Down && !m_inputHistory.isEmpty()) {
            if (m_historyIndex >= 0 && m_historyIndex < m_inputHistory.size() - 1) {
                ++m_historyIndex;
                m_inputEdit->setPlainText(m_inputHistory[m_historyIndex]);
                m_inputEdit->moveCursor(QTextCursor::End);
            } else if (m_historyIndex >= 0) {
                m_historyIndex = -1;
                m_inputEdit->clear();
            }
            return true;
        }
    }

    // 落地卡片点击 — 根据实际列名构建 prompt
    if (event->type() == QEvent::MouseButtonPress) {
        auto *frame = qobject_cast<QFrame *>(watched);
        if (frame) {
            if (frame->property("intent").isValid()) {
                const QString intent = frame->property("intent").toString();
                if (!intent.isEmpty()) {
                    sendPrompt(buildContextualPrompt(intent));
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

// ============================================================
// Send button style
// ============================================================

void AIAssistantPanel::updateSendButtonStyle()
{
    const bool hasText = !m_inputEdit->toPlainText().trimmed().isEmpty();
    const bool isActive = hasText && !m_service->isBusy();

    m_sendButton->setEnabled(!m_service->isBusy() && hasText);
    m_sendButton->setProperty("active", isActive);
    m_sendButton->style()->unpolish(m_sendButton);
    m_sendButton->style()->polish(m_sendButton);
    m_sendButton->update();
}

// ============================================================
// Message handling
// ============================================================

void AIAssistantPanel::appendMessage(const QString &role, const QString &text)
{
    const QString intent = (role == QStringLiteral("user")) ? detectIntent(text) : QString();
    m_messages.append(ChatMessage{role, text.trimmed(), intent});
    updateStateVisibility();
    updateContextLabel();

    // 用户或系统消息会改变助手建议区的显示条件，因此直接重建消息流。
    // assistant 消息仍在 onRequestFinished 中统一刷新。
    if (role != QStringLiteral("assistant")) {
        refreshTranscript();
    }
}

void AIAssistantPanel::appendHtmlFragment(const QString &html)
{
    if (m_chatView->document()->isEmpty()) {
        m_chatView->setHtml(QStringLiteral(
            "<div style='font-family:\"Microsoft YaHei UI\"; font-size:13px;'>%1</div>").arg(html));
    } else {
        QTextCursor cursor(m_chatView->document());
        cursor.movePosition(QTextCursor::End);
        cursor.insertHtml(html);
    }
    scrollToBottom();
}

void AIAssistantPanel::scrollToBottom()
{
    QScrollBar *bar = m_chatView->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void AIAssistantPanel::refreshTranscript()
{
    updateStateVisibility();
    if (m_messages.isEmpty()) {
        m_chatView->clear();
        return;
    }

    QString html = QStringLiteral("<div style='font-family:\"Microsoft YaHei UI\"; font-size:13px;'>");
    int lastAssistant = -1;
    for (int i = m_messages.size() - 1; i >= 0; --i) {
        if (m_messages[i].role == QStringLiteral("assistant")) {
            lastAssistant = i;
            break;
        }
    }
    const bool latestIsAssistant =
        !m_messages.isEmpty() && m_messages.last().role == QStringLiteral("assistant");
    for (int i = 0; i < m_messages.size(); ++i) {
        const ChatMessage &message = m_messages[i];
        if (message.role == QStringLiteral("user")) {
            html += buildUserBubbleHtml(message.text);
        } else if (message.role == QStringLiteral("assistant")) {
            if (message.intent != QStringLiteral("general")) {
                html += renderThinkingCard(message.intent, true);
                html += renderPreviewCard(message.intent);
            }
            html += renderAssistantBody(message.text, message.intent);
            // 建议芯片只在最后一条助手消息显示
            if (latestIsAssistant && i == lastAssistant) {
                html += renderSuggestionChips(message.intent, message.text);
            }
        } else {
            html += buildSystemHintHtml(message.text);
        }
    }
    html += QStringLiteral("</div>");

    m_chatView->setHtml(html);
    scrollToBottom();
}

// ============================================================
// Context label & state visibility
// ============================================================

void AIAssistantPanel::updateContextLabel()
{
    if (m_messages.isEmpty()) {
        m_contextLabel->hide();
        return;
    }

    if (m_currentData.isNull() || m_currentData->isEmpty()) {
        m_contextLabel->setText(QStringLiteral("当前未连接表格"));
    } else {
        const QString fileText = m_currentFileName.isEmpty()
                                     ? QStringLiteral("未命名数据")
                                     : m_currentFileName;
        m_contextLabel->setTextFormat(Qt::RichText);
        m_contextLabel->setText(
            QStringLiteral("<span style='color:#7b86a2;'>%1</span>"
                           "<span style='color:#c7ccd6;'> | </span>"
                           "<span style='color:#8d96a8;'>%2</span>"
                           "<span style='color:#c7ccd6;'> | </span>"
                           "<span style='color:#8d96a8;'>%3 行 × %4 列</span>")
                .arg(fileText.toHtmlEscaped(),
                     (m_filteredView ? QStringLiteral("筛选数据") : QStringLiteral("完整数据")).toHtmlEscaped(),
                     QString::number(m_currentData->rowCount()),
                     QString::number(m_currentData->columnCount())));
    }

    m_contextLabel->show();
}

void AIAssistantPanel::updateStateVisibility()
{
    const bool empty = m_messages.isEmpty();
    m_emptyStateWidget->setVisible(empty);
    m_chatView->setVisible(!empty);
    if (empty) {
        m_contextLabel->hide();
    }
}

// ============================================================
// Prompt composition
// ============================================================

QString AIAssistantPanel::composePrompt(const QString &userText) const
{
    // API 无状态，不需要传完整对话历史。
    // 只保留上一轮问题作为简短上下文。
    QString context;
    for (int i = m_messages.size() - 2; i >= qMax(0, m_messages.size() - 6); --i) {
        const ChatMessage &msg = m_messages[i];
        if (msg.role == QStringLiteral("user")) {
            context = QStringLiteral("（上一次提问：%1）\n").arg(msg.text);
            break;
        }
    }

    return context + userText;
}

void AIAssistantPanel::sendPrompt(const QString &promptText)
{
    const QString trimmedPrompt = promptText.trimmed();
    if (trimmedPrompt.isEmpty()) {
        return;
    }

    const QString intent = detectIntent(trimmedPrompt);
    const bool useDataContext = intent != QStringLiteral("general");

    if (useDataContext && (m_currentData.isNull() || m_currentData->isEmpty())) {
        appendMessage(QStringLiteral("system"), QStringLiteral("当前没有可分析的数据，请先打开文件。"));
        return;
    }

    // 记录输入历史
    m_inputHistory.append(trimmedPrompt);
    if (m_inputHistory.size() > 50) {
        m_inputHistory.removeFirst();
    }
    m_historyIndex = -1;

    m_lastSentPrompt = trimmedPrompt;
    appendMessage(QStringLiteral("user"), trimmedPrompt);
    m_inputEdit->clear();
    m_pendingIntent = intent;
    m_service->requestInsights(m_currentData, composePrompt(trimmedPrompt), m_filteredView, useDataContext);
}

QString AIAssistantPanel::detectIntent(const QString &promptText) const
{
    const QString text = promptText.trimmed().toLower();
    const bool hasLoadedData = !m_currentData.isNull() && !m_currentData->isEmpty();

    const bool hasChartKeyword = containsAnyKeyword(text, {
        QStringLiteral("图表"), QStringLiteral("可视化"), QStringLiteral("趋势图"),
        QStringLiteral("柱状图"), QStringLiteral("折线图"), QStringLiteral("饼图"),
        QStringLiteral("散点图"), QStringLiteral("雷达图")
    });
    const bool wantsChartOutput = containsAnyKeyword(text, {
        QStringLiteral("推荐"), QStringLiteral("生成"), QStringLiteral("做"),
        QStringLiteral("画"), QStringLiteral("绘制"), QStringLiteral("展示")
    });

    const QStringList dataKeywords = {
        QStringLiteral("表格"), QStringLiteral("数据"), QStringLiteral("字段"),
        QStringLiteral("列"), QStringLiteral("行"), QStringLiteral("统计"),
        QStringLiteral("销量"), QStringLiteral("销售"), QStringLiteral("金额"),
        QStringLiteral("占比"), QStringLiteral("同比"), QStringLiteral("环比"),
        QStringLiteral("分组"), QStringLiteral("筛选"), QStringLiteral("异常值"),
        QStringLiteral("预测"), QStringLiteral("均值"), QStringLiteral("中位数"),
        QStringLiteral("标准差"), QStringLiteral("相关性"), QStringLiteral("样本"),
        QStringLiteral("报表"), QStringLiteral("工作表"), QStringLiteral("excel"),
        QStringLiteral("sheet")
    };

    const QStringList currentDataReferenceKeywords = {
        QStringLiteral("当前表格"), QStringLiteral("当前数据"), QStringLiteral("当前文件"),
        QStringLiteral("这个表"), QStringLiteral("这张表"), QStringLiteral("这份数据"),
        QStringLiteral("这些数据"), QStringLiteral("这些字段"), QStringLiteral("当前筛选"),
        QStringLiteral("筛选后"), QStringLiteral("可见数据"), QStringLiteral("这列"),
        QStringLiteral("这些列"), QStringLiteral("这行"), QStringLiteral("这些行")
    };

    const QStringList genericKeywords = {
        QStringLiteral("c++"), QStringLiteral("c#"), QStringLiteral("java"),
        QStringLiteral("python"), QStringLiteral("javascript"), QStringLiteral("rust"),
        QStringLiteral("go"), QStringLiteral("算法"), QStringLiteral("数据结构"),
        QStringLiteral("线程"), QStringLiteral("并发"), QStringLiteral("内存"),
        QStringLiteral("操作系统"), QStringLiteral("网络"), QStringLiteral("面试"),
        QStringLiteral("设计模式"), QStringLiteral("编译"), QStringLiteral("框架"),
        QStringLiteral("翻译"), QStringLiteral("润色"), QStringLiteral("改写"),
        QStringLiteral("写个"), QStringLiteral("写一段"), QStringLiteral("邮件"),
        QStringLiteral("简历"), QStringLiteral("请假"), QStringLiteral("难过"),
        QStringLiteral("焦虑"), QStringLiteral("心情"), QStringLiteral("英语"),
        QStringLiteral("日语"), QStringLiteral("韩语"), QStringLiteral("法语")
    };

    const QStringList analysisKeywords = {
        QStringLiteral("分析"), QStringLiteral("解读"), QStringLiteral("总结"),
        QStringLiteral("概述"), QStringLiteral("发现"), QStringLiteral("异常"),
        QStringLiteral("趋势"), QStringLiteral("对比"), QStringLiteral("建议"),
        QStringLiteral("洞察"), QStringLiteral("梳理"), QStringLiteral("统计")
    };

    const bool hasDataKeyword = containsAnyKeyword(text, dataKeywords);
    const bool referencesCurrentData = containsAnyKeyword(text, currentDataReferenceKeywords);
    const bool hasGenericKeyword = containsAnyKeyword(text, genericKeywords);

    if (hasChartKeyword && (referencesCurrentData || hasDataKeyword || (hasLoadedData && wantsChartOutput))) {
        return QStringLiteral("chart");
    }

    if (hasDataKeyword || (hasLoadedData && referencesCurrentData)) {
        return QStringLiteral("analysis");
    }

    if (hasGenericKeyword && !hasDataKeyword) {
        return QStringLiteral("general");
    }

    if (!hasLoadedData) {
        return QStringLiteral("general");
    }

    if (containsAnyKeyword(text, analysisKeywords)) {
        return QStringLiteral("analysis");
    }

    return QStringLiteral("general");
}

// ============================================================
// Dynamic content builders (based on actual table data)
// ============================================================

QString AIAssistantPanel::buildContextualPrompt(const QString &intent) const
{
    if (m_currentData.isNull() || m_currentData->isEmpty()) {
        return (intent == QStringLiteral("chart"))
            ? QStringLiteral("请基于当前数据推荐图表，并说明各图表适合展示哪些字段。")
            : QStringLiteral("请对当前表格做整体数据分析，概述结构、关键发现和异常点。");
    }

    QStringList colNames;
    for (int c = 0; c < m_currentData->columnCount(); ++c) {
        colNames << m_currentData->header(c).trimmed();
    }
    const QString colList = colNames.join(QStringLiteral("、"));

    if (intent == QStringLiteral("chart")) {
        return QStringLiteral("当前表格共 %1 行 × %2 列，字段包括：%3。"
                              "请基于这些数据推荐合适的图表类型，并说明各图表适合展示哪些字段和结论。")
            .arg(m_currentData->rowCount())
            .arg(m_currentData->columnCount())
            .arg(colList);
    }

    return QStringLiteral("当前表格共 %1 行 × %2 列，字段包括：%3。"
                          "请对表格做整体数据分析，概述数据结构、关键发现和异常点。")
        .arg(m_currentData->rowCount())
        .arg(m_currentData->columnCount())
        .arg(colList);
}

QString AIAssistantPanel::buildDynamicNarrative(const QString &intent) const
{
    if (m_currentData.isNull() || m_currentData->isEmpty()) {
        return (intent == QStringLiteral("chart"))
            ? QStringLiteral("正在识别字段类型与数据特征，匹配最佳可视化方案。")
            : QStringLiteral("正在分析表格数据，识别关键字段与异常点。");
    }

    if (intent == QStringLiteral("chart")) {
        return QStringLiteral("正在分析 %1 行 × %2 列的表格结构，识别可量化指标与分类维度，匹配合适的图表类型。")
            .arg(m_currentData->rowCount())
            .arg(m_currentData->columnCount());
    }

    return QStringLiteral("正在分析 %1 行 × %2 列的表格内容，识别字段结构、关键指标与数据异常点。")
        .arg(m_currentData->rowCount())
        .arg(m_currentData->columnCount());
}

QStringList AIAssistantPanel::buildDynamicThinkingSteps(const QString &intent) const
{
    QString firstNum, firstCat;
    if (!m_currentData.isNull() && !m_currentData->isEmpty()) {
        for (int c = 0; c < m_currentData->columnCount(); ++c) {
            const QString name = m_currentData->header(c).trimmed();
            if (name.isEmpty()) {
                continue;
            }
            if (firstNum.isEmpty() && m_currentData->isNumeric(c)) {
                firstNum = name;
            } else if (firstCat.isEmpty() && !m_currentData->isNumeric(c)) {
                firstCat = name;
            }
            if (!firstNum.isEmpty() && !firstCat.isEmpty()) {
                break;
            }
        }
    }

    if (intent == QStringLiteral("chart")) {
        QStringList steps;
        steps << QStringLiteral("扫描表格结构，识别分类字段与可量化指标");
        if (!firstNum.isEmpty()) {
            steps << QStringLiteral("检查%1的数据完整性和空值情况").arg(firstNum);
        } else {
            steps << QStringLiteral("检查数据完整性和空值情况");
        }
        steps << QStringLiteral("匹配合适的图表类型（柱状图、折线图、占比图等）");
        steps << QStringLiteral("提炼图表标题与重点解读结论");
        return steps;
    }

    QStringList steps;
    steps << QStringLiteral("读取表格结构（%1 行 × %2 列）")
                 .arg(m_currentData.isNull() ? 0 : m_currentData->rowCount())
                 .arg(m_currentData.isNull() ? 0 : m_currentData->columnCount());
    if (!firstNum.isEmpty()) {
        steps << QStringLiteral("分析%1的分布、均值和异常值").arg(firstNum);
    } else {
        steps << QStringLiteral("分析各字段的数据分布与统计特征");
    }
    if (!firstCat.isEmpty()) {
        steps << QStringLiteral("检查%1的分类构成与频次").arg(firstCat);
    } else {
        steps << QStringLiteral("识别数据中的关键模式和趋势");
    }
    steps << QStringLiteral("提炼核心结论并给出分析建议");
    return steps;
}

QStringList AIAssistantPanel::buildDynamicSuggestions(const QString &intent,
                                                      const QString &assistantText) const
{
    if (intent == QStringLiteral("general")) {
        QStringList suggestions;
        const QString text = assistantText.toLower();
        if (text.contains(QStringLiteral("c++")) || text.contains(QStringLiteral("编程"))) {
            suggestions << QStringLiteral("给我一个更好理解的示例");
            suggestions << QStringLiteral("总结成 3 条核心要点");
            suggestions << QStringLiteral("再讲讲常见面试追问");
        } else {
            suggestions << QStringLiteral("换一个更通俗的说法");
            suggestions << QStringLiteral("用一个实际案例解释");
            suggestions << QStringLiteral("总结成 3 个重点");
        }
        return suggestions;
    }

    if (m_currentData.isNull() || m_currentData->isEmpty()) {
        return {
            QStringLiteral("对当前表格做整体数据分析"),
            QStringLiteral("识别数据中的关键指标和异常值"),
            QStringLiteral("推荐适合当前数据的可视化图表")
        };
    }

    QString firstNumCol, secondNumCol, firstCatCol, timeCol, summaryMetricCol;
    QStringList timeLikeNumericHeaders;
    const int cols = m_currentData->columnCount();
    for (int c = 0; c < cols; ++c) {
        const QString name = m_currentData->header(c).trimmed();
        const QString colText = name.isEmpty() ? QStringLiteral("列%1").arg(c + 1) : name;
        const bool isNumeric = m_currentData->isNumeric(c);
        const bool timeLike = isTimeLikeHeader(colText);
        const bool metricLike = isMetricLikeHeader(colText);

        if (isNumeric) {
            if (firstNumCol.isEmpty()) {
                firstNumCol = colText;
            } else if (secondNumCol.isEmpty()) {
                secondNumCol = colText;
            }

            if (metricLike && summaryMetricCol.isEmpty()) {
                summaryMetricCol = colText;
            }
            if (timeLike) {
                timeLikeNumericHeaders << colText;
                if (timeCol.isEmpty()) {
                    timeCol = colText;
                }
            }
        } else {
            if (timeLike && timeCol.isEmpty()) {
                timeCol = colText;
            } else if (firstCatCol.isEmpty()) {
                firstCatCol = colText;
            }
        }
    }

    if (summaryMetricCol.isEmpty()) {
        summaryMetricCol = firstNumCol;
    }

    QStringList suggestions;
    const QString assistantLower = assistantText.toLower();
    const QString timeWord = commonTimeWord(timeLikeNumericHeaders + (timeCol.isEmpty() ? QStringList() : QStringList{timeCol}));

    auto appendSuggestion = [&suggestions](const QString &text) {
        const QString trimmed = text.trimmed();
        if (trimmed.isEmpty() || suggestions.contains(trimmed)) {
            return;
        }
        suggestions << trimmed;
    };

    if (intent == QStringLiteral("chart")) {
        if (!firstCatCol.isEmpty() && !timeWord.isEmpty() && !timeLikeNumericHeaders.isEmpty()) {
            appendSuggestion(QStringLiteral("生成各%1的%2趋势对比图").arg(firstCatCol, timeWord));
        }
        if (!firstCatCol.isEmpty() && !summaryMetricCol.isEmpty()) {
            appendSuggestion(QStringLiteral("生成各%1的%2对比柱状图").arg(firstCatCol, summaryMetricCol));
        }
        if (!summaryMetricCol.isEmpty()) {
            appendSuggestion(QStringLiteral("找出%1最高的记录并突出展示").arg(summaryMetricCol));
        }
        appendSuggestion(QStringLiteral("推荐适合当前数据的可视化图表"));
    } else {
        if (!firstCatCol.isEmpty() && !timeWord.isEmpty() && !timeLikeNumericHeaders.isEmpty()) {
            appendSuggestion(QStringLiteral("分析各%1在不同%2的变化趋势").arg(firstCatCol, timeWord));
        }
        if (!firstCatCol.isEmpty() && !summaryMetricCol.isEmpty()) {
            appendSuggestion(QStringLiteral("分析各%1的%2排名与占比").arg(firstCatCol, summaryMetricCol));
        } else if (!firstCatCol.isEmpty()) {
            appendSuggestion(QStringLiteral("分析%1的分布与频次统计").arg(firstCatCol));
        }

        if (!summaryMetricCol.isEmpty() && assistantLower.contains(QStringLiteral("增长"))) {
            if (!timeWord.isEmpty()) {
                appendSuggestion(QStringLiteral("计算各%1%2的增长率").arg(timeWord, summaryMetricCol));
            } else {
                appendSuggestion(QStringLiteral("计算%1的增长率变化").arg(summaryMetricCol));
            }
        }

        if (!summaryMetricCol.isEmpty() && assistantLower.contains(QStringLiteral("异常"))) {
            appendSuggestion(QStringLiteral("定位%1中的异常值和波动点").arg(summaryMetricCol));
        }

        if (!firstNumCol.isEmpty() && !secondNumCol.isEmpty()) {
            appendSuggestion(QStringLiteral("分析%1与%2之间的相关性").arg(firstNumCol, secondNumCol));
        }

        if (suggestions.size() < 3) {
            if (!summaryMetricCol.isEmpty()) {
                appendSuggestion(QStringLiteral("分析%1的整体分布与统计特征").arg(summaryMetricCol));
            } else {
                appendSuggestion(QStringLiteral("识别数据中的异常值和关键趋势"));
            }
        }
    }

    while (suggestions.size() > 3) {
        suggestions.removeLast();
    }

    return suggestions;
}

// ============================================================
// Renderers
// ============================================================

QString AIAssistantPanel::renderThinkingCard(const QString &intent, bool completed) const
{
    const QString title = completed ? QStringLiteral("已完成深度思考  ⌄")
                                    : QStringLiteral("正在深度思考...");
    const QString narrative = buildDynamicNarrative(intent);
    const QStringList steps = buildDynamicThinkingSteps(intent);

    QString stepsHtml;
    for (const QString &step : steps) {
        stepsHtml += QStringLiteral(
                         "<tr>"
                         "<td style='padding:4px 8px 4px 0; color:#22c55e; font-weight:700; vertical-align:top;'>✓</td>"
                         "<td style='padding:4px 0; color:#4b5563; line-height:1.7;'>%1</td>"
                         "</tr>")
                         .arg(escapeHtml(step));
    }

    return QStringLiteral(
               "<div style='margin:0 0 14px 0;'>"
               "<div style='font-size:12px; color:#9aa3b2; margin-bottom:2px;'>灵思</div>"
               "<div style='font-size:14px; font-weight:700; color:#4b5563; margin-bottom:10px;'>%1</div>"
               "<div style='color:#5f6b7b; line-height:1.85; margin-bottom:10px; font-size:12px;'>%2</div>"
               "<div style='background:#f7f7f5; border-radius:14px; padding:12px 14px;'>"
               "<table cellpadding='0' cellspacing='0' style='margin:0;'>%3</table>"
               "</div>"
               "</div>")
        .arg(title, escapeHtml(narrative), stepsHtml);
}

QString AIAssistantPanel::renderAssistantBody(const QString &text, const QString &intent) const
{
    return buildAssistantCardHtml(text, intent);
}

QString AIAssistantPanel::renderPreviewCard(const QString &intent) const
{
    if (m_currentData.isNull() || m_currentData->isEmpty()) {
        return QString();
    }

    // 寻找数值列和分类列
    int numericColumn = -1;
    int labelColumn = -1;
    QVector<double> values;

    for (int column = 0; column < m_currentData->columnCount(); ++column) {
        if (!m_currentData->isNumeric(column)) {
            continue;
        }
        QVector<double> parsed;
        int validCount = 0;
        for (int row = 0; row < m_currentData->rowCount(); ++row) {
            bool ok = false;
            const double value = m_currentData->at(row, column).toString().remove(',').toDouble(&ok);
            if (ok) {
                parsed.append(value);
                ++validCount;
            } else {
                parsed.append(qQNaN());
            }
        }

        if (validCount >= qMax(3, m_currentData->rowCount() / 2)) {
            numericColumn = column;
            values = parsed;
            break;
        }
    }

    if (numericColumn < 0) {
        return QString();
    }

    for (int column = 0; column < m_currentData->columnCount(); ++column) {
        if (column == numericColumn) {
            continue;
        }

        bool allNumeric = true;
        for (int row = 0; row < m_currentData->rowCount(); ++row) {
            bool ok = false;
            m_currentData->at(row, column).toString().remove(',').toDouble(&ok);
            if (!ok && !m_currentData->at(row, column).toString().trimmed().isEmpty()) {
                allNumeric = false;
                break;
            }
        }

        if (!allNumeric) {
            labelColumn = column;
            break;
        }
    }

    if (labelColumn < 0) {
        labelColumn = 0;
    }

    double maxValue = 0.0;
    for (double value : values) {
        if (!qIsNaN(value)) {
            maxValue = qMax(maxValue, value);
        }
    }
    if (maxValue <= 0.0) {
        return QString();
    }

    const QString numColName = m_currentData->header(numericColumn).trimmed();
    const QString chartTitle = (intent == QStringLiteral("chart"))
        ? QStringLiteral("%1 数据对比").arg(numColName.isEmpty() ? QStringLiteral("数值") : numColName)
        : QStringLiteral("数据预览图");

    QVector<QPair<QString, double>> previewItems;
    int rendered = 0;
    for (int row = 0; row < m_currentData->rowCount() && rendered < 4; ++row) {
        const double value = values.value(row, qQNaN());
        if (qIsNaN(value)) {
            continue;
        }

        const QString label = m_currentData->at(row, labelColumn).toString().trimmed().isEmpty()
                                  ? QStringLiteral("项%1").arg(row + 1)
                                  : m_currentData->at(row, labelColumn).toString().trimmed();
        previewItems.append(qMakePair(label, value));
        ++rendered;
    }

    if (previewItems.isEmpty()) {
        return QString();
    }

    const QString dataUrl = previewChartDataUrl(previewItems);
    if (dataUrl.isEmpty()) {
        return QString();
    }

    return QStringLiteral(
               "<div style='margin:0 0 14px 0;'>"
               "<div style='background:#ffffff; border:1px solid #ebe7e2; border-radius:16px; padding:12px 12px 10px 12px;'>"
               "<div style='font-size:12px; font-weight:700; color:#4b5563; margin-bottom:8px;'>%1</div>"
               "<img src='%2' style='display:block; width:100%%; border:none; border-radius:10px; background:#fbfbfa;' />"
               "</div>"
               "</div>")
        .arg(escapeHtml(chartTitle), dataUrl);
}

QString AIAssistantPanel::renderSuggestionChips(const QString &intent,
                                                const QString &assistantText) const
{
    const QStringList suggestions = buildDynamicSuggestions(intent, assistantText);
    if (suggestions.isEmpty()) {
        return QString();
    }

    QString chipsHtml;
    for (const QString &item : suggestions) {
        const QString encoded = QString::fromUtf8(item.toUtf8().toPercentEncoding());
        chipsHtml += QStringLiteral(
                         "<div style='margin:0 0 8px 0;'>"
                         "<a href='suggest:/%1' style='text-decoration:none;'>"
                         "<span style='display:inline-block; background:#ece9e4; color:#4b5563; "
                         "padding:8px 14px; border-radius:999px; font-size:12px; font-weight:600;'>%2  ›</span>"
                         "</a>"
                         "</div>")
                         .arg(encoded, escapeHtml(item));
    }

    return QStringLiteral(
               "<div style='margin:6px 0 26px 0;'>"
               "%1"
               "</div>")
        .arg(chipsHtml);
}

// ============================================================
// Loading animation
// ============================================================

void AIAssistantPanel::startLoadingAnimation()
{
    m_loadingFrame = 0;
    m_statusLabel->setText(kSpinnerFrames[0] + QStringLiteral(" 正在分析..."));
    m_loadingTimer->start();
}

void AIAssistantPanel::stopLoadingAnimation()
{
    m_loadingTimer->stop();
}

// ============================================================
// Conversation history
// ============================================================

void AIAssistantPanel::archiveCurrentConversation()
{
    if (m_messages.isEmpty()) {
        return;
    }

    ConversationSnapshot snapshot;
    snapshot.title = buildConversationTitle(m_messages);
    snapshot.fileName = m_currentFileName;
    snapshot.messages = m_messages;
    snapshot.lastPrompt = m_lastSentPrompt;

    // 避免连续归档完全相同的会话快照
    if (!m_conversationHistory.isEmpty()) {
        const ConversationSnapshot &last = m_conversationHistory.last();
        if (last.fileName == snapshot.fileName
            && last.messages.size() == snapshot.messages.size()
            && last.lastPrompt == snapshot.lastPrompt) {
            bool same = true;
            for (int i = 0; i < last.messages.size(); ++i) {
                if (last.messages[i].role != snapshot.messages[i].role
                    || last.messages[i].text != snapshot.messages[i].text
                    || last.messages[i].intent != snapshot.messages[i].intent) {
                    same = false;
                    break;
                }
            }
            if (same) {
                return;
            }
        }
    }

    // 限制历史记录数量，保留最近 20 条
    if (m_conversationHistory.size() >= 20) {
        m_conversationHistory.remove(0, m_conversationHistory.size() - 19);
    }

    m_conversationHistory.append(snapshot);
}

QString AIAssistantPanel::buildConversationTitle(const QVector<ChatMessage> &messages) const
{
    // 从第一条用户消息中提取标题
    for (const ChatMessage &msg : messages) {
        if (msg.role == QStringLiteral("user")) {
            QString title = msg.text.trimmed();
            if (title.length() > 24) {
                title = title.left(24) + QStringLiteral("...");
            }
            return title;
        }
    }

    return QStringLiteral("新对话");
}

void AIAssistantPanel::onHistoryButtonClicked()
{
    QMenu menu(this);
    menu.setStyleSheet(QStringLiteral(
        "QMenu { background:#ffffff; border:1px solid #e7e2db; border-radius:12px; padding:6px; }"
        "QMenu::item { padding:7px 10px; border-radius:8px; font-size:13px; color:#344054; }"
        "QMenu::item:selected { background:#f3f1ed; }"
        "QMenu::item:disabled { color:#a4acb9; background:transparent; }"
        "QMenu::separator { height:1px; background:#efebe6; margin:6px 4px; }"));

    auto addTitleChip = [&menu](const QString &text) {
        auto *label = new QLabel(text, &menu);
        label->setStyleSheet(QStringLiteral(
            "QLabel { background:#4b5563; color:#ffffff; border-radius:7px; padding:3px 8px;"
            "font-size:12px; font-weight:700; margin:2px 2px 4px 2px; }"));
        auto *action = new QWidgetAction(&menu);
        action->setDefaultWidget(label);
        menu.addAction(action);
    };

    auto addSectionTitle = [&menu](const QString &text) {
        auto *label = new QLabel(text, &menu);
        label->setStyleSheet(QStringLiteral(
            "QLabel { color:#b0b7c3; font-size:12px; font-weight:700; padding:4px 6px 2px 6px; }"));
        auto *action = new QWidgetAction(&menu);
        action->setDefaultWidget(label);
        menu.addAction(action);
    };

    auto addHistoryAction = [](QMenu *target, const QString &title, int historyIndex, bool enabled = true) {
        QString shownTitle = title.trimmed();
        if (shownTitle.isEmpty()) {
            shownTitle = QStringLiteral("未命名对话");
        }
        QAction *action = target->addAction(shownTitle);
        action->setEnabled(enabled);
        if (enabled) {
            action->setProperty("historyIndex", historyIndex);
        }
        return action;
    };

    addTitleChip(QStringLiteral("历史对话"));
    addSectionTitle(QStringLiteral("关于当前文档的对话"));
    bool hasCurrentDocItem = false;
    if (!m_messages.isEmpty()) {
        QString currentTitle = buildConversationTitle(m_messages);
        if (currentTitle.isEmpty()) {
            currentTitle = QStringLiteral("当前对话");
        }
        addHistoryAction(&menu, currentTitle, -1);
        hasCurrentDocItem = true;
    }
    if (!m_currentFileName.isEmpty()) {
        int currentDocCount = 0;
        for (int i = m_conversationHistory.size() - 1; i >= 0 && currentDocCount < 3; --i) {
            const ConversationSnapshot &snapshot = m_conversationHistory[i];
            if (snapshot.fileName == m_currentFileName) {
                addHistoryAction(&menu, snapshot.title, i);
                hasCurrentDocItem = true;
                ++currentDocCount;
            }
        }
    }
    if (!hasCurrentDocItem) {
        addHistoryAction(&menu, QStringLiteral("暂无相关历史"), -3, false);
    }

    menu.addSeparator();
    addSectionTitle(QStringLiteral("最近对话"));
    if (m_conversationHistory.isEmpty()) {
        addHistoryAction(&menu, QStringLiteral("暂无历史对话"), -3, false);
    } else {
        int shown = 0;
        for (int i = m_conversationHistory.size() - 1; i >= 0 && shown < 4; --i, ++shown) {
            const ConversationSnapshot &snapshot = m_conversationHistory[i];
            addHistoryAction(&menu, snapshot.title, i);
        }
    }

    menu.addSeparator();

    QMenu *allHistoryMenu = menu.addMenu(QStringLiteral("全部历史对话"));
    if (m_conversationHistory.isEmpty()) {
        addHistoryAction(allHistoryMenu, QStringLiteral("暂无历史对话"), -3, false);
    } else {
        for (int i = m_conversationHistory.size() - 1; i >= 0; --i) {
            const ConversationSnapshot &snapshot = m_conversationHistory[i];
            QAction *action = addHistoryAction(allHistoryMenu, snapshot.title, i);
            if (!snapshot.fileName.isEmpty()) {
                action->setToolTip(snapshot.fileName);
            }
        }
    }

    QAction *favoriteAction = menu.addAction(QStringLiteral("收藏夹"));
    favoriteAction->setEnabled(false);

    QAction *clearHistoryAction = menu.addAction(QStringLiteral("清空历史对话"));
    clearHistoryAction->setProperty("command", QStringLiteral("clear_history"));

    QAction *selected = menu.exec(m_historyButton->mapToGlobal(QPoint(0, m_historyButton->height() + 8)));
    if (!selected) {
        return;
    }

    const QVariant historyIndexVariant = selected->property("historyIndex");
    if (historyIndexVariant.isValid()) {
        const int selectedIndex = historyIndexVariant.toInt();
        if (selectedIndex == -1) {
            focusComposer();
        } else {
            restoreConversation(selectedIndex);
        }
        return;
    }

    const QString command = selected->property("command").toString();
    if (command == QStringLiteral("clear_history")) {
        m_conversationHistory.clear();
        return;
    }
}

void AIAssistantPanel::restoreConversation(int historyIndex)
{
    if (historyIndex < 0 || historyIndex >= m_conversationHistory.size()) {
        return;
    }

    if (!m_messages.isEmpty()) {
        archiveCurrentConversation();
    }

    const ConversationSnapshot &snapshot = m_conversationHistory[historyIndex];
    m_messages = snapshot.messages;
    m_lastSentPrompt = snapshot.lastPrompt;

    refreshTranscript();
    updateSendButtonStyle();
}

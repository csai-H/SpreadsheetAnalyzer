#include "AIInsightService.h"

#include "../core/TableData.h"
#include "../statistics/DescriptiveStats.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSettings>
#include <QUrl>
#include <QtMath>

#include <algorithm>

namespace {

QString configuredValue(const QString &key,
                        const QStringList &environmentFallbacks = {})
{
    QSettings settings;
    const QString storedValue = settings.value(key).toString().trimmed();
    if (!storedValue.isEmpty()) {
        return storedValue;
    }

    for (const QString &variableName : environmentFallbacks) {
        const QString value = qEnvironmentVariable(variableName.toUtf8().constData()).trimmed();
        if (!value.isEmpty()) {
            return value;
        }
    }

    return QString();
}

QString shortenCellText(QString text, int maxLength = 36)
{
    text = text.simplified();
    if (text.isEmpty()) {
        return QStringLiteral("<空>");
    }

    if (text.size() <= maxLength) {
        return text;
    }

    return text.left(maxLength - 3) + "...";
}

bool tryParseNumber(QString text, double *value)
{
    text = text.trimmed();
    if (text.isEmpty()) {
        return false;
    }

    bool percent = false;
    if (text.endsWith('%')) {
        percent = true;
        text.chop(1);
    }

    text.remove(' ');
    text.remove(',');

    bool ok = false;
    const double parsed = text.toDouble(&ok);
    if (!ok) {
        return false;
    }

    *value = percent ? parsed / 100.0 : parsed;
    return true;
}

QString formatDouble(double value)
{
    return QString::number(value, 'f', 4);
}

QString frequencyPreview(const QHash<QString, int> &counts, int limit = 3)
{
    QVector<QPair<QString, int>> items;
    items.reserve(counts.size());
    for (auto it = counts.cbegin(); it != counts.cend(); ++it) {
        items.append(qMakePair(it.key(), it.value()));
    }

    std::sort(items.begin(), items.end(), [](const auto &left, const auto &right) {
        if (left.second != right.second) {
            return left.second > right.second;
        }
        return left.first < right.first;
    });

    QStringList preview;
    for (int index = 0; index < items.size() && index < limit; ++index) {
        preview << QStringLiteral("%1(%2)")
                       .arg(shortenCellText(items[index].first, 20))
                       .arg(items[index].second);
    }

    return preview.isEmpty() ? QStringLiteral("无") : preview.join(QStringLiteral("、"));
}

QString normalizeMultilineText(QString text)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    return text;
}

bool isUsefulAnalysisHeading(const QString &line)
{
    static const QRegularExpression regex(
        QStringLiteral(
            "^\\s{0,3}(?:#{1,6}\\s*)?(?:\\d+[\\.、]\\s*)?"
            "(数据概览|核心指标|关键结论|分析结论|关键发现|主要发现|数据质量|异常点|建议|图表建议|可视化建议|重点发现|下一步)"
            "\\s*[:：]?$"));
    return regex.match(line.trimmed()).hasMatch();
}

bool isInstructionEchoLine(const QString &line)
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    static const QRegularExpression headingRegex(
        QStringLiteral(
            "^\\s{0,3}(?:#{1,6}\\s*)?(?:\\d+[\\.、]\\s*)?"
            "(分析请求|用户(?:问题|诉求|需求|关注点)|角色|约束|结构|输出(?:格式|要求)|表格摘要|数据摘要|任务说明|思考过程|推理过程|处理步骤|工作流)"
            "\\s*[:：]?$"));
    static const QRegularExpression bulletRegex(
        QStringLiteral(
            "^\\s*[-*•]\\s*(角色|约束|结构|输出(?:格式|要求)|任务说明|用户(?:问题|诉求|需求|关注点)|表格摘要|数据摘要)"
            "\\s*[:：].*$"));

    return headingRegex.match(trimmed).hasMatch()
           || bulletRegex.match(trimmed).hasMatch()
           || trimmed.contains(QStringLiteral("不要编造不存在的字段"))
           || trimmed.contains(QStringLiteral("不要复述提示词"))
           || trimmed.contains(QStringLiteral("角色设定"))
           || trimmed.contains(QStringLiteral("结构说明"))
           || trimmed.contains(QStringLiteral("内部步骤"))
           || trimmed.contains(QStringLiteral("用户问题原文"))
           || trimmed.contains(QStringLiteral("数据摘要标题"));
}

QString collapseBlankLines(QString text)
{
    text = normalizeMultilineText(text).trimmed();
    text.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));
    return text.trimmed();
}

QString normalizeEndpoint(QString endpoint)
{
    endpoint = endpoint.trimmed();
    if (endpoint.isEmpty()) {
        return endpoint;
    }

    while (endpoint.endsWith(QLatin1Char('/'))) {
        endpoint.chop(1);
    }

    if (endpoint.endsWith(QStringLiteral("/api/paas/v4"))) {
        endpoint += QStringLiteral("/chat/completions");
    }

    return endpoint;
}

QString stripInstructionEcho(QString text)
{
    text = normalizeMultilineText(text).trimmed();
    if (text.isEmpty()) {
        return text;
    }

    QStringList lines = text.split(QLatin1Char('\n'));
    int firstUsefulLine = -1;
    int firstEchoLine = -1;

    for (int index = 0; index < lines.size(); ++index) {
        const QString line = lines[index].trimmed();
        if (firstEchoLine < 0 && isInstructionEchoLine(line)) {
            firstEchoLine = index;
        }
        if (firstUsefulLine < 0 && isUsefulAnalysisHeading(line)) {
            firstUsefulLine = index;
        }
    }

    if (firstEchoLine >= 0 && firstUsefulLine > firstEchoLine) {
        lines = lines.mid(firstUsefulLine);
    }

    QStringList cleaned;
    bool seenUsefulSection = false;
    bool previousBlank = false;

    for (const QString &rawLine : lines) {
        const QString trimmed = rawLine.trimmed();
        const bool isBlank = trimmed.isEmpty();

        if (!seenUsefulSection && isInstructionEchoLine(trimmed)) {
            continue;
        }

        if (isUsefulAnalysisHeading(trimmed)) {
            seenUsefulSection = true;
        }

        if (seenUsefulSection && isInstructionEchoLine(trimmed)) {
            continue;
        }

        if (isBlank) {
            if (previousBlank) {
                continue;
            }
            previousBlank = true;
            cleaned << QString();
            continue;
        }

        previousBlank = false;
        cleaned << rawLine;
    }

    return collapseBlankLines(cleaned.join(QLatin1Char('\n')));
}

} // namespace

AIInsightService::AIInsightService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void AIInsightService::requestInsights(const QSharedPointer<Core::TableData> &data,
                                       const QString &focusPrompt,
                                       bool filteredView,
                                       bool useDataContext)
{
    if (m_busy) {
        emit requestFailed(QStringLiteral("上一条 AI 请求尚未完成，请稍后重试。"));
        return;
    }

    if (useDataContext && (data.isNull() || data->isEmpty())) {
        emit requestFailed(QStringLiteral("当前没有可供分析的数据。"));
        return;
    }

    const QString apiKey = configuredApiKey();
    if (apiKey.isEmpty()) {
        emit requestFailed(QStringLiteral("未配置智谱 API Key，请先在设置中填写或设置环境变量。"));
        return;
    }

    QNetworkRequest request{QUrl(configuredEndpoint())};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), configuredModel());
    payload.insert(QStringLiteral("stream"), false);
    payload.insert(QStringLiteral("temperature"), 0.3);
    payload.insert(QStringLiteral("max_tokens"), 1600);
    payload.insert(QStringLiteral("thinking"), QJsonObject{
        {QStringLiteral("type"), QStringLiteral("disabled")}
    });

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"), buildSystemPrompt()}
    });

    QString userPrompt;
    if (useDataContext) {
        userPrompt = buildPrompt(*data, focusPrompt, filteredView);
    } else {
        userPrompt = buildGeneralPrompt(focusPrompt);
    }

    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), userPrompt}
    });
    payload.insert(QStringLiteral("messages"), messages);

    m_busy = true;
    emit requestStarted();

    QNetworkReply *reply = m_networkManager->post(
        request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

bool AIInsightService::isBusy() const
{
    return m_busy;
}

QString AIInsightService::defaultEndpoint()
{
    return QStringLiteral("https://open.bigmodel.cn/api/paas/v4/chat/completions");
}

QString AIInsightService::defaultModel()
{
    return QStringLiteral("glm-4.7-flash");
}

QString AIInsightService::configuredApiKey()
{
    return configuredValue(QStringLiteral("AI/apiKey"),
                           {QStringLiteral("ZHIPUAI_API_KEY"),
                            QStringLiteral("ZHIPU_API_KEY"),
                            QStringLiteral("GLM_API_KEY")});
}

QString AIInsightService::configuredEndpoint()
{
    const QString endpoint = configuredValue(QStringLiteral("AI/endpoint"));
    return endpoint.isEmpty() ? defaultEndpoint() : normalizeEndpoint(endpoint);
}

QString AIInsightService::configuredModel()
{
    const QString model = configuredValue(QStringLiteral("AI/model"));
    return model.isEmpty() ? defaultModel() : model;
}

QString AIInsightService::buildSystemPrompt() const
{
    return QStringLiteral("你是办公软件右侧侧栏里的 AI 助手。"
                          "你既可以回答通用问题，也可以基于表格数据做分析。"
                          "当用户问题与表格无关时，请直接给出通用问答，不要拒答。"
                          "当用户问题与表格有关时，再结合数据摘要回答，不要编造字段和数值。"
                          "请直接回答用户问题，不要输出思考过程或复述提示词。"
                          "使用中文和简洁 Markdown。");
}

QString AIInsightService::buildGeneralPrompt(const QString &focusPrompt) const
{
    const QString effectivePrompt = focusPrompt.trimmed().isEmpty()
                                        ? QStringLiteral("请直接回答用户的问题。")
                                        : focusPrompt.trimmed();
    return QStringLiteral("请直接回答下面的问题，不需要引用表格数据：\n%1").arg(effectivePrompt);
}

QString AIInsightService::buildPrompt(const Core::TableData &data,
                                      const QString &focusPrompt,
                                      bool filteredView) const
{
    const QString effectivePrompt = focusPrompt.trimmed().isEmpty()
                                        ? QStringLiteral("请概述该数据的结构、关键发现、可能的异常点，并给出下一步分析建议。")
                                        : focusPrompt.trimmed();

    QString prompt;
    prompt += QStringLiteral("【重要】下面是用户的问题，请直接、针对性地回答，不要套用任何固定模板：\n");
    prompt += effectivePrompt;
    prompt += QStringLiteral("\n\n下面的数据摘要仅供分析参考（不要逐段复述）：\n");
    prompt += buildDatasetSummary(data, filteredView);
    return prompt;
}

QString AIInsightService::buildDatasetSummary(const Core::TableData &data,
                                              bool filteredView) const
{
    QStringList lines;
    lines << QStringLiteral("分析范围: %1")
                 .arg(filteredView ? QStringLiteral("当前筛选后的可见数据")
                                   : QStringLiteral("当前完整数据"));
    lines << QStringLiteral("数据规模: %1 行 x %2 列")
                 .arg(data.rowCount())
                 .arg(data.columnCount());
    lines << QStringLiteral("列名: %1").arg(data.headers().join(QStringLiteral("、")));
    lines << QString();
    lines << QStringLiteral("字段概览:");

    for (int column = 0; column < data.columnCount(); ++column) {
        const QString header = data.header(column);
        int nonEmptyCount = 0;
        int numericCount = 0;
        int invalidNumericCandidateCount = 0;
        QVector<double> numericValues;
        QHash<QString, int> frequencies;

        for (int row = 0; row < data.rowCount(); ++row) {
            const QString text = data.at(row, column).toString().trimmed();
            if (text.isEmpty()) {
                continue;
            }

            ++nonEmptyCount;
            frequencies[text] += 1;

            double parsedValue = 0.0;
            if (tryParseNumber(text, &parsedValue)) {
                ++numericCount;
                numericValues.append(parsedValue);
            } else {
                ++invalidNumericCandidateCount;
            }
        }

        const int emptyCount = data.rowCount() - nonEmptyCount;
        const bool looksNumeric =
            nonEmptyCount > 0 &&
            numericCount >= qMax(3, static_cast<int>(qCeil(nonEmptyCount * 0.6)));

        if (looksNumeric && !numericValues.isEmpty()) {
            const Statistics::DescriptiveSummary summary =
                Statistics::DescriptiveStats::summarize(numericValues);
            lines << QStringLiteral(
                         "- %1: 数值列，非空=%2，空值=%3，可解析数值=%4，均值=%5，中位数=%6，最小=%7，最大=%8，标准差=%9")
                         .arg(header)
                         .arg(nonEmptyCount)
                         .arg(emptyCount)
                         .arg(numericCount)
                         .arg(formatDouble(summary.mean))
                         .arg(formatDouble(summary.median))
                         .arg(formatDouble(summary.min))
                         .arg(formatDouble(summary.max))
                         .arg(formatDouble(summary.stdDev));
        } else {
            lines << QStringLiteral(
                         "- %1: 文本/类别列，非空=%2，空值=%3，唯一值=%4，常见值=%5")
                         .arg(header)
                         .arg(nonEmptyCount)
                         .arg(emptyCount)
                         .arg(frequencies.size())
                         .arg(frequencyPreview(frequencies));

            if (numericCount > 0 && invalidNumericCandidateCount > 0) {
                lines << QStringLiteral("  备注: 该列存在混合格式，%1 个值可解析为数值，%2 个值无法解析。")
                             .arg(numericCount)
                             .arg(invalidNumericCandidateCount);
            }
        }
    }

    const int maxSampleRows = qMin(data.rowCount(), 8);
    const int maxSampleColumns = qMin(data.columnCount(), 8);
    lines << QString();
    lines << QStringLiteral("样本行:");
    for (int row = 0; row < maxSampleRows; ++row) {
        QStringList cells;
        for (int column = 0; column < maxSampleColumns; ++column) {
            cells << QStringLiteral("%1=%2")
                         .arg(data.header(column), shortenCellText(data.at(row, column).toString()));
        }

        if (data.columnCount() > maxSampleColumns) {
            cells << QStringLiteral("其余 %1 列已省略").arg(data.columnCount() - maxSampleColumns);
        }

        lines << QStringLiteral("%1. %2").arg(row + 1).arg(cells.join(QStringLiteral(" | ")));
    }

    if (data.rowCount() > maxSampleRows) {
        lines << QStringLiteral("其余 %1 行样本已省略。").arg(data.rowCount() - maxSampleRows);
    }

    return lines.join('\n');
}

QString AIInsightService::objectKeys(const QJsonObject &object) const
{
    QStringList keys = object.keys();
    std::sort(keys.begin(), keys.end());
    return keys.join(QStringLiteral(", "));
}

QString AIInsightService::responseDumpPath() const
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/ai_last_response.json");
}

void AIInsightService::writeResponseDump(const QByteArray &body) const
{
    QFile file(responseDumpPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }

    file.write(body);
    file.close();
}

QString AIInsightService::extractTextContent(const QJsonValue &contentValue) const
{
    if (contentValue.isString()) {
        return contentValue.toString();
    }

    if (!contentValue.isArray()) {
        return QString();
    }

    QStringList fragments;
    const QJsonArray items = contentValue.toArray();
    for (const QJsonValue &item : items) {
        if (item.isString()) {
            fragments << item.toString();
            continue;
        }

        if (!item.isObject()) {
            continue;
        }

        const QJsonObject object = item.toObject();
        if (object.contains(QStringLiteral("text"))) {
            fragments << object.value(QStringLiteral("text")).toString();
        } else if (object.contains(QStringLiteral("content"))) {
            fragments << object.value(QStringLiteral("content")).toString();
        }
    }

    return fragments.join(QString());
}

QString AIInsightService::sanitizeAssistantText(const QString &text) const
{
    QString sanitized = normalizeMultilineText(text).trimmed();
    sanitized.remove(QRegularExpression(QStringLiteral("<think>[\\s\\S]*?</think>")));
    sanitized.remove(QRegularExpression(QStringLiteral("^```[a-zA-Z0-9_-]*\\s*")));
    sanitized.remove(QRegularExpression(QStringLiteral("\\s*```$")));
    sanitized = stripInstructionEcho(sanitized);

    if (sanitized.startsWith(QStringLiteral("完整解读"))) {
        sanitized.remove(QRegularExpression(QStringLiteral("^完整解读\\s*[:：]\\s*")));
    }

    return collapseBlankLines(sanitized);
}

void AIInsightService::handleReply(QNetworkReply *reply)
{
    const QByteArray body = reply->readAll();
    writeResponseDump(body);
    const QJsonDocument document = QJsonDocument::fromJson(body);
    const QJsonObject root = document.object();

    auto finishWithError = [this, reply](const QString &message) {
        m_busy = false;
        emit requestFailed(message);
        reply->deleteLater();
    };

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMessage = reply->errorString();
        if (root.contains(QStringLiteral("error")) && root.value(QStringLiteral("error")).isObject()) {
            const QJsonObject errorObject = root.value(QStringLiteral("error")).toObject();
            const QString apiMessage = errorObject.value(QStringLiteral("message")).toString();
            if (!apiMessage.isEmpty()) {
                errorMessage = apiMessage;
            }
        } else if (root.contains(QStringLiteral("msg"))) {
            const QString apiMessage = root.value(QStringLiteral("msg")).toString();
            if (!apiMessage.isEmpty()) {
                errorMessage = apiMessage;
            }
        }

        finishWithError(QStringLiteral("AI 请求失败: %1").arg(errorMessage));
        return;
    }

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        finishWithError(QStringLiteral("AI 返回为空，未找到可用结果。原始响应已保存到 %1。")
                            .arg(responseDumpPath()));
        return;
    }

    const QJsonObject choiceObject = choices.first().toObject();
    const QJsonObject messageObject = choiceObject.value(QStringLiteral("message")).toObject();
    const QJsonObject deltaObject = choiceObject.value(QStringLiteral("delta")).toObject();
    const QString finishReason = choiceObject.value(QStringLiteral("finish_reason")).toString();

    QString content = sanitizeAssistantText(
        extractTextContent(messageObject.value(QStringLiteral("content"))));
    if (content.isEmpty()) {
        content = sanitizeAssistantText(messageObject.value(QStringLiteral("content")).toString());
    }
    if (content.isEmpty()) {
        content = sanitizeAssistantText(extractTextContent(deltaObject.value(QStringLiteral("content"))));
    }
    if (content.isEmpty()) {
        content = sanitizeAssistantText(deltaObject.value(QStringLiteral("content")).toString());
    }
    if (content.isEmpty()) {
        content = sanitizeAssistantText(root.value(QStringLiteral("output_text")).toString());
    }
    if (content.isEmpty()) {
        content = sanitizeAssistantText(root.value(QStringLiteral("text")).toString());
    }
    if (content.isEmpty() && root.value(QStringLiteral("data")).isObject()) {
        const QJsonObject dataObject = root.value(QStringLiteral("data")).toObject();
        content = sanitizeAssistantText(dataObject.value(QStringLiteral("content")).toString());
        if (content.isEmpty()) {
            content = sanitizeAssistantText(dataObject.value(QStringLiteral("text")).toString());
        }
    }

    if (content.isEmpty()) {
        QString reasoningHint;
        if (messageObject.contains(QStringLiteral("reasoning_content"))
            && !messageObject.value(QStringLiteral("reasoning_content")).toString().trimmed().isEmpty()) {
            reasoningHint = QStringLiteral(
                "\n模型返回了 reasoning_content，但没有返回最终 content。"
                "\n侧栏不会直接显示推理过程文本。");
        }

        if (finishReason == QStringLiteral("length")) {
            finishWithError(QStringLiteral(
                "AI 在生成最终答案前已达到长度上限，未返回可显示的正文。%1\n原始响应已保存到: %2")
                                .arg(reasoningHint, responseDumpPath()));
            return;
        }

        QString diagnostic = QStringLiteral(
            "AI 返回成功，但没有解析到正文。\n"
            "模型: %1\n"
            "finish_reason: %2\n"
            "choice 字段: %3\n"
            "message 字段: %4\n"
            "delta 字段: %5\n"
            "原始响应已保存到: %6%7")
                                 .arg(root.value(QStringLiteral("model")).toString(configuredModel()),
                                      finishReason.isEmpty() ? QStringLiteral("<空>") : finishReason,
                                      objectKeys(choiceObject),
                                      objectKeys(messageObject),
                                      objectKeys(deltaObject),
                                      responseDumpPath(),
                                      reasoningHint);

        finishWithError(diagnostic);
        return;
    }

    m_busy = false;
    emit requestFinished(content);
    reply->deleteLater();
}

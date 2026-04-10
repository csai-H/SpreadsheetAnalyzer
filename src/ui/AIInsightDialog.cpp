#include "AIInsightDialog.h"

#include "../ai/AIInsightService.h"
#include "../core/TableData.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

AIInsightDialog::AIInsightDialog(const QSharedPointer<Core::TableData> &data,
                                 bool filteredView,
                                 QWidget *parent)
    : QDialog(parent)
    , m_data(data)
    , m_filteredView(filteredView)
    , m_service(new AIInsightService(this))
{
    setWindowTitle(QStringLiteral("AI 数据洞察"));
    resize(840, 680);

    auto *mainLayout = new QVBoxLayout(this);

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);
    updateSummaryLabel();
    mainLayout->addWidget(m_summaryLabel);

    auto *focusLabel = new QLabel(QStringLiteral("告诉 AI 你的关注点："), this);
    mainLayout->addWidget(focusLabel);

    m_focusEdit = new QPlainTextEdit(this);
    m_focusEdit->setPlaceholderText(
        QStringLiteral("例如：请关注异常值、增长趋势、字段质量问题，并给出 3 条后续分析建议。"));
    m_focusEdit->setMaximumHeight(110);
    m_focusEdit->setPlainText(
        QStringLiteral("请概述该数据的结构、关键发现、可能的异常点，并给出下一步分析建议。"));
    mainLayout->addWidget(m_focusEdit);

    auto *resultLabel = new QLabel(QStringLiteral("AI 返回结果："), this);
    mainLayout->addWidget(resultLabel);

    m_resultEdit = new QTextEdit(this);
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText(QStringLiteral("点击“生成洞察”后，这里会显示 AI 分析结果。"));
    mainLayout->addWidget(m_resultEdit, 1);

    auto *buttonBox = new QDialogButtonBox(this);
    m_generateButton = buttonBox->addButton(QStringLiteral("生成洞察"), QDialogButtonBox::ActionRole);
    m_copyButton = buttonBox->addButton(QStringLiteral("复制结果"), QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_generateButton, &QPushButton::clicked, this, &AIInsightDialog::onGenerateClicked);
    connect(m_copyButton, &QPushButton::clicked, this, &AIInsightDialog::onCopyClicked);
    m_copyButton->setEnabled(false);

    m_statusLabel = new QLabel(QStringLiteral("就绪"), this);
    m_statusLabel->setWordWrap(true);

    auto *footerLayout = new QHBoxLayout();
    footerLayout->addWidget(m_statusLabel, 1);
    footerLayout->addWidget(buttonBox);
    mainLayout->addLayout(footerLayout);

    connect(m_service, &AIInsightService::requestStarted,
            this, &AIInsightDialog::onRequestStarted);
    connect(m_service, &AIInsightService::requestFinished,
            this, &AIInsightDialog::onRequestFinished);
    connect(m_service, &AIInsightService::requestFailed,
            this, &AIInsightDialog::onRequestFailed);
}

AIInsightDialog::~AIInsightDialog() = default;

void AIInsightDialog::onGenerateClicked()
{
    if (m_data.isNull() || m_data->isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("当前没有可供分析的数据。"));
        return;
    }

    m_service->requestInsights(m_data, m_focusEdit->toPlainText(), m_filteredView);
}

void AIInsightDialog::onCopyClicked()
{
    const QString text = m_resultEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    QApplication::clipboard()->setText(text);
    m_statusLabel->setText(QStringLiteral("AI 结果已复制到剪贴板。"));
}

void AIInsightDialog::onRequestStarted()
{
    m_generateButton->setEnabled(false);
    m_copyButton->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("正在调用智谱 GLM 接口，请稍候..."));
}

void AIInsightDialog::onRequestFinished(const QString &insights)
{
    m_resultEdit->setPlainText(insights);
    m_generateButton->setEnabled(true);
    m_copyButton->setEnabled(true);
    m_statusLabel->setText(QStringLiteral("AI 洞察生成完成。"));
}

void AIInsightDialog::onRequestFailed(const QString &errorMessage)
{
    m_generateButton->setEnabled(true);
    m_statusLabel->setText(errorMessage);
    QMessageBox::warning(this, QStringLiteral("AI 请求失败"), errorMessage);
}

void AIInsightDialog::updateSummaryLabel()
{
    const QString scopeText = m_filteredView
                                  ? QStringLiteral("当前筛选后的可见数据")
                                  : QStringLiteral("当前完整数据");
    const int rowCount = m_data ? m_data->rowCount() : 0;
    const int columnCount = m_data ? m_data->columnCount() : 0;

    m_summaryLabel->setText(
        QStringLiteral("本次将分析 %1，数据规模为 %2 行 × %3 列。为了控制 token 消耗，发送给模型的是字段摘要和样本行，不是整张表。")
            .arg(scopeText)
            .arg(rowCount)
            .arg(columnCount));
}

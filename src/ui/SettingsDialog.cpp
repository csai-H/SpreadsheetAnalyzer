#include "SettingsDialog.h"
#include "../utils/ThemeManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    resize(550, 450);

    m_tabWidget = new QTabWidget(this);

    loadAvailableThemes();

    setupGeneralTab();
    setupViewTab();
    setupAppearanceTab();
    setupChartTab();

    // 按钮框
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
        QDialogButtonBox::Apply | QDialogButtonBox::Reset
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &SettingsDialog::onResetToDefaults);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addWidget(buttonBox);

    loadSettings();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::loadAvailableThemes()
{
    m_availableThemes = ThemeManager::instance().availableThemes();

    // 保存当前主题名称
    m_initialTheme = ThemeManager::instance().currentTheme().name;
}

void SettingsDialog::setupGeneralTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    // 自动保存
    auto *generalGroup = new QGroupBox("常规");
    auto *generalLayout = new QFormLayout();

    m_checkAutoSave = new QCheckBox("自动保存");
    m_spinAutoSaveInterval = new QSpinBox();
    m_spinAutoSaveInterval->setRange(1, 60);
    m_spinAutoSaveInterval->setValue(5);
    m_spinAutoSaveInterval->setSuffix(" 分钟");

    m_checkReopenFiles = new QCheckBox("重新打开文件");
    m_checkConfirmOnExit = new QCheckBox("退出时确认");

    generalLayout->addRow("自动保存:", m_checkAutoSave);
    generalLayout->addRow("保存间隔:", m_spinAutoSaveInterval);
    generalLayout->addRow(m_checkReopenFiles);
    generalLayout->addRow(m_checkConfirmOnExit);

    generalGroup->setLayout(generalLayout);
    layout->addWidget(generalGroup);
    layout->addStretch();

    m_tabWidget->addTab(tab, "常规");
}

void SettingsDialog::setupViewTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    auto *viewGroup = new QGroupBox("视图");
    auto *viewLayout = new QFormLayout();

    m_spinFontSize = new QSpinBox();
    m_spinFontSize->setRange(8, 24);
    m_spinFontSize->setValue(10);
    m_spinFontSize->setSuffix(" pt");

    m_comboTheme = new QComboBox();
    m_comboTheme->addItem("默认");
    m_comboTheme->addItem("深色");
    m_comboTheme->addItem("柔和");
    m_comboTheme->addItem("鲜艳");
    m_comboTheme->addItem("商务");

    m_checkShowGrid = new QCheckBox("显示网格线");
    m_checkShowGrid->setChecked(true);

    m_checkShowToolbar = new QCheckBox("显示工具栏");
    m_checkShowToolbar->setChecked(true);

    viewLayout->addRow("字体大小:", m_spinFontSize);
    viewLayout->addRow("颜色主题:", m_comboTheme);
    viewLayout->addRow(m_checkShowGrid);
    viewLayout->addRow(m_checkShowToolbar);

    viewGroup->setLayout(viewLayout);
    layout->addWidget(viewGroup);
    layout->addStretch();

    m_tabWidget->addTab(tab, "视图");
}

void SettingsDialog::setupAppearanceTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    auto *appearanceGroup = new QGroupBox("界面主题");
    auto *appearanceLayout = new QVBoxLayout();

    // 主题选择
    auto *themeSelectLayout = new QHBoxLayout();
    auto *themeLabel = new QLabel("选择主题:");

    m_comboAppTheme = new QComboBox();
    for (const ThemeInfo& theme : m_availableThemes) {
        m_comboAppTheme->addItem(theme.displayName, theme.name);
    }

    // 设置当前主题
    QString currentThemeName = ThemeManager::instance().currentTheme().name;
    int currentIndex = m_comboAppTheme->findData(currentThemeName);
    if (currentIndex >= 0) {
        m_comboAppTheme->setCurrentIndex(currentIndex);
    }

    connect(m_comboAppTheme, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);

    themeSelectLayout->addWidget(themeLabel);
    themeSelectLayout->addWidget(m_comboAppTheme);
    themeSelectLayout->addStretch();

    // 预览按钮
    m_btnPreviewTheme = new QPushButton("预览主题");
    connect(m_btnPreviewTheme, &QPushButton::clicked, this, &SettingsDialog::onPreviewTheme);
    m_btnPreviewTheme->setProperty("class", "secondary");

    themeSelectLayout->addWidget(m_btnPreviewTheme);

    appearanceLayout->addLayout(themeSelectLayout);

    // 主题描述
    m_lblThemeDescription = new QLabel();
    m_lblThemeDescription->setWordWrap(true);
    m_lblThemeDescription->setMinimumHeight(60);
    m_lblThemeDescription->setStyleSheet("padding: 8px; border-radius: 4px;");

    // 更新描述
    onThemeChanged(m_comboAppTheme->currentIndex());

    appearanceLayout->addWidget(m_lblThemeDescription);
    appearanceLayout->addStretch();

    appearanceGroup->setLayout(appearanceLayout);
    layout->addWidget(appearanceGroup);
    layout->addStretch();

    m_tabWidget->addTab(tab, "外观");
}

void SettingsDialog::setupChartTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    auto *chartGroup = new QGroupBox("图表");
    auto *chartLayout = new QFormLayout();

    m_checkAnimation = new QCheckBox("启用动画");
    m_checkAnimation->setChecked(true);

    m_spinAnimationDuration = new QSpinBox();
    m_spinAnimationDuration->setRange(100, 2000);
    m_spinAnimationDuration->setValue(500);
    m_spinAnimationDuration->setSuffix(" ms");

    m_comboDefaultChartType = new QComboBox();
    m_comboDefaultChartType->addItem("柱状图");
    m_comboDefaultChartType->addItem("折线图");
    m_comboDefaultChartType->addItem("散点图");
    m_comboDefaultChartType->addItem("饼图");

    chartLayout->addRow(m_checkAnimation);
    chartLayout->addRow("动画时长:", m_spinAnimationDuration);
    chartLayout->addRow("默认图表类型:", m_comboDefaultChartType);

    chartGroup->setLayout(chartLayout);
    layout->addWidget(chartGroup);
    layout->addStretch();

    m_tabWidget->addTab(tab, "图表");
}

void SettingsDialog::onThemeChanged(int index)
{
    if (index < 0 || index >= m_availableThemes.size()) {
        return;
    }

    const ThemeInfo& theme = m_availableThemes[index];
    m_lblThemeDescription->setText(QString("<b>%1</b><br>%2")
        .arg(theme.displayName)
        .arg(theme.description));

    // 根据主题类型设置描述背景色
    if (theme.type == ThemeType::Dark) {
        m_lblThemeDescription->setStyleSheet(
            "padding: 8px; border-radius: 4px; "
            "background-color: #313244; color: #cdd6f4;"
        );
    } else {
        m_lblThemeDescription->setStyleSheet(
            "padding: 8px; border-radius: 4px; "
            "background-color: #ecf5ff; color: #409eff;"
        );
    }
}

void SettingsDialog::onPreviewTheme()
{
    int index = m_comboAppTheme->currentIndex();
    if (index < 0 || index >= m_availableThemes.size()) {
        return;
    }

    const ThemeInfo& theme = m_availableThemes[index];
    ThemeManager::instance().setTheme(theme);
}

void SettingsDialog::onAccepted()
{
    saveSettings();
    accept();
}

void SettingsDialog::onRejected()
{
    // 恢复初始主题
    if (!m_initialTheme.isEmpty()) {
        for (const ThemeInfo& theme : m_availableThemes) {
            if (theme.name == m_initialTheme) {
                ThemeManager::instance().setTheme(theme);
                break;
            }
        }
    }
    reject();
}

void SettingsDialog::onApply()
{
    saveSettings();
}

void SettingsDialog::onResetToDefaults()
{
    // 重置为默认值
    m_checkAutoSave->setChecked(false);
    m_spinAutoSaveInterval->setValue(5);
    m_checkReopenFiles->setChecked(false);
    m_checkConfirmOnExit->setChecked(true);
    m_spinFontSize->setValue(10);
    m_comboTheme->setCurrentIndex(0);
    m_checkShowGrid->setChecked(true);
    m_checkShowToolbar->setChecked(true);
    m_checkAnimation->setChecked(true);
    m_spinAnimationDuration->setValue(500);
    m_comboDefaultChartType->setCurrentIndex(0);

    // 重置主题为默认
    for (int i = 0; i < m_availableThemes.size(); ++i) {
        if (m_availableThemes[i].name == "modern") {
            m_comboAppTheme->setCurrentIndex(i);
            onPreviewTheme();
            break;
        }
    }
}

void SettingsDialog::loadSettings()
{
    QSettings settings;

    settings.beginGroup("General");
    m_checkAutoSave->setChecked(settings.value("autoSave", false).toBool());
    m_spinAutoSaveInterval->setValue(settings.value("autoSaveInterval", 5).toInt());
    m_checkReopenFiles->setChecked(settings.value("reopenFiles", false).toBool());
    m_checkConfirmOnExit->setChecked(settings.value("confirmOnExit", true).toBool());
    settings.endGroup();

    settings.beginGroup("View");
    m_spinFontSize->setValue(settings.value("fontSize", 10).toInt());
    m_comboTheme->setCurrentIndex(settings.value("theme", 0).toInt());
    m_checkShowGrid->setChecked(settings.value("showGrid", true).toBool());
    m_checkShowToolbar->setChecked(settings.value("showToolbar", true).toBool());
    settings.endGroup();

    settings.beginGroup("Chart");
    m_checkAnimation->setChecked(settings.value("animation", true).toBool());
    m_spinAnimationDuration->setValue(settings.value("animationDuration", 500).toInt());
    m_comboDefaultChartType->setCurrentIndex(settings.value("defaultChartType", 0).toInt());
    settings.endGroup();
}

void SettingsDialog::saveSettings()
{
    QSettings settings;

    settings.beginGroup("General");
    settings.setValue("autoSave", m_checkAutoSave->isChecked());
    settings.setValue("autoSaveInterval", m_spinAutoSaveInterval->value());
    settings.setValue("reopenFiles", m_checkReopenFiles->isChecked());
    settings.setValue("confirmOnExit", m_checkConfirmOnExit->isChecked());
    settings.endGroup();

    settings.beginGroup("View");
    settings.setValue("fontSize", m_spinFontSize->value());
    settings.setValue("theme", m_comboTheme->currentIndex());
    settings.setValue("showGrid", m_checkShowGrid->isChecked());
    settings.setValue("showToolbar", m_checkShowToolbar->isChecked());
    settings.endGroup();

    settings.beginGroup("Chart");
    settings.setValue("animation", m_checkAnimation->isChecked());
    settings.setValue("animationDuration", m_spinAnimationDuration->value());
    settings.setValue("defaultChartType", m_comboDefaultChartType->currentIndex());
    settings.endGroup();

    // 保存主题设置
    int themeIndex = m_comboAppTheme->currentIndex();
    if (themeIndex >= 0 && themeIndex < m_availableThemes.size()) {
        const ThemeInfo& theme = m_availableThemes[themeIndex];
        ThemeManager::instance().setTheme(theme);
        ThemeManager::instance().saveCurrentTheme();
        m_initialTheme = theme.name;
    }
}

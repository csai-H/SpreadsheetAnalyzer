#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

// 前向声明
struct ThemeInfo;

/**
 * @brief 设置对话框
 *
 * 应用程序全局设置
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

private slots:
    void onAccepted();
    void onRejected();
    void onResetToDefaults();
    void onApply();
    void onThemeChanged(int index);
    void onPreviewTheme();

private:
    void setupGeneralTab();
    void setupViewTab();
    void setupChartTab();
    void setupAppearanceTab();
    void loadSettings();
    void saveSettings();
    void loadAvailableThemes();

    QTabWidget *m_tabWidget;

    // 常规设置
    QCheckBox *m_checkAutoSave;
    QSpinBox *m_spinAutoSaveInterval;
    QCheckBox *m_checkReopenFiles;
    QCheckBox *m_checkConfirmOnExit;

    // 视图设置
    QSpinBox *m_spinFontSize;
    QComboBox *m_comboTheme;
    QCheckBox *m_checkShowGrid;
    QCheckBox *m_checkShowToolbar;

    // 外观设置
    QComboBox *m_comboAppTheme;
    QPushButton *m_btnPreviewTheme;
    QLabel *m_lblThemeDescription;

    // 图表设置
    QCheckBox *m_checkAnimation;
    QSpinBox *m_spinAnimationDuration;
    QComboBox *m_comboDefaultChartType;

    // 主题信息
    QList<ThemeInfo> m_availableThemes;
    QString m_initialTheme;
};

#endif // SETTINGSDIALOG_H

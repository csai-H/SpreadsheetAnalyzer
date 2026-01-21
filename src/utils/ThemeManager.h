#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QWidget>
#include <memory>

/**
 * @brief 主题类型
 */
enum class ThemeType {
    Light,
    Dark
};

/**
 * @brief 主题信息
 */
struct ThemeInfo {
    QString name;
    QString displayName;
    QString description;
    QString styleSheetPath;
    ThemeType type;

    ThemeInfo() = default;
    ThemeInfo(const QString& n, const QString& dn, const QString& desc,
              const QString& path, ThemeType t)
        : name(n), displayName(dn), description(desc),
          styleSheetPath(path), type(t) {}
};

/**
 * @brief 主题管理器
 *
 * 负责应用程序主题的加载、切换和管理，支持平滑过渡动画
 */
class ThemeManager
{
public:
    /**
     * @brief 获取单例实例
     */
    static ThemeManager& instance();

    /**
     * @brief 初始化主题管理器
     */
    void initialize();

    /**
     * @brief 获取所有可用主题
     */
    QList<ThemeInfo> availableThemes() const;

    /**
     * @brief 获取当前主题
     */
    ThemeInfo currentTheme() const;

    /**
     * @brief 设置主题（带动画效果）
     * @param themeName 主题名称
     * @param enableAnimation 是否启用动画，默认为true
     * @return 是否成功
     */
    bool setTheme(const QString& themeName, bool enableAnimation = true);

    /**
     * @brief 设置主题（使用ThemeInfo，带动画效果）
     * @param theme 主题信息
     * @param enableAnimation 是否启用动画，默认为true
     * @return 是否成功
     */
    bool setTheme(const ThemeInfo& theme, bool enableAnimation = true);

    /**
     * @brief 从文件加载样式表
     * @param filePath 样式表文件路径
     * @return 样式表内容，失败返回空字符串
     */
    QString loadStyleSheet(const QString& filePath);

    /**
     * @brief 保存当前主题设置
     */
    void saveCurrentTheme();

    /**
     * @brief 加载保存的主题设置
     */
    void loadSavedTheme();

    /**
     * @brief 获取主题样式表目录
     */
    QString stylesDirectory() const;

    /**
     * @brief 刷新主题（重新应用当前主题）
     */
    void refresh();

    /**
     * @brief 设置动画持续时间（毫秒）
     */
    void setAnimationDuration(int duration);

    /**
     * @brief 获取动画持续时间
     */
    int animationDuration() const;

private:
    ThemeManager();
    ~ThemeManager() = default;

    // 禁止拷贝
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    /**
     * @brief 注册内置主题
     */
    void registerBuiltInThemes();

    /**
     * @brief 扫描并注册自定义主题
     */
    void scanCustomThemes();

    /**
     * @brief 查找样式表文件的完整路径
     */
    QString findStyleSheetPath(const QString& fileName);

    /**
     * @brief 应用淡出动画
     */
    void fadeOut(QWidget* widget, std::function<void()> callback);

    /**
     * @brief 应用淡入动画
     */
    void fadeIn(QWidget* widget);

private:
    QList<ThemeInfo> m_themes;
    ThemeInfo m_currentTheme;
    QString m_settingsPath;
    bool m_initialized;
    int m_animationDuration = 200; // 默认动画时长200ms
};

#endif // THEME_MANAGER_H

#ifndef COLORTHEMEMANAGER_H
#define COLORTHEMEMANAGER_H

#include <QColor>
#include <QVector>
#include <QString>
#include <QMap>

namespace Charts {

/**
 * @brief 颜色主题
 */
struct ColorTheme
{
    QString name;
    QString description;
    QVector<QColor> colors;        // 主色调序列
    QColor backgroundColor;
    QColor textColor;
    QColor gridColor;
    QColor highlightColor;         // 高亮色
};

/**
 * @brief 颜色主题管理器
 */
class ColorThemeManager
{
public:
    static ColorThemeManager& instance();

    /**
     * @brief 获取主题
     */
    ColorTheme getTheme(const QString& themeName) const;

    /**
     * @brief 获取所有主题名称
     */
    QVector<QString> themeNames() const;

    /**
     * @brief 设置当前主题
     */
    void setCurrentTheme(const QString& themeName);
    ColorTheme currentTheme() const;

    /**
     * @brief 从主题获取颜色（循环使用）
     */
    QColor getColor(int index) const;
    QColor getColor(int index, const QString& themeName) const;

    /**
     * @brief 加载自定义主题
     */
    bool loadTheme(const QString& filePath);

    /**
     * @brief 保存主题
     */
    bool saveTheme(const ColorTheme& theme, const QString& filePath);

private:
    ColorThemeManager();
    void initializeDefaultThemes();

    QString m_currentTheme;
    QMap<QString, ColorTheme> m_themes;
};

} // namespace Charts

#endif // COLORTHEMEMANAGER_H

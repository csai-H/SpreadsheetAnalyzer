#include "ColorThemeManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Charts {

ColorThemeManager::ColorThemeManager()
{
    initializeDefaultThemes();
    m_currentTheme = "default";
}

ColorThemeManager& ColorThemeManager::instance()
{
    static ColorThemeManager manager;
    return manager;
}

void ColorThemeManager::initializeDefaultThemes()
{
    // 默认主题
    ColorTheme defaultTheme;
    defaultTheme.name = "default";
    defaultTheme.description = "默认主题";
    defaultTheme.colors = {
        QColor(52, 152, 219),    // 蓝色
        QColor(46, 204, 113),    // 绿色
        QColor(155, 89, 182),    // 紫色
        QColor(241, 196, 15),    // 黄色
        QColor(230, 126, 34),    // 橙色
        QColor(231, 76, 60),     // 红色
        QColor(26, 188, 156),    // 青色
        QColor(52, 73, 94)       // 深灰
    };
    defaultTheme.backgroundColor = Qt::white;
    defaultTheme.textColor = Qt::black;
    defaultTheme.gridColor = QColor(200, 200, 200);
    defaultTheme.highlightColor = QColor(231, 76, 60);
    m_themes["default"] = defaultTheme;

    // 深色主题
    ColorTheme darkTheme;
    darkTheme.name = "dark";
    darkTheme.description = "深色主题";
    darkTheme.colors = {
        QColor(52, 152, 219),
        QColor(46, 204, 113),
        QColor(155, 89, 182),
        QColor(241, 196, 15),
        QColor(230, 126, 34),
        QColor(231, 76, 60)
    };
    darkTheme.backgroundColor = QColor(44, 62, 80);
    darkTheme.textColor = Qt::white;
    darkTheme.gridColor = QColor(100, 100, 100);
    darkTheme.highlightColor = QColor(52, 152, 219);
    m_themes["dark"] = darkTheme;

    // 柔和主题
    ColorTheme pastelTheme;
    pastelTheme.name = "pastel";
    pastelTheme.description = "柔和主题";
    pastelTheme.colors = {
        QColor(174, 198, 207),
        QColor(124, 172, 169),
        QColor(126, 157, 168),
        QColor(142, 168, 173),
        QColor(189, 210, 194),
        QColor(212, 234, 201)
    };
    pastelTheme.backgroundColor = QColor(250, 250, 250);
    pastelTheme.textColor = QColor(60, 60, 60);
    pastelTheme.gridColor = QColor(220, 220, 220);
    pastelTheme.highlightColor = QColor(124, 172, 169);
    m_themes["pastel"] = pastelTheme;

    // 鲜艳主题
    ColorTheme vibrantTheme;
    vibrantTheme.name = "vibrant";
    vibrantTheme.description = "鲜艳主题";
    vibrantTheme.colors = {
        QColor(255, 0, 100),     // 亮红
        QColor(0, 255, 150),      // 亮绿
        QColor(0, 150, 255),      // 亮蓝
        QColor(255, 200, 0),      // 亮黄
        QColor(150, 0, 255),      // 亮紫
        QColor(255, 100, 0)       // 亮橙
    };
    vibrantTheme.backgroundColor = QColor(240, 248, 255);
    vibrantTheme.textColor = Qt::black;
    vibrantTheme.gridColor = QColor(180, 200, 220);
    vibrantTheme.highlightColor = QColor(0, 150, 255);
    m_themes["vibrant"] = vibrantTheme;

    // 商务主题
    ColorTheme businessTheme;
    businessTheme.name = "business";
    businessTheme.description = "商务主题";
    businessTheme.colors = {
        QColor(31, 78, 121),      // 深蓝
        QColor(58, 135, 173),     // 中蓝
        QColor(107, 174, 214),    // 浅蓝
        QColor(217, 234, 242),    // 极浅蓝
        QColor(149, 55, 53),      // 深红
        QColor(222, 96, 94)       // 浅红
    };
    businessTheme.backgroundColor = Qt::white;
    businessTheme.textColor = QColor(51, 51, 51);
    businessTheme.gridColor = QColor(220, 220, 220);
    businessTheme.highlightColor = QColor(31, 78, 121);
    m_themes["business"] = businessTheme;
}

ColorTheme ColorThemeManager::getTheme(const QString& themeName) const
{
    return m_themes.value(themeName, m_themes.value("default"));
}

QVector<QString> ColorThemeManager::themeNames() const
{
    return m_themes.keys().toVector();
}

void ColorThemeManager::setCurrentTheme(const QString& themeName)
{
    if (m_themes.contains(themeName)) {
        m_currentTheme = themeName;
    }
}

ColorTheme ColorThemeManager::currentTheme() const
{
    return getTheme(m_currentTheme);
}

QColor ColorThemeManager::getColor(int index) const
{
    ColorTheme theme = currentTheme();
    return theme.colors[index % theme.colors.size()];
}

QColor ColorThemeManager::getColor(int index, const QString& themeName) const
{
    ColorTheme theme = getTheme(themeName);
    return theme.colors[index % theme.colors.size()];
}

bool ColorThemeManager::loadTheme(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject json = doc.object();
    ColorTheme theme;

    theme.name = json["name"].toString();
    theme.description = json["description"].toString();

    // 解析颜色
    QJsonArray colorsArray = json["colors"].toArray();
    for (const auto& colorValue : colorsArray) {
        QJsonObject colorObj = colorValue.toObject();
        int r = colorObj["r"].toInt();
        int g = colorObj["g"].toInt();
        int b = colorObj["b"].toInt();
        theme.colors.append(QColor(r, g, b));
    }

    // 解析背景色
    QJsonObject bgColorObj = json["backgroundColor"].toObject();
    theme.backgroundColor = QColor(bgColorObj["r"].toInt(),
                                 bgColorObj["g"].toInt(),
                                 bgColorObj["b"].toInt());

    // 解析文本色
    QJsonObject textColorObj = json["textColor"].toObject();
    theme.textColor = QColor(textColorObj["r"].toInt(),
                              textColorObj["g"].toInt(),
                              textColorObj["b"].toInt());

    m_themes[theme.name] = theme;
    return true;
}

bool ColorThemeManager::saveTheme(const ColorTheme& theme, const QString& filePath)
{
    QJsonObject json;
    json["name"] = theme.name;
    json["description"] = theme.description;

    // 保存颜色
    QJsonArray colorsArray;
    for (const auto& color : theme.colors) {
        QJsonObject colorObj;
        colorObj["r"] = color.red();
        colorObj["g"] = color.green();
        colorObj["b"] = color.blue();
        colorsArray.append(colorObj);
    }
    json["colors"] = colorsArray;

    // 保存背景色
    QJsonObject bgColorObj;
    bgColorObj["r"] = theme.backgroundColor.red();
    bgColorObj["g"] = theme.backgroundColor.green();
    bgColorObj["b"] = theme.backgroundColor.blue();
    json["backgroundColor"] = bgColorObj;

    // 保存文本色
    QJsonObject textColorObj;
    textColorObj["r"] = theme.textColor.red();
    textColorObj["g"] = theme.textColor.green();
    textColorObj["b"] = theme.textColor.blue();
    json["textColor"] = textColorObj;

    QJsonDocument doc(json);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson());
    file.close();

    return true;
}

} // namespace Charts

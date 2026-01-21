#include "ThemeManager.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>

ThemeManager::ThemeManager()
    : m_initialized(false)
{
    m_settingsPath = QCoreApplication::applicationDirPath() + "/theme.ini";
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::initialize()
{
    if (m_initialized) {
        return;
    }

    registerBuiltInThemes();
    scanCustomThemes();
    loadSavedTheme();

    m_initialized = true;
}

void ThemeManager::registerBuiltInThemes()
{
    // 浅色主题（现代风格）
    m_themes.append(ThemeInfo(
        "modern",
        "浅色主题",
        "现代简洁的浅色主题，适合日常使用",
        "modern.qss",
        ThemeType::Light
    ));

    // 深色主题（护眼风格）
    m_themes.append(ThemeInfo(
        "dark",
        "深色主题",
        "护眼的深色主题，减少眼睛疲劳",
        "dark.qss",
        ThemeType::Dark
    ));
}

void ThemeManager::scanCustomThemes()
{
    QString stylesDir = stylesDirectory();
    QDir dir(stylesDir);

    if (!dir.exists()) {
        qWarning() << "Styles directory does not exist:" << stylesDir;
        return;
    }

    QStringList filters;
    filters << "*.qss";
    dir.setNameFilters(filters);

    QFileInfoList fileList = dir.entryInfoList(QDir::Files);

    for (const QFileInfo& fileInfo : fileList) {
        QString fileName = fileInfo.baseName();

        // 跳过已注册的主题
        bool alreadyRegistered = false;
        for (const ThemeInfo& theme : m_themes) {
            if (theme.name == fileName) {
                alreadyRegistered = true;
                break;
            }
        }

        if (!alreadyRegistered) {
            ThemeInfo customTheme(
                fileName,
                fileName, // 使用文件名作为显示名
                "自定义主题",
                fileInfo.fileName(),
                ThemeType::Light // 默认为浅色
            );
            m_themes.append(customTheme);
        }
    }
}

QList<ThemeInfo> ThemeManager::availableThemes() const
{
    return m_themes;
}

ThemeInfo ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

bool ThemeManager::setTheme(const QString& themeName, bool enableAnimation)
{
    for (const ThemeInfo& theme : m_themes) {
        if (theme.name == themeName) {
            return setTheme(theme, enableAnimation);
        }
    }
    return false;
}

bool ThemeManager::setTheme(const ThemeInfo& theme, bool enableAnimation)
{
    QString fullPath = findStyleSheetPath(theme.styleSheetPath);

    if (fullPath.isEmpty()) {
        qWarning() << "Theme style sheet not found:" << theme.styleSheetPath;
        return false;
    }

    QString styleSheet = loadStyleSheet(fullPath);
    if (styleSheet.isEmpty()) {
        qWarning() << "Failed to load style sheet:" << fullPath;
        return false;
    }

    // 获取主窗口
    QWidget* mainWindow = nullptr;
    for (QWidget* widget : qApp->topLevelWidgets()) {
        if (widget->isWindow() && !widget->windowTitle().isEmpty()) {
            mainWindow = widget;
            break;
        }
    }

    if (enableAnimation && mainWindow) {
        // 带动画的主题切换
        fadeOut(mainWindow, [this, styleSheet, theme, mainWindow]() {
            qApp->setStyleSheet(styleSheet);
            m_currentTheme = theme;
            fadeIn(mainWindow);
            qDebug() << "Theme changed to:" << theme.displayName << "(with animation)";
        });
    } else {
        // 直接应用主题（无动画）
        qApp->setStyleSheet(styleSheet);
        m_currentTheme = theme;
        qDebug() << "Theme changed to:" << theme.displayName;
    }

    return true;
}

QString ThemeManager::loadStyleSheet(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Style sheet file does not exist:" << filePath;
        return QString();
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to open style sheet file:" << filePath;
        return QString();
    }

    QString styleSheet = QString::fromUtf8(file.readAll());
    file.close();

    return styleSheet;
}

void ThemeManager::saveCurrentTheme()
{
    QSettings settings(m_settingsPath, QSettings::IniFormat);
    settings.setValue("theme/name", m_currentTheme.name);
    settings.setValue("theme/type", static_cast<int>(m_currentTheme.type));
    settings.sync();
}

void ThemeManager::loadSavedTheme()
{
    QSettings settings(m_settingsPath, QSettings::IniFormat);
    QString savedThemeName = settings.value("theme/name", "modern").toString();

    // 尝试加载保存的主题（启动时不使用动画）
    if (!savedThemeName.isEmpty()) {
        for (const ThemeInfo& theme : m_themes) {
            if (theme.name == savedThemeName) {
                setTheme(theme, false); // 启动时不使用动画
                return;
            }
        }
    }

    // 如果没有保存的主题或加载失败，使用默认主题
    if (!m_themes.isEmpty()) {
        setTheme(m_themes.first(), false); // 启动时不使用动画
    }
}

QString ThemeManager::stylesDirectory() const
{
    // 尝试多个可能的目录
    QStringList possiblePaths;

    // 1. 可执行文件目录/resources/styles
    possiblePaths << QCoreApplication::applicationDirPath() + "/resources/styles";

    // 2. 从build目录运行时的相对路径
    possiblePaths << QCoreApplication::applicationDirPath() + "/../resources/styles";

    // 3. 当前工作目录/resources/styles
    possiblePaths << QDir::currentPath() + "/resources/styles";

    for (const QString& path : possiblePaths) {
        QDir dir(path);
        if (dir.exists()) {
            return path;
        }
    }

    // 默认返回第一个路径
    return possiblePaths.first();
}

QString ThemeManager::findStyleSheetPath(const QString& fileName)
{
    QString stylesDir = stylesDirectory();
    QString fullPath = stylesDir + "/" + fileName;

    if (QFile::exists(fullPath)) {
        return fullPath;
    }

    qWarning() << "Style sheet file not found:" << fullPath;
    return QString();
}

void ThemeManager::refresh()
{
    if (!m_currentTheme.name.isEmpty()) {
        setTheme(m_currentTheme, false); // 刷新时不使用动画
    }
}

void ThemeManager::setAnimationDuration(int duration)
{
    m_animationDuration = duration;
}

int ThemeManager::animationDuration() const
{
    return m_animationDuration;
}

void ThemeManager::fadeOut(QWidget* widget, std::function<void()> callback)
{
    if (!widget) {
        if (callback) {
            callback();
        }
        return;
    }

    // 创建透明度效果
    auto* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);

    // 创建淡出动画
    auto* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(m_animationDuration);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);

    // 动画结束后执行回调
    QObject::connect(animation, &QPropertyAnimation::finished, [widget, effect, callback]() {
        if (callback) {
            callback();
        }
        // 清理效果
        widget->setGraphicsEffect(nullptr);
        effect->deleteLater();
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ThemeManager::fadeIn(QWidget* widget)
{
    if (!widget) {
        return;
    }

    // 创建透明度效果
    auto* effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);

    // 创建淡入动画
    auto* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(m_animationDuration);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    // 动画结束后清理效果
    QObject::connect(animation, &QPropertyAnimation::finished, [widget, effect]() {
        widget->setGraphicsEffect(nullptr);
        effect->deleteLater();
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

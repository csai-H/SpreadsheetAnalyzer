#ifndef DATALOADER_H
#define DATALOADER_H

#include <QString>
#include <QVariant>
#include <QVector>
#include "../core/TableData.h"

/**
 * @brief 数据加载结果
 */
struct LoadResult
{
    bool success = false;
    QString errorMessage;
    Core::TableData* data = nullptr;

    static LoadResult successResult(Core::TableData* data);
    static LoadResult errorResult(const QString& error);
};

/**
 * @brief 数据加载器抽象接口
 */
class IDataLoader
{
public:
    virtual ~IDataLoader() = default;

    virtual LoadResult load(const QString& filePath) = 0;
    virtual bool supports(const QString& filePath) const = 0;
    virtual QStringList supportedExtensions() const = 0;
};

#endif // DATALOADER_H

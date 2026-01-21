#ifndef CSVLOADER_H
#define CSVLOADER_H

#include "DataLoader.h"
#include <QChar>
#include <QString>

/**
 * @brief CSV 文件加载器
 */
class CsvLoader : public IDataLoader
{
public:
    explicit CsvLoader(QObject* parent = nullptr);
    ~CsvLoader() override;

    LoadResult load(const QString& filePath) override;
    bool supports(const QString& filePath) const override;
    QStringList supportedExtensions() const override;

    void setDelimiter(const QChar& delimiter);
    void setHasHeader(bool hasHeader);
    void setTrimWhitespace(bool trim);
    void setEncoding(const QString& encoding);

private:
    QStringList parseCsvLine(const QString& line);

    struct Impl;
    Impl* m_impl;
};

#endif // CSVLOADER_H

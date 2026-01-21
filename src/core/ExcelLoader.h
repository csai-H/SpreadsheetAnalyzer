#ifndef EXCELLOADER_H
#define EXCELLOADER_H

#include "DataLoader.h"
#include <QString>

class ExcelLoader : public IDataLoader
{
public:
    explicit ExcelLoader(QObject* parent = nullptr);
    ~ExcelLoader() override;

    LoadResult load(const QString& filePath) override;
    bool supports(const QString& filePath) const override;
    QStringList supportedExtensions() const override;

    void setSheetName(const QString& sheetName);
    void setSheetIndex(int index);
    void setHasHeader(bool hasHeader);

private:
    struct Impl;
    Impl* m_impl;
};

#endif // EXCELLOADER_H

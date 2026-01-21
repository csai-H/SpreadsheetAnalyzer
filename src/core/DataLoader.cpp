#include "DataLoader.h"

LoadResult LoadResult::successResult(Core::TableData* data)
{
    LoadResult result;
    result.success = true;
    result.data = data;
    return result;
}

LoadResult LoadResult::errorResult(const QString& error)
{
    LoadResult result;
    result.success = false;
    result.errorMessage = error;
    return result;
}

#ifndef SQLADAPTER_H
#define SQLADAPTER_H

#include "IAdapter.h"
#include "SqlAdaptee.h"
#include <memory>

class SqlAdapter : public IAdapter
{

public:
    explicit SqlAdapter(std::shared_ptr<SqlAdaptee> adaptee);

    // Реализация Target (IAdapter)
    DataSet loadData(const QString& filePath) override;
    bool canLoad(const QString& filePath) const override;
    QString getType() const override;

private:
    std::shared_ptr<SqlAdaptee> m_adaptee;
};

#endif // SQLADAPTER_H

#ifndef JSONADAPTER_H
#define JSONADAPTER_H


#include "IAdapter.h"
#include "JsonAdaptee.h"
#include <memory>

class JsonAdapter : public IAdapter {  // ← наследует Target!

public:
    explicit JsonAdapter(std::shared_ptr<JsonAdaptee> adaptee);

    // Реализация Target (IAdapter)
    DataSet loadData(const QString& filePath) override;
    bool canLoad(const QString& filePath) const override;
    QString getType() const override;

private:
    std::shared_ptr<JsonAdaptee> m_adaptee;  // ← содержит Adaptee!

};


#endif // JSONADAPTER_H

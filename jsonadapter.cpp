#include "jsonadapter.h"

JsonAdapter::JsonAdapter(std::shared_ptr<JsonAdaptee> adaptee)
    : m_adaptee(adaptee)
{
}

DataSet JsonAdapter::loadData(const QString& filePath)
{
    // Адаптер вызывает специфический метод Adaptee!
    return m_adaptee->loadFromJson(filePath);
}

bool JsonAdapter::canLoad(const QString& filePath) const
{
    return m_adaptee->canLoad(filePath);
}

QString JsonAdapter::getType() const
{
    return "JSONAdapter";
}

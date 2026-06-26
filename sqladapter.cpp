#include "sqladapter.h"

SqlAdapter::SqlAdapter(std::shared_ptr<SqlAdaptee> adaptee)
    : m_adaptee(adaptee)
{
}

bool SqlAdapter::canLoad(const QString& filePath) const
{
   return m_adaptee->canLoad(filePath);
}

DataSet SqlAdapter::loadData(const QString& filePath)
{
    // Адаптер вызывает специфический метод Adaptee!
    return m_adaptee->loadFromDatabase(filePath);
}

QString SqlAdapter::getType() const
{
    return "SQLiteAdapter";
}

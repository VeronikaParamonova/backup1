#ifndef IADAPTER_H
#define IADAPTER_H


#include <QString>
#include "modeldata.h"

class IAdapter {
public:
    virtual ~IAdapter() = default;

    // Загружает данные из файла
    virtual DataSet loadData(const QString& filePath) = 0;

    // Проверяет, может ли загрузчик обработать этот файл
    virtual bool canLoad(const QString& filePath) const = 0;

    // Возвращает тип загрузчика (для отладки и логирования)
    virtual QString getType() const = 0;

    // Проверяет, валидны ли данные
    virtual bool isValid() const { return true; }
};

#endif // IADAPTER_H

#ifndef JSONADAPTEE_H
#define JSONADAPTEE_H

#include <QObject>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QJsonDocument>
#include "modeldata.h"

class JsonAdaptee : public QObject
{
    Q_OBJECT

public:
    explicit JsonAdaptee(QObject* parent = nullptr);

    DataSet loadFromJson(const QString& filePath);
    bool canLoad(const QString& filePath) const;
    QString getType() const  { return "JSON"; }

signals:
    void logInfo(const QString& message);
    void logError(const QString& message);

private:
    bool m_isValid;

    DataSet parseArrayFormat(const QJsonArray& array, const QString& filePath);
    DataSet parseObjectFormat(const QJsonObject& object, const QString& filePath);
    DataSet parseKeyValueFormat(const QJsonArray& array);
    QDateTime parseDateTime(const QString& str);
};

#endif // JSONADAPTEE_H

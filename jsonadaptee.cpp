#include "jsonadaptee.h"

JsonAdaptee::JsonAdaptee(QObject* parent) : QObject(parent), m_isValid(false){}

bool JsonAdaptee::canLoad(const QString& filePath) const
{
    return filePath.endsWith(".json", Qt::CaseInsensitive);
}

DataSet JsonAdaptee::loadFromJson(const QString& filePath)
{
    DataSet result;
    result.sourcePath = filePath;
    result.sourceType = getType();

    emit logInfo("Loading JSON file: " + filePath);


    if (!QFile::exists(filePath))
    {
        emit logError("File not found: " + filePath);
        return result;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        emit logError("Cannot open file: " + filePath);
        return result;
    }

    QByteArray data = file.readAll();
    file.close();


    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit logError("JSON parse error: " + parseError.errorString());
        return result;
    }

    m_isValid = true;

    // Определяем формат JSON и парсим
    if (doc.isArray())
    {
        result = parseArrayFormat(doc.array(), filePath);
    }
    else if (doc.isObject())
    {
        result = parseObjectFormat(doc.object(), filePath);
    }
    else
    {
        emit logError("Unsupported JSON format");
    }

    if (!result.isEmpty())
    {
        emit logInfo("Loaded " + QString::number(result.points.size()) + " data points");
    }

    return result;
}

DataSet JsonAdaptee::parseArrayFormat(const QJsonArray& array, const QString& filePath)
{
    DataSet result;
    result.sourcePath = filePath;
    result.sourceType = getType();
    result.name = "Data from JSON";

    // Если каждый элемент — массив [date, value]
    for (const QJsonValue& val : array)
    {
        if (val.isArray())
        {// Формат: [[date, value], [date, value], ...]
            QJsonArray item = val.toArray();
            if (item.size() >= 2)
            {
                QString dateStr = item[0].toString();
                double value = item[1].toDouble();
                QDateTime dt = parseDateTime(dateStr);
                if (dt.isValid())
                {
                    result.points.append(PointData(dt, value));
                }
            }
        }
        else if (val.isObject())
        {   // Если элемент — объект с полями date и value
            // Формат: [{"date": "...", "value": ...}, ...]
            QJsonObject obj = val.toObject();
            PointData point;
            if (obj.contains("date") || obj.contains("Date"))
            {
                QString key = obj.contains("date") ? "date" : "Date";
                point.date = parseDateTime(obj[key].toString());
            }
            if (obj.contains("value") || obj.contains("Value"))
            {
                QString key = obj.contains("value") ? "value" : "Value";
                point.value = obj[key].toDouble();
            }
            if (point.date.isValid())
            {
                result.points.append(point);
            }
        }
    }

    return result;
}

DataSet JsonAdaptee::parseObjectFormat(const QJsonObject& object, const QString& filePath)
{
    DataSet result;
    result.sourcePath = filePath;
    result.sourceType = getType();

    // Пробуем найти массив данных по ключам
    QStringList possibleKeys = {"data", "values", "points", "series", "Data", "Values", "Points", "Series"};

    for (const QString& key : possibleKeys)
    {
        if (object.contains(key) && object[key].isArray())
        {
            result.name = key;
            DataSet temp = parseKeyValueFormat(object[key].toArray());
            result.points = temp.points;
            if (!result.isEmpty())
            {
                return result;
            }
        }
    }

    // Если имя не найдено, пробуем взять из поля "name"
    if (object.contains("name")) {
        result.name = object["name"].toString();
    }

    // Ищем массив данных по ключу
    for (auto it = object.begin(); it != object.end(); ++it)
    {
        if (it.value().isArray())
        {
            DataSet temp = parseKeyValueFormat(it.value().toArray());
            if (!temp.isEmpty())
            {
                result.points = temp.points;
                if (result.name.isEmpty())
                {
                    result.name = it.key();
                }
                break;
            }
        }
    }

    return result;
}

DataSet JsonAdaptee::parseKeyValueFormat(const QJsonArray& array)
{
    DataSet result;

    for (const QJsonValue& val : array) {
        if (val.isObject())
        {
            QJsonObject obj = val.toObject();
            PointData point;

            // Ищем дату
            QString dateKey = obj.contains("date") ? "date" : (obj.contains("Date") ? "Date" : "");
            if (!dateKey.isEmpty())
            {
                point.date = parseDateTime(obj[dateKey].toString());
            }

            // Ищем значение
            QString valueKey = obj.contains("value") ? "value" : (obj.contains("Value") ? "Value" : "");
            if (!valueKey.isEmpty())
            {
                point.value = obj[valueKey].toDouble();
            }

            if (point.date.isValid())
            {
                result.points.append(point);
            }
        }
    }

    return result;
}

QDateTime JsonAdaptee::parseDateTime(const QString& str)
{
    QDateTime dt;

    // Пробуем разные форматы
    QStringList formats =
        {
        "yyyy-MM-dd HH:mm:ss",
        "yyyy-MM-dd",
        "dd.MM.yyyy HH:mm:ss",
        "dd.MM.yyyy",
        "MM/dd/yyyy HH:mm:ss",
        "MM/dd/yyyy",
        "yyyy-MM-ddTHH:mm:ss",
        "yyyy-MM-ddTHH:mm:ssZ"
        };

    for (const QString& format : formats)
    {
        dt = QDateTime::fromString(str, format);
        if (dt.isValid())
        {
            break;
        }
    }

    return dt;
}

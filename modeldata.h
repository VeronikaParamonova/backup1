#ifndef MODELDATA_H
#define MODELDATA_H

#include <QDateTime>
#include <QString>
#include <QList>

// Структура для хранения одной точки данных
struct PointData {
    QDateTime date;   // Дата и время
    double value;     // Значение

    PointData() : date(QDateTime()), value(0.0) {}
    PointData(const QDateTime& d, double v) : date(d), value(v) {}
};

// Структура для хранения набора данных
struct DataSet {
    QString name;               // Название набора данных (например, "Temperature")
    QList<PointData> points;    // Точки данных
    QString sourceType;         // Тип источника ("SQLite" или "JSON")
    QString sourcePath;         // Путь к файлу

    DataSet() {}
    DataSet(const QString& n, const QList<PointData>& p, const QString& type, const QString& path)
        : name(n), points(p), sourceType(type), sourcePath(path) {}

    bool isEmpty() const { return points.isEmpty(); }
    int size() const { return points.size(); }
};

#endif // MODELDATA_H

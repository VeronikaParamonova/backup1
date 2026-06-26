#ifndef SQLADAPTEE_H
#define SQLADAPTEE_H

#include <QObject>
#include <QtSql>
#include <QDebug>
#include <QFile>

#include "modeldata.h"

class SqlAdaptee : public QObject
{
    Q_OBJECT

public:
    explicit SqlAdaptee(QObject* parent = nullptr);
    ~SqlAdaptee();

    DataSet loadFromDatabase(const QString& filePath) ;
    bool canLoad(const QString& filePath) const ;
    QString getType() const  { return "SQLite"; }

signals:
    void logInfo(const QString& message);
    void logError(const QString& message);

private:
    QSqlDatabase m_db;
    bool m_isValid;
    QString m_connectionName;

    bool openDatabase(const QString& filePath);
    void closeDatabase();
    QList<PointData> parseQueryResult(QSqlQuery& query);
    QString detectTableName(QSqlQuery& query);
};

#endif // SQLADAPTEE_H

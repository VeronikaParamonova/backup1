
#include "sqladaptee.h"
#include <QFile>
#include <QDateTime>
#include <QDebug>

SqlAdaptee::SqlAdaptee(QObject* parent) : QObject(parent), m_isValid(false)
{
    // Создаём уникальное имя для соединения с БД
    static int counter = 0;
    m_connectionName = "sqlite_conn_" + QString::number(++counter);
    // Регистрируем драйвер SQLite
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);//первый аргумент - идентификатор драйвер СУБД, второй - имя подключения
}

SqlAdaptee::~SqlAdaptee()
{
    closeDatabase();
    // Удаляем подключение
    if (!m_connectionName.isEmpty())
    {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool SqlAdaptee::canLoad(const QString& filePath) const
{
    // Проверяем расширение файла
    return (filePath.endsWith(".db", Qt::CaseInsensitive) || filePath.endsWith(".sqlite", Qt::CaseInsensitive) || filePath.endsWith(".sqlite3", Qt::CaseInsensitive)); //без учёта регистра

}

DataSet SqlAdaptee::loadFromDatabase(const QString& filePath)
{

    DataSet result;
    result.sourcePath = filePath;
    result.sourceType = getType();

    emit logInfo("Loading SQLite database: " + filePath);

    // Проверяем существование файла
    if (!QFile::exists(filePath))
    {
        emit logError("File not found: " + filePath);
        return result;
    }

    // Открываем базу данных
    if (!openDatabase(filePath))
    {
        emit logError("Failed to open database: " + filePath);
        return result;
    }

    // Получаем список таблиц
    QSqlQuery tableQuery(m_db);
    QStringList tables;

    if (tableQuery.exec("SELECT name FROM sqlite_master WHERE type='table'"))
    {
        while (tableQuery.next()) {
            QString tableName = tableQuery.value(0).toString();
            // Пропускаем служебные таблицы SQLite
            if (!tableName.startsWith("sqlite_")) {
                tables << tableName;
            }
        }
    }

    if (tables.isEmpty()) {
        emit logError("No tables found in database");
        closeDatabase();
        return result;
    }

    // Берём первую таблицу (или ищем по имени, если знаем)
    QString tableName = tables.first();

    //Определяем столбцы для даты и значения
    QString dateColumn, valueColumn;

    // Получаем информацию о столбцах
    QSqlQuery columnQuery(m_db);
    columnQuery.exec("PRAGMA table_info(" + tableName + ")");

    QList<QPair<QString, QString>> columns; // name, type
    while (columnQuery.next())
    {
        QString colName = columnQuery.value(1).toString();
        QString colType = columnQuery.value(2).toString().toUpper();
        columns << qMakePair(colName, colType);
    }

    // Ищем столбцы для даты и значения
    for (const auto& col : columns)
    {
        QString name = col.first.toLower();
        QString type = col.second;

        // Ищем столбец для даты
        if (dateColumn.isEmpty() &&
            (name.contains("date") || name.contains("time") ||
             name.contains("datetime") || type.contains("DATE") ||
             type.contains("DATETIME") || type.contains("TIMESTAMP"))) {
            dateColumn = col.first;
        }

        // Ищем столбец для значения
        if (valueColumn.isEmpty() &&
            (name.contains("value") || name.contains("temp") ||
             name.contains("data") || name.contains("val") ||
             type.contains("REAL") || type.contains("INTEGER") ||
             type.contains("NUMERIC") || type.contains("FLOAT") ||
             type.contains("DOUBLE"))) {
            valueColumn = col.first;
        }
    }

    // Если не нашли столбцы — используем первые два
    if (dateColumn.isEmpty() && columns.size() >= 1) {
        dateColumn = columns[0].first;
    }
    if (valueColumn.isEmpty() && columns.size() >= 2) {
        valueColumn = columns[1].first;
    }

    emit logInfo("Using table: " + tableName + ", date: " + dateColumn + ", value: " + valueColumn);

    // Формируем запрос
    QString queryStr = QString("SELECT %1, %2 FROM %3")
                           .arg(dateColumn)
                           .arg(valueColumn)
                           .arg(tableName);

    // Если есть ограничение по количеству записей (например, для тестирования)
    // queryStr += " LIMIT 1000";

    QSqlQuery query(m_db);
    if (!query.exec(queryStr)) {
        emit logError("Query failed: " + query.lastError().text());
        closeDatabase();
        return result;
    }

    // Парсим результаты
    QList<PointData> points = parseQueryResult(query);

    if (points.isEmpty())
    {
        emit logError("No data found in table: " + tableName);
    }
    else
    {
        result.name = tableName;
        result.points = points;
        emit logInfo("Loaded " + QString::number(points.size()) + " data points");
    }

    closeDatabase();
    return result;
}

bool SqlAdaptee::openDatabase(const QString& filePath)
{
    if (m_db.isOpen())
    {
        closeDatabase();
    }

    m_db.setDatabaseName(filePath);

    if (!m_db.open())
    {
        emit logError("Error opening database: " + m_db.lastError().text());
        m_isValid = false;
        return false;
    }

    m_isValid = true;
    return true;
}

void SqlAdaptee::closeDatabase()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
    m_isValid = false;
}

QList<PointData> SqlAdaptee::parseQueryResult(QSqlQuery& query)
{
    QList<PointData> points;

    while (query.next()) {
        QDateTime dateTime;
        double value = 0.0;

        // Пробуем получить дату
        QVariant dateVar = query.value(0);
        if (dateVar.canConvert<QDateTime>())
        {
            dateTime = dateVar.toDateTime();
        }
        else if (dateVar.canConvert<QString>())
        {
            // Пробуем парсить строку как дату
            QString dateStr = dateVar.toString();
            dateTime = QDateTime::fromString(dateStr, Qt::ISODate);
            if (!dateTime.isValid())
            {
                dateTime = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
            }
            if (!dateTime.isValid()) {
                dateTime = QDateTime::fromString(dateStr, "yyyy-MM-dd");
            }
        }
        else if (dateVar.canConvert<qlonglong>())
        {
            // Unix timestamp
            dateTime = QDateTime::fromMSecsSinceEpoch(dateVar.toLongLong() * 1000);
        }

        // Получаем значение
        QVariant valueVar = query.value(1);
        if (valueVar.canConvert<double>())
        {
            value = valueVar.toDouble();
        }
        else if (valueVar.canConvert<QString>())
        {
            value = valueVar.toString().toDouble();
        }

        // Если дата не валидна, используем порядковый номер
        if (!dateTime.isValid())
        {
            dateTime = QDateTime::currentDateTime().addSecs(points.size() * 3600);
        }

        points.append(PointData(dateTime, value));
    }

    return points;
}

QString SqlAdaptee::detectTableName(QSqlQuery& query)
{
    // Не используется в текущей реализации
    return "";
}

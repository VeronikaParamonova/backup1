#include <QCoreApplication>//для прил c GUI
#include <QApplication> //для прил без GUI
#include <QDebug>
#include <memory>
#include "ModelData.h"
#include "IAdapter.h"
#include "SqlAdaptee.h"
#include "JsonAdaptee.h"
#include "SqlAdapter.h"
#include "JsonAdapter.h"
#include "ioccontainer.h"
#include <QFileInfo>
#include <QStringList>
#include "mainwindow.h"


// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================



void printSeparator(const QString& title = "")
{
    qDebug().noquote() << "\n" << QString(70, '=');
    if (!title.isEmpty()) {
        qDebug().noquote() << "  " << title;
        qDebug().noquote() << QString(70, '-');
    }
}

// ============================================================
// ПОЛНАЯ printDataSet (с выводом данных)
// ============================================================

void printDataSet(const DataSet& data, const QString& title = "Data")
{
    if (data.isEmpty()) {
        qDebug().noquote() << "  No data loaded!";
        return;
    }

    qDebug().noquote() << "\n " << title;
    qDebug().noquote() << "   Name:        " << data.name;
    qDebug().noquote() << "   Source type: " << data.sourceType;
    qDebug().noquote() << "   Source path: " << data.sourcePath;
    qDebug().noquote() << "   Points:      " << data.size();

    // Статистика
    if (data.size() > 0) {
        double minVal = data.points[0].value;
        double maxVal = data.points[0].value;
        double sum = 0;

        for (const auto& p : data.points) {
            minVal = qMin(minVal, p.value);
            maxVal = qMax(maxVal, p.value);
            sum += p.value;
        }

        qDebug().noquote() << "   Min:         " << minVal;
        qDebug().noquote() << "   Max:         " << maxVal;
        qDebug().noquote() << "   Avg:         " << (sum / data.size());
    }

    // Первые 5 точек
    int previewCount = qMin(5, data.size());
    qDebug().noquote() << "\n   First " << previewCount << " points:";

    for (int i = 0; i < previewCount; ++i) {
        const PointData& p = data.points[i];
        int index = i + 1;
        QString dateStr = p.date.toString("yyyy-MM-dd HH:mm:ss");
        QString valueStr = QString::number(p.value, 'f', 2);

        qDebug().noquote() << QString("     [%1] %2 → %3")
                              .arg(index, 2)
                              .arg(dateStr)
                              .arg(valueStr);
    }

    // Последние 5 точек (если их больше 10)
    if (data.size() >= 10) {
        qDebug().noquote() << "     ...";

        int start = data.size() - 5;
        qDebug().noquote() << "\n   Last 5 points:";

        for (int i = start; i < data.size(); ++i) {
            const PointData& p = data.points[i];
            int index = i + 1;
            QString dateStr = p.date.toString("yyyy-MM-dd HH:mm:ss");
            QString valueStr = QString::number(p.value, 'f', 2);

            qDebug().noquote() << QString("     [%1] %2 → %3")
                                  .arg(index, 2)
                                  .arg(dateStr)
                                  .arg(valueStr);
        }
    }
}

// ============================================================
// MAIN
// ============================================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto container = std::make_shared<IOCContainer>();  // ← создаём shared_ptr

    // Регистрируем Adaptee (синглтоны)
    container->RegisterInstance<SqlAdaptee>(std::make_shared<SqlAdaptee>());
    container->RegisterInstance<JsonAdaptee>(std::make_shared<JsonAdaptee>());

     // Регистрируем Adapter (конкретные адаптеры)
    container->RegisterFactory<SqlAdapter, SqlAdapter, SqlAdaptee>();
    container->RegisterFactory<JsonAdapter, JsonAdapter, JsonAdaptee>();

    // Регистрируем ОДИН адаптер для IAdapter
    container->RegisterAdapter<IAdapter>(
        // Условие: принимает ЛЮБОЙ файл, который мы поддерживаем
        [](const QString& path) {
            return path.endsWith(".db") ||
                   path.endsWith(".sqlite") ||
                   path.endsWith(".sqlite3") ||
                   path.endsWith(".json");
        },
        // Фабрика: выбирает адаптер по расширению файла
        [&container](const QString& path) -> std::shared_ptr<IAdapter> {
            if (path.endsWith(".db") || path.endsWith(".sqlite") || path.endsWith(".sqlite3")) {
                return std::static_pointer_cast<IAdapter>(container->GetObject<SqlAdapter>());
            } else if (path.endsWith(".json")) {
                return std::static_pointer_cast<IAdapter>(container->GetObject<JsonAdapter>());
            }
            return nullptr;
        }
        );

    MainWindow window(container);
    window.show();

    return app.exec();
//QCoreApplication app(argc, argv);

//    qDebug().noquote() << "\n========================================";
//    qDebug().noquote() << "STARTING TEST";
//    qDebug().noquote() << "========================================";

//    IOCContainer container;

//    container.RegisterInstance<SqlAdaptee>(std::make_shared<SqlAdaptee>());
//    container.RegisterInstance<JsonAdaptee>(std::make_shared<JsonAdaptee>());

//    container.RegisterFactory<SqlAdapter, SqlAdapter, SqlAdaptee>();
//    container.RegisterFactory<JsonAdapter, JsonAdapter, JsonAdaptee>();


//    // Регистрируем ОДИН адаптер для IAdapter
//    container.RegisterAdapter<IAdapter>(
//        // Условие: принимает ЛЮБОЙ файл, который мы поддерживаем
//        [](const QString& path) {
//            return path.endsWith(".db") ||
//                   path.endsWith(".sqlite") ||
//                   path.endsWith(".sqlite3") ||
//                   path.endsWith(".json");
//        },
//        // Фабрика: выбирает адаптер по расширению файла
//        [&container](const QString& path) -> std::shared_ptr<IAdapter> {
//            if (path.endsWith(".db") || path.endsWith(".sqlite") || path.endsWith(".sqlite3")) {
//                return std::static_pointer_cast<IAdapter>(container.GetObject<SqlAdapter>());
//            } else if (path.endsWith(".json")) {
//                return std::static_pointer_cast<IAdapter>(container.GetObject<JsonAdapter>());
//            }
//            return nullptr;
//        }
//        );




//    QStringList testFiles = {
//        "C:/Users/User/Downloads/HUMIDITY_MOSCOW.sqlite",
//        "C:/Users/User/Downloads/BLOOD_SUGAR.sqlite",
//        "C:/Users/User/Downloads/NORDPOOL_PRICES.sqlite",
//        "C:/Users/User/Downloads/PRICES_NATURAL_GAS_USD.sqlite",
//        "C:/Users/User/Downloads/TEMPERATURE_NOVOSIB.sqlite",
//        "C:/Users/User/Desktop/TRPO/Graphs/big_time.json",
//        "C:/Users/User/Desktop/TRPO/Graphs/mas_mas.json",
//        "C:/Users/User/Desktop/TRPO/Graphs/mas_obj.json",
//        "C:/Users/User/Desktop/TRPO/Graphs/obj_masiv_meta.json"
//    };

//    for (const QString& filePath : testFiles) {
//        printSeparator("Testing: " + QFileInfo(filePath).fileName());

//        if (!QFile::exists(filePath)) {
//            qDebug().noquote() << "File not found:" << filePath;
//            continue;
//        }

//        qDebug().noquote() << "File exists, calling GetObject<IAdapter>(" << filePath << ")...";

//        auto adapter = container.GetObject<IAdapter>(filePath);

//        qDebug().noquote() << "Adapter obtained:" << (adapter ? "valid" : "null");

//        if (!adapter) {
//            qDebug().noquote() << "No adapter found for:" << filePath;
//            continue;
//        }

//        qDebug().noquote() << "Using: " << adapter->getType();

//        qDebug().noquote() << "Calling loadData...";
//        DataSet data = adapter->loadData(filePath);
//        qDebug().noquote() << "loadData finished, points:" << data.size();

//        printDataSet(data, "Loaded Data");
//    }

//    printSeparator(" TEST COMPLETE");

//    return 0;
}

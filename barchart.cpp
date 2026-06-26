#include "barchart.h"

QT_CHARTS_USE_NAMESPACE

    QChartView* BarChart::createChart(const DataSet& data)
{
    if (data.isEmpty()) {
        return new QChartView();
    }

    // 1. Определяем, сколько точек показывать
    int maxPoints = 50;  // ← максимум столбцов на графике
    int step = qMax(1, data.size() / maxPoints);

    // 2. Создаём набор столбцов
    QBarSet* barSet = new QBarSet("Значения");

    // 3. Добавляем данные с шагом
    for (int i = 0; i < data.size(); i += step) {
        *barSet << data.points[i].value;
    }

    // 4. Создаём серию
    QBarSeries* series = new QBarSeries();
    series->append(barSet);

    // 5. Создаём ОСИ С ДАТАМИ
    QStringList categories;

    // ← ДЛЯ МАЛЕНЬКИХ ФАЙЛОВ (≤ 50 точек) — ПОКАЗЫВАЕМ ВСЕ
    if (data.size() <= 50) {
        for (const PointData& point : data.points) {
            categories << point.date.toString("dd.MM");
        }
    } else {
        // ← ДЛЯ БОЛЬШИХ — КАЖДУЮ N-Ю
        int labelStep = qMax(1, data.size() / 20);  // ~20 подписей
        for (int i = 0; i < data.size(); i += step) {
            if (i % labelStep == 0 || i == 0) {
                categories << data.points[i].date.toString("dd.MM");
            } else {
                categories << "";  // ← пустая подпись
            }
        }
    }

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Дата");

    // ← ПОВОРАЧИВАЕМ, ЕСЛИ МНОГО ПОДПИСЕЙ
    if (categories.size() > 10) {
        axisX->setLabelsAngle(90);
    }

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Значение");

    // 6. Создаём график
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(data.name);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    return chartView;
}


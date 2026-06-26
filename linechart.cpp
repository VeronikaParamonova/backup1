#include "linechart.h"

QT_CHARTS_USE_NAMESPACE

QChartView* LineChart::createChart(const DataSet& data)
{
    if (data.isEmpty()) {
        return new QChartView();
    }

    // 1. Создаём серию точек
    QLineSeries* series = new QLineSeries();
    series->setName("Значения");

    // 2. Добавляем данные (с шагом для больших файлов)
    int maxPoints = 1000;  // ← максимум точек на графике
    int step = qMax(1, data.size() / maxPoints);

    for (int i = 0; i < data.size(); i += step) {
        series->append(i, data.points[i].value);
    }

    // 3. Создаём оси
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Индекс (время)");
    axisX->setLabelFormat("%d");

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Значение");

    // 4. Создаём график
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

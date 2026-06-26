#ifndef LINECHART_H
#define LINECHART_H

#include "ichart.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

QT_CHARTS_USE_NAMESPACE

    class LineChart : public IChart
{
public:
    QChartView* createChart(const DataSet& data) override;
    QString getType() const override { return "LineChart"; }
};

#endif // LINECHART_H

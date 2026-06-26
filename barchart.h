#ifndef BARCHART_H
#define BARCHART_H

#include "ichart.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

    class BarChart : public IChart
{
public:
    QChartView* createChart(const DataSet& data) override;
    QString getType() const override { return "BarChart"; }
};

#endif // BARCHART_H

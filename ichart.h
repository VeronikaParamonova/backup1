#ifndef ICHART_H
#define ICHART_H

#include <QtCharts/QChartView>
#include "ModelData.h"

class IChart
{
public:
    virtual ~IChart() = default;

    // Создаёт график на основе данных
    virtual QtCharts::QChartView* createChart(const DataSet& data) = 0;

    // Возвращает название типа графика
    virtual QString getType() const = 0;
};


#endif // ICHART_H

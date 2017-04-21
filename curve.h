#ifndef CURVE_H
#define CURVE_H

#include "chartview.h"
#include <QString>
#include <QColor>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QColor>

class Curve : public QObject
{

    enum CurveType
    {
        FLOAT32 =0,
        UINT32  =1,
        INT32   =2,
        UINT16  =3,
        INT16   =4,
        UINT8   =5,
        INT8    =6
    };

public:

    // Fields
    QString fn;
    QString cname;
    int type;
    QColor color;
    bool displayed;
    QtCharts::QLineSeries* fullseries;
    QtCharts::QLineSeries* displayseries;
    int display_width = 0;
    int idx;
    int xoffset = 0;
    int yoffset = 0;

    // Constructor
    Curve(int id);

    // Destructor
    ~Curve();

    // Function
    QtCharts::QLineSeries*  getFullSeries();
    QtCharts::QLineSeries*  getDisplaySeries();
    QtCharts::QLineSeries*  getSubSeries(int xmin, int xmax);
    QList<QPointF> downsample_minmax(float *data, int factor, int nbpoints);

    QColor getColor();

    bool isLoaded();

    void setColor(QColor c);

    int length();

    void resetFullSeries();
    void resetDisplaySeries();
    void updateDisplaySeries(int width, float zoomfactor);
    void updateDisplaySeries();
    void shift(int offset);

private:
    float *getrawdata(int *length);

public slots:
    void curve_clicked(QPointF pt);

};

#endif // CURVE_H

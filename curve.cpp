#include "curve.h"
#include "scatool.h"
#include <assert.h>
#include <QLineSeries>

Curve::Curve(int id) :
    QObject()
{
    this->fn = "";
    this->displayed = false;
    this->idx = id;
    this->series = 0;
}

Curve::~Curve()
{
    if (this->series != 0)
    {
        this->series->clear();
        delete this->series;
    }
}

QColor Curve::getColor()
{
    if (this->series)
    {
        return this->series->color();
    }
    else
    {
        QColor *gray = new QColor(Qt::gray);
        return *gray;
    }
}

QtCharts::QLineSeries* Curve::getSeries()
{
    if (this->series)
    {
        return this->series;
    }
    else
    {
        // get size
        FILE *file = fopen(fn.toLatin1().data(),"rb");
        assert(file);

        std::fseek(file,0,SEEK_END);
        int nbpoints = ftell(file)>>2;
        std::fseek(file,0,0);

        float *data;
        // allocate data memory
        data = (float *)malloc(nbpoints<<2);
        // read data
        std::fread(data,sizeof(float),nbpoints,file);

        fclose(file);

        QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
        /*int width = ScaTool::main_plot->chart()->windowFrameRect().width();
        printf("width %d\n",width);
        float dmin = data[0];
        float dmax = data[0];
        int factor = nbpoints/width;
        for (int i = 0; i < nbpoints; i ++)
        {
            if (i%factor == 0)
            {
                series->append(1+(i/factor),dmin);
                series->append(1+(i/factor),dmax);
                dmin = std::numeric_limits<float>::max();
                dmax = std::numeric_limits<float>::min();
                continue;
            }
            dmin = std::min(data[i],dmin);
            dmax = std::max(data[i],dmax);

        }*/
        for (int i = 0; i < nbpoints; i ++)
        {
            series->append(i,data[i]);
        }


        free(data);

        // Customize series
        series->setUseOpenGL(true);
        series->setPointsVisible();

        // handler
        QObject::connect(series,&QLineSeries::clicked,this,&Curve::curve_clicked);
        QObject::connect(series,&QLineSeries::hovered,this,&Curve::curve_clicked);
        this->series = series;
        return series;
    }
}

void Curve::curve_clicked(QPointF pt)
{
    ScaTool::statusbar->showMessage("Curve coordinate: (" + QString::number(pt.x()) + ", " + QString::number(pt.y()) + ")",0);
}

void Curve::curve_hovered(QPointF pt, bool b)
{

}

void Curve::shift(int offset)
{
    this->xoffset += offset;

    QLineSeries * cseries = getSeries();

    QList<QPointF> *ptlist = new QList<QPointF>;

    for (int i = 0 ; i < cseries->count(); i ++)
    {
        QPointF pt = cseries->at(i);
        pt.rx() += offset;
        ptlist->append(pt);
    }
    cseries->replace(*ptlist);
}


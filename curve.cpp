#include "curve.h"
#include "scatool.h"
#include <assert.h>
#include <QLineSeries>

Curve::Curve(int id) :
    QObject()
{
    this->fn = "";
    this->cname = "";
    this->displayed = false;
    this->idx = id;
    this->fullseries = 0;
    this->displayseries = 0;
}

Curve::~Curve()
{
    if (this->fullseries != 0)
        resetFullSeries();
    if (this->displayseries != 0)
        resetDisplaySeries();
}

QColor Curve::getColor()
{
    if (this->displayseries)
    {
        return this->displayseries->color();
    }
    else
    {
        QColor *gray = new QColor(Qt::gray);
        return *gray;
    }
}

void Curve::resetFullSeries()
{
    this->fullseries->clear();
    delete this->fullseries;
    this->fullseries = 0;
}

void Curve::resetDisplaySeries()
{
    this->displayseries->clear();
    delete this->displayseries;
    this->displayseries = 0;
}

QtCharts::QLineSeries* Curve::getFullSeries()
{
    if (this->fullseries)
    {
        return this->fullseries;
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

        QtCharts::QLineSeries* fullseries = new QtCharts::QLineSeries();

        for (int i = 0; i < nbpoints; i ++)
        {
            fullseries->append(i,data[i]);
        }


        free(data);

        this->fullseries = fullseries;
        return fullseries;
    }
}

void Curve::updateDisplaySeries(int width, float zoomfactor, int xmin, int xmax)
{
    // get size
    FILE *file = fopen(fn.toLatin1().data(),"rb");
    assert(file);

    std::fseek(file,0,SEEK_END);
    int nbpoints = ftell(file)>>2;
    std::fseek(file,0,0);

    int absmin = std::max(0,xmin-10);
    int absmax = std::min(nbpoints,xmax+10);
    int abswidth = absmax-absmin;
    float *data;
    // allocate data memory
    data = (float *)malloc(abswidth*sizeof(float));
    // read data
    std::fseek(file,absmin*sizeof(float),SEEK_SET);
    std::fread(data,sizeof(float),abswidth,file);
    fclose(file);

    int factor = (nbpoints/width)/zoomfactor;
    // factor less than 3 no need to downsample
    if (factor < 3)
        factor = 1;

    this->displayseries->clear();

    QList<QPointF> points_list = downsample_minmax(data,factor,absmin,absmax);

    this->displayseries->replace(points_list);

    free(data);
}

QList<QPointF> Curve::downsample_minmax(float *data,int factor, int absmin, int absmax)
{
    QList<QPointF> points_list;
    int abswidth = absmax-absmin;
    float dmin = std::numeric_limits<float>::max();
    float dmax = std::numeric_limits<float>::min();
    for (int i = 0; i < abswidth; i ++)
    {
        if (factor > 1)
        {
            if (i%factor == 0)
            {
                dmin = std::min(data[i],dmin);
                dmax = std::max(data[i],dmax);
                points_list.append(QPointF(absmin+i,dmin));
                points_list.append(QPointF(absmin+i,dmax));

                dmin = std::numeric_limits<float>::max();
                dmax = std::numeric_limits<float>::min();
                continue;
            }
            dmin = std::min(data[i],dmin);
            dmax = std::max(data[i],dmax);
        }
        else
        {
            points_list.append(QPointF(absmin+i,data[i]));
        }

    }
    return points_list;
}
QtCharts::QLineSeries* Curve::getDisplaySeries()
{
    if (!this->displayseries)
    {
        this->displayseries = new QtCharts::QLineSeries();
        int width = ScaTool::main_plot->chart()->windowFrameRect().width();

        updateDisplaySeries(width,1,0,this->length());

        // Customize displayseries
        this->displayseries->setUseOpenGL(true);
        //this->displayseries->setPointsVisible();

        // handler
        QObject::connect(this->displayseries,&QLineSeries::clicked,this,&Curve::curve_clicked);
        QObject::connect(this->displayseries,&QLineSeries::hovered,this,&Curve::curve_clicked);
    }
    return this->displayseries;
}

int Curve::length()
{
    // get size
    FILE *file = fopen(fn.toLatin1().data(),"rb");
    assert(file);

    std::fseek(file,0,SEEK_END);
    int nbpoints = ftell(file)>>2;
    fclose(file);

    return nbpoints;
}

void Curve::curve_clicked(QPointF pt)
{
    ScaTool::statusbar->showMessage("Curve coordinate: (" + QString::number(pt.x()) + ", " + QString::number(pt.y()) + ")",0);
}

void Curve::shift(int offset)
{
    this->xoffset += offset;

    QLineSeries * cseries = getFullSeries();

    QList<QPointF> *ptlist = new QList<QPointF>;

    for (int i = 0 ; i < cseries->count(); i ++)
    {
        QPointF pt = cseries->at(i);
        pt.rx() += offset;
        ptlist->append(pt);
    }
    cseries->replace(*ptlist);
}


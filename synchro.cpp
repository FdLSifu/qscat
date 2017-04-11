#include "synchro.h"
#include "mainwindow.h"

/*Synchro::Synchro(QObject *parent)
{
    //QObject(parent);
    //Synchro(0);
}*/

Synchro::Synchro(int num)
{
    this->numpass = num;
    this->method = SYNCHRO_METHOD_SOD;
    this->precision = 1;

    //QObject::connect(this,&Synchro::finish,MainWindow::getInstance(),&MainWindow::updateStatusBar);
    //connect(this,SIGNAL(finish()), MainWindow::getInstance(),SLOT(updateStatusBar()));

}

Synchro::~Synchro()
{

}

void Synchro::run()
{
    min_dist_curve();
}

int Synchro::min_dist_curve()
{
    qreal dist = 0;
    qreal distmin = std::numeric_limits<qreal>::max();
    int offset;

    for (int s = leftwindow; s < rightwindow; s++)
    {
        dist = 0;
        for (int p = leftpattern ; p < rightpattern; p += precision)
        {
            dist += qAbs(cur_ref->series->at(p).y() - curve->series->at(p+s).y());
        }
        distmin = std::min(dist,distmin);
        if (distmin == dist)
        {
            offset = -s;
        }
    }
    emit this->finish();
    curve->shift(offset);
    return distmin;

}

void Synchro::finish()
{

}

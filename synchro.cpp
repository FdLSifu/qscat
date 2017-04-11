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
    min_dist_curve(this->curve,this->cur_ref,this->leftwindow,this->rightwindow,this->leftpattern,this->rightpattern);
}

int Synchro::min_dist_curve(Curve *c, Curve *ref_curve, int lwin, int rwin, int lpattern, int rpattern)
{
    qreal dist = 0;
    qreal distmin = std::numeric_limits<qreal>::max();
    int offset;

    for (int s = lwin; s < rwin; s++)
    {
        dist = 0;
        for (int p = lpattern ; p < rpattern; p ++)
        {
            dist += qAbs(ref_curve->series->at(p).y() - c->series->at(p+s).y());
        }
        distmin = std::min(dist,distmin);
        if (distmin == dist)
        {
            offset = -s;
        }
    }
    emit this->finish();
    c->shift(offset);
    return distmin;

}

void Synchro::finish()
{

}

#include "synchro.h"
#include "mainwindow.h"


Synchro::Synchro(int num):
      QObject()
{
    this->numpass = num;
    this->method = SYNCHRO_METHOD_SOD;
    this->precision = 1;
    QObject::connect(this,&Synchro::finish,ScaTool::synchrodialog,&SynchroDialog::update_progressdialog);
}

void Synchro::run()
{
    Synchro::min_dist_curve();
}

int Synchro::min_dist_curve()
{
    qreal dist = 0;
    qreal distmin = std::numeric_limits<qreal>::max();
    int offset = 0;

    QLineSeries * ref_subseries = cur_ref->getSubSeries(leftpattern,rightpattern);
    QLineSeries * work_subseries = curve->getSubSeries(leftpattern+leftwindow, rightpattern+rightwindow);

    for (int s = leftwindow; s < rightwindow; s++)
    {
        dist = 0;

        for (int p = leftpattern ; p < rightpattern; p += precision)
            dist += qAbs(ref_subseries->at(p-leftpattern).y() - work_subseries->at(p+s-leftpattern-leftwindow).y());

        distmin = std::min(dist,distmin);
        if (distmin == dist)
        {
            offset = -s;
        }
    }

    curve->shift(offset - curve->xoffset);

    delete ref_subseries;
    delete work_subseries;
    emit this->finish();
    return distmin;
}

void finish()
{
}

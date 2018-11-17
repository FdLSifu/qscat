#include "synchro.h"
#include "mainwindow.h"
#include "assert.h"
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

bool Synchro::stop = false;
QMutex Synchro::mutex;

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
    if (method == SYNCHRO_METHOD_SOD)
    {
        result.clear();
        for(int i = 0; i < this->curves.length() ; i++)
        {
            // Start the computation.
            QFuture<qreal> future = QtConcurrent::run(Synchro::min_dist_curve,this,i);
            result.append(future);
        }
    }
}

qreal Synchro::min_dist_curve(Synchro *sync,int idx)
{
    qreal dist = 0;
    int offset = 0;

    qreal distmin = std::numeric_limits<qreal>::max();
    Curve * curve = sync->curves.at(idx);
    Curve * cur_ref = sync->curves.at(sync->curve_ref_idx);

    int initial_ref_offset = sync->curve_offset.at(sync->curve_ref_idx);
    int initial_offset = sync->curve_offset.at(idx);

    int xmin_ref = sync->leftpattern-initial_ref_offset;
    int xmax_ref = sync->rightpattern-initial_ref_offset;
    int xmin_work = sync->leftpattern+sync->leftwindow-initial_offset;
    int xmax_work = sync->rightpattern+sync->rightwindow-initial_offset;

    float * ref_subseries = cur_ref->getSubSeries(xmin_ref, xmax_ref);
    float * work_subseries = curve->getSubSeries(xmin_work,xmax_work);

    if (cur_ref != curve)
    {
        for (int s = sync->leftwindow; s < sync->rightwindow; s++)
        {
            dist = 0;
            // Stop and exit
            Synchro::mutex.lock();
            if(Synchro::stop)
            {
                Synchro::mutex.unlock();
                distmin = 0;
                break;
            }
            Synchro::mutex.unlock();

            for (int p = sync->leftpattern ; p < sync->rightpattern; p += sync->precision)
                dist += qAbs(ref_subseries[p-sync->leftpattern] - work_subseries[p+s-sync->leftpattern-sync->leftwindow]);

            distmin = std::min(dist,distmin);
            if (distmin == dist)
            {
                offset = -s;
            }
        }
    }
    else
    {
        offset = 0;
        distmin = 0;
    }
    {
        // apply shift
        curve->shift(offset - curve->xoffset + initial_offset);

        // Populate offsets
        if (curve->offsets.length() > sync->numpass)
            curve->offsets.replace(sync->numpass,offset + initial_offset);
        else
            curve->offsets.append(offset + initial_offset);

        if (ref_subseries)
            free(ref_subseries);
        if (work_subseries)
            free(work_subseries);
    }
    // emit signals for progressbar
    emit sync->finish();

    return distmin;
}

void finish()
{
}

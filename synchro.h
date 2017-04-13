#ifndef SYNCHRO_H
#define SYNCHRO_H

#include "curve.h"
#include <QRunnable>
#include <QObject>

#define SYNCHRO_METHOD_SOD 0

class Synchro : public QObject, public QRunnable
{
   Q_OBJECT
public:
    int numpass;
    int method;
    Curve * curve;
    Curve * cur_ref;
    int leftpattern,rightpattern;
    int leftwindow,rightwindow;
    int precision;
    Synchro(int num);
    //int min_dist_curve(Curve *c, Curve *ref_curve, int lwin, int rwin, int lpattern, int rpattern);
    int min_dist_curve();
private:
    void run();

signals:
    void finish();
};

#endif // SYNCHRO_H

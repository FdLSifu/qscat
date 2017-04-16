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
    // Fields
    Curve * curve;
    Curve * cur_ref;
    int numpass;
    int method;
    int leftpattern,rightpattern;
    int leftwindow,rightwindow;
    int precision;

    // Constructor
    Synchro(int num);

    // Function
    int min_dist_curve();
private:
    void run();

signals:
    void finish();
};

#endif // SYNCHRO_H

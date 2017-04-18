#ifndef CURVELISTWIDGET_H
#define CURVELISTWIDGET_H

#include "curve.h"
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>

namespace Ui {
class CurveListWidget;
}

class CurveListWidget : public QWidget
{
    Q_OBJECT

public:
    bool firstDisplayed;
    QList<QPushButton*> *list_colors;
    QList<QCheckBox*> *list_checkbox;
    explicit CurveListWidget(QWidget *parent = 0);
    ~CurveListWidget();


    void    clear();
    void    addCurve(Curve *curve);
    Curve * getSelectedCurve();

private slots:
    void chkbox_toggled(bool state);
    void colorbtn_pressed();

private:
    Ui::CurveListWidget *ui;
};

#endif // CURVELISTWIDGET_H

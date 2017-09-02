#ifndef CURVELISTWIDGET_H
#define CURVELISTWIDGET_H

#include "curve.h"
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>

namespace Ui {
class CurveListWidget;
}

class CurveListWidget : public QWidget
{
    Q_OBJECT

public:
    bool firstDisplayed;
    explicit CurveListWidget(QWidget *parent = 0);
    ~CurveListWidget();

    void    clear();
    void    addCurve(Curve *curve);
    Curve * getSelectedCurve();
    QList<Curve *> getSelectedCurves();
    void setCurveRangeMax(void);

private slots:
    void global_type_changed(int type);
    void rowselected(int row, int column);
    void on_clearall_pressed();
    void on_displayall_pressed();
    void on_displayoff_pressed();
    void on_deletecurve_pressed();
    void on_redraw_pressed();
    void updateshiftvalue();

private:
    Ui::CurveListWidget *ui;
};

#endif // CURVELISTWIDGET_H

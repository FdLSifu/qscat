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

    Curve * getSelectedCurve();
    QVector<Curve *> getSelectedCurves();
    void setCurveRangeMax(void);
    void load_dataSet(QString filepath_dataset);
    void clear_dataSet();
private slots:
    void global_type_changed(int type);
    void on_clearall_pressed();
    void on_displayall_pressed();
    void on_displayoff_pressed();
    void on_redraw_pressed();
    void on_deleteCurve_pressed();
    void on_opendata_pressed();
    void colorbtn_pressed(const QModelIndex &index);
    void on_cleardata_pressed();

    void on_openoffsets_pressed();

    void on_saveoffsets_pressed();

private:
    Ui::CurveListWidget *ui;
};

#endif // CURVELISTWIDGET_H

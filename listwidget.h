#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QtCharts/QChartView>
#include <QListWidget>
#include "curve.h"

class ScaTool;
class ListWidget : public QListWidget
{
public:
    bool firstDisplayed;
    ListWidget();
    QListWidgetItem * getItemByName(QString name);
    QStringList * getListName();
public slots:
    void toogle_item(QListWidgetItem *);
};

#endif // LISTWIDGET_H

#include "listwidget.h"
#include "scatool.h"
#include <QtCharts/QLineSeries>
#include <QAreaSeries>
ListWidget::ListWidget()
{
    connect(this, &ListWidget::itemDoubleClicked,this,&ListWidget::toogle_item);
    firstDisplayed = true;
}

void ListWidget::toogle_item(QListWidgetItem * item)
{
    QString selected_fn = item->text();
    Curve * curve = ScaTool::getCurveByName(selected_fn);

    if (curve->displayed)
    {
        // remove from chart
        item->setTextColor(Qt::gray);
        curve->displayed = false;
        curve->getSeries()->hide();
    }
    else
    {
        curve->displayed = true;
        // Check if series is already present
        if (ScaTool::main_plot->chart()->series().indexOf(curve->getSeries()) >= 0)
            curve->getSeries()->show();
        else
        {
            QtCharts::QLineSeries * curseries = curve->getSeries();
            if (firstDisplayed)
            {

                ScaTool::main_plot->chart()->addSeries(curseries);
                ScaTool::main_plot->chart()->createDefaultAxes();
                firstDisplayed = false;
            }
            else
            {
                ScaTool::main_plot->chart()->addSeries(curseries);
                curseries->attachAxis(ScaTool::main_plot->chart()->axisX());
                curseries->attachAxis(ScaTool::main_plot->chart()->axisY());
            }
        }

        item->setTextColor(curve->getColor());
    }
}

QListWidgetItem * ListWidget::getItemByName(QString name)
{
    for(int i = 0; i < this->count(); i++)
    {
        QListWidgetItem * item = this->item(i);
        if (item->text() == name)
            return item;
    }
    return 0;
}

QStringList * ListWidget::getListName()
{

    QStringList * strlist = new QStringList();

    for(int i = 0; i < this->count(); i++)
    {
        QListWidgetItem * item = this->item(i);
        strlist->append((item->text()));
    }
    return strlist;
}


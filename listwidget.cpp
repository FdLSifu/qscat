#include "listwidget.h"
#include "scatool.h"
#include "chart.h"
#include <QtCharts/QLineSeries>
#include <QAreaSeries>
#include <QValueAxis>

ListWidget::ListWidget(QWidget *parent) : QListWidget(parent)
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
        curve->getDisplaySeries()->hide();
    }
    else
    {
        curve->displayed = true;
        // Check if series is already present
        if (ScaTool::main_plot->chart()->series().indexOf(curve->getDisplaySeries()) >= 0)
            curve->getDisplaySeries()->show();
        else
        {
            QtCharts::QLineSeries * curseries = curve->getDisplaySeries();
            if (firstDisplayed)
            {

                ScaTool::main_plot->chart()->addSeries(curseries);
                ScaTool::main_plot->chart()->createDefaultAxes();
                ScaTool::main_plot->chart()->orig_width = curve->length();
                connect(static_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisX()), &QValueAxis::rangeChanged,ScaTool::main_plot->chart(), &Chart::on_rangeChanged);
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


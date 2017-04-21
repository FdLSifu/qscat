#include "curvelistwidget.h"
#include "ui_curvelistwidget.h"
#include "scatool.h"
#include <QTableWidgetItem>
#include <QValueAxis>
#include <QColorDialog>

CurveListWidget::CurveListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurveListWidget)
{
    ui->setupUi(this);
    this->firstDisplayed = true;

    this->list_checkbox = new QList<QCheckBox*>();
    this->list_colors = new QList<QPushButton*>();
    this->list_cmbbox = new QList<QComboBox*>();
}

CurveListWidget::~CurveListWidget()
{
    delete ui;
}

void CurveListWidget::clear()
{
    ui->table_curve->clear();
}

void CurveListWidget::addCurve(Curve *curve)
{
    int rowidx = ui->table_curve->rowCount();
    int colidx = 0;
    ui->table_curve->insertRow(rowidx);

    //Nb
    ui->table_curve->setItem(rowidx,colidx,new QTableWidgetItem(QString::number(rowidx)));
    ui->table_curve->item(rowidx,colidx)->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

    //Color
    colidx ++;
    QPushButton *colorbtn = new QPushButton(this);
    this->list_colors->append(colorbtn);
    colorbtn->setPalette(QPalette(curve->getColor()));
    ui->table_curve->setCellWidget(rowidx,colidx,colorbtn);


    //Name
    colidx ++;
    ui->table_curve->setItem(rowidx,colidx,new QTableWidgetItem(curve->cname));
    ui->table_curve->item(rowidx,colidx)->setToolTip(curve->fn);
    ui->table_curve->item(rowidx,colidx)->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

    //Type
    colidx++;
    QComboBox *cmbbox = new QComboBox(this);
    cmbbox->addItem("float32");
    cmbbox->addItem("uint32");
    cmbbox->addItem("int32");
    cmbbox->addItem("uint16");
    cmbbox->addItem("int16");
    cmbbox->addItem("uint8");
    cmbbox->addItem("int8");
    this->list_cmbbox->append(cmbbox);
    ui->table_curve->setCellWidget(rowidx,colidx,cmbbox);

    //Offset
    colidx++;
    ui->table_curve->setItem(rowidx,colidx,new QTableWidgetItem(QString::number(curve-> xoffset)));
    ui->table_curve->item(rowidx,colidx)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable|Qt::ItemIsSelectable);

    //Displayed
    colidx++;
    QCheckBox *chkbox = new QCheckBox(this);
    this->list_checkbox->append(chkbox);
    ui->table_curve->setCellWidget(rowidx,colidx,chkbox);

    // Handler
    connect(colorbtn,&QPushButton::pressed,this,&CurveListWidget::colorbtn_pressed);
    connect(chkbox,&QCheckBox::toggled,this,&CurveListWidget::chkbox_toggled);
    connect(cmbbox,SIGNAL(currentIndexChanged(int)),this,SLOT(curve_type_changed(int)));
}

Curve * CurveListWidget::getSelectedCurve()
{

    QList<QTableWidgetItem *> itemlist = ui->table_curve->selectedItems();

    if (itemlist.length() < 1)
        return 0;
    else
    {
        int rowidx = itemlist.first()->row();
        return ScaTool::getCurveByName(ui->table_curve->item(rowidx,2)->text());
    }
}

QList<Curve *> CurveListWidget::getSelectedCurves()
{

    QList<QTableWidgetItem *> itemlist = ui->table_curve->selectedItems();

    QList<Curve*> clist = QList<Curve*>();
    for (int i = 0 ; i < itemlist.length() ; i ++)
    {
        int rowidx = itemlist.at(i)->row();
        clist.append(ScaTool::getCurveByName(ui->table_curve->item(rowidx,2)->text()));
    }
    return clist;
}

void CurveListWidget::chkbox_toggled(bool state)
{
    int rowidx = this->list_checkbox->indexOf((QCheckBox*)sender());
    Curve * curve = ScaTool::getCurveByName(ui->table_curve->item(rowidx,2)->text());

    // Check if curve already displayed
    if (curve->displayed)
    {
        // set ui color button to default color
        this->list_colors->at(rowidx)->setPalette(QPalette(Qt::white));
        // Set displayed to false (state is false here)
        curve->displayed = state;
        // Hide the curve from display
        curve->getDisplaySeries()->hide();
    }
    else
    {
        // set displayed to true (state is true here)
        curve->displayed = state;

        // Check if series is already present
        if (curve->isLoaded())
        {
            // Display it
            curve->getDisplaySeries()->show();
        }
        else
        {
            // Curve doesn't exist yet, we need to create it
            QtCharts::QLineSeries * curseries = curve->getDisplaySeries();
            ScaTool::main_plot->chart()->addSeries(curseries);
            if (firstDisplayed)
            {
                // We create axis realted to added series
                ScaTool::main_plot->chart()->createDefaultAxes();
                // Set original width to ease zoom work
                ScaTool::main_plot->chart()->xaxis_width = curve->length();

                // handler
                connect(qobject_cast<QValueAxis *>(ScaTool::main_plot->chart()->axisX()), &QValueAxis::rangeChanged,ScaTool::main_plot->chart(), &Chart::on_rangeChanged);

                // no more firstdisplayed
                firstDisplayed = false;
            }
            else
            {
                // reuse existed axis
                curseries->attachAxis(ScaTool::main_plot->chart()->axisX());
                curseries->attachAxis(ScaTool::main_plot->chart()->axisY());
            }
        }
        // Update color button from color curve
        this->list_colors->at(rowidx)->setPalette(QPalette( curve->getDisplaySeries()->color()));
    }
}


void CurveListWidget::colorbtn_pressed()
{
    int rowidx = this->list_colors->indexOf((QPushButton*)sender());
    Curve * curve = ScaTool::getCurveByName(ui->table_curve->item(rowidx,2)->text());

    QColor color = QColorDialog::getColor(curve->getColor());

    // Update curve color
    curve->setColor(color);

    // Update curve color on list
    if (curve->displayed)
    {
        this->list_colors->at(rowidx)->setPalette(QPalette(color));
        curve->displayseries->setColor(color);

        // Trick to redraw
        emit curve->displayseries->pointsReplaced();
    }

    // Update if curve is loaded but not displayed
    if (curve->isLoaded())
    {
        curve->getDisplaySeries()->setColor(curve->color);
    }

}

void CurveListWidget::curve_type_changed(int type)
{
    int rowidx = this->list_cmbbox->indexOf((QComboBox*)sender());
    Curve * curve = ScaTool::getCurveByName(ui->table_curve->item(rowidx,2)->text());

    if (curve == 0)
            return;
    curve->type = type;

    curve->updateDisplaySeries();
}

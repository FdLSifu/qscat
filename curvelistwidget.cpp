#include "curvelistwidget.h"
#include "ui_curvelistwidget.h"
#include "scatool.h"
#include <QTableWidgetItem>
#include <QValueAxis>
#include <QColorDialog>
#include <QFileDialog>
#include "curvetablemodel.h"

CurveListWidget::CurveListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurveListWidget)
{
    ui->setupUi(this);
    this->firstDisplayed = true;

    ui->type_box->addItem("float32");
    ui->type_box->addItem("uint32");
    ui->type_box->addItem("int32");
    ui->type_box->addItem("uint16");
    ui->type_box->addItem("int16");
    ui->type_box->addItem("uint8");
    ui->type_box->addItem("int8");
    ui->type_box->addItem("double");
    connect(ui->type_box,SIGNAL(currentIndexChanged(int)),this,SLOT(global_type_changed(int)));

    ScaTool::curve_table_model = new CurveTableModel(ui->tableview_curve);
    ui->tableview_curve->setModel(ScaTool::curve_table_model);
    ui->tableview_curve->setEditTriggers(QAbstractItemView::AllEditTriggers);
    connect(ui->tableview_curve,&QTableView::pressed,this,&CurveListWidget::colorbtn_pressed);

}
CurveListWidget::~CurveListWidget()
{
    delete ui;
}

Curve * CurveListWidget::getSelectedCurve()
{

    QItemSelectionModel *select = ui->tableview_curve->selectionModel();

    if (!select->hasSelection())
        return 0;
    else
    {
        int rowidx = select->selectedIndexes().first().row();
        return ScaTool::curves->at(rowidx);
    }
}

QVector<Curve *> CurveListWidget::getSelectedCurves()
{

    QItemSelectionModel *select = ui->tableview_curve->selectionModel();

    QVector<Curve*> clist = QVector<Curve*>();
    for (int i = 0 ; i < select->selectedIndexes().length() ; i ++)
    {
        int rowidx = select->selectedIndexes().at(i).row();
        Curve * curve = ScaTool::curves->at(rowidx);
        if(!clist.contains(curve))
            clist.append(curve);
    }
    return clist;
}

void CurveListWidget::global_type_changed(int type)
{
    Curve * curve;
    ScaTool::global_type = type;
    for(int rowidx = 0; rowidx < ScaTool::curves->length(); rowidx++)
    {
        curve = ScaTool::curves->at(rowidx);
        if (curve == 0)
            continue;

        curve->setType(type);

// TODO: Fix resizing
//        QtCharts::QLineSeries *curseries = curve->getDisplaySeries();
//        curseries->attachAxis(ScaTool::main_plot->chart()->axisX());
//        curseries->attachAxis(ScaTool::main_plot->chart()->axisY());

        ScaTool::statusbar->showMessage("Modifying curve ... "+QString::number(rowidx)+"/"+QString::number(ScaTool::curves->length()),0);
    }
    ScaTool::statusbar->showMessage("Done",1000);

    // Finally, update maximum range in Attack Window
    if (ScaTool::curves->length() > 0) {
        int points = ScaTool::curves->first()->getLength();

        ScaTool::attackdialog->setPtsNb(points);
        ScaTool::main_plot->chart()->xaxis_width = points;
    } else {
        ScaTool::attackdialog->setPtsNb(0);
    }

    emit ScaTool::curve_table_model->layoutChanged();
}

void CurveListWidget::on_clearall_pressed()
{
    if (ScaTool::curves->size() <= 0)
        return;

    // Because the curves delete themself, we need to do this...
    qDeleteAll(QVector<Curve*>(*ScaTool::curves));
    ScaTool::statusbar->showMessage("Deleted all curves", 1000);
    emit ScaTool::curve_table_model->layoutChanged();
}

void CurveListWidget::on_displayall_pressed()
{
    Curve *curve;
    for(int i = 0; i < ScaTool::curves->length() ; i++)
    {
        curve = ScaTool::curves->at(i);
        curve->chkbox_toggled(true);
    }
}

void CurveListWidget::on_displayoff_pressed()
{
    Curve *curve;
    for(int i = 0; i < ScaTool::curves->length() ; i++)
    {
        curve = ScaTool::curves->at(i);
        curve->chkbox_toggled(false);
    }
}

void CurveListWidget::on_deleteCurve_pressed()
{   
    QVector<Curve *> curves = getSelectedCurves();
    if (curves.length() <= 0)
        return;

    // Delete all selected curves (the curve destructer removes it from the curves)
    qDeleteAll(curves);
    ui->tableview_curve->selectionModel()->clearSelection();

    ScaTool::statusbar->showMessage(QString("Deleted %1 selected curves").arg(curves.length()), 1000);
    emit ScaTool::curve_table_model->layoutChanged();
}

void CurveListWidget::on_redraw_pressed()
{
    Curve *curve;
    int start = ui->curveStart->value();
    int end = ui->curveEnd->value() + 1;

    if ((end - start) < 1)
        return;

    if (!ScaTool::curves->length())
        return;

    if ((end-1) >= ScaTool::curves->length())
    end = ScaTool::curves->length();

    for(int i = 0; i < ScaTool::curves->length() ; i++) {
        curve = ScaTool::curves->at(i);
        curve->chkbox_toggled(false);
    }

    for(int i = start; i < end ; i++) {
        curve = ScaTool::curves->at(i);
        curve->chkbox_toggled(true);
    }
}

void CurveListWidget::setCurveRangeMax(void)
{
    ui->label_range->setText("[0 - " + QString::number(ScaTool::curves->length() - 1) + "]");
}

void CurveListWidget::clear_dataSet()
{
    Curve * curve;
    for(int i = 0; i < ScaTool::curves->length() ; i++) {
        curve = ScaTool::curves->at(i);
        curve->textin = "";
    }

    emit ScaTool::curve_table_model->layoutChanged();
    ScaTool::attackdialog->setTraceNb(0);
    ScaTool::attackdialog->setPtsNb(0);
}

void CurveListWidget::load_dataSet(QString filepath_dataset)
{
    Curve *curve;
    int N = ScaTool::curves->length();
    int input_len = 16; // HARDCODED FOR AES => BAD!
    QFile qf;
    QString input_dataset = filepath_dataset;

    qf.setFileName(input_dataset);
    if (!qf.open(QIODevice::ReadOnly))
        return;

    clear_dataSet();

    QByteArray bin = qf.readAll();
    // Load data even longer
    if ( ((bin.length() % input_len) != 0) || ((bin.length()/input_len) < N) )
        return;

    for (int i = 0; i < N; i++) {
        QString cl = "";
        curve = (*ScaTool::curves)[i];
        for (int j = 0; j < input_len; j++)
        {
            cl.append(QString().sprintf("%02x",(unsigned char)bin[(input_len*i)+j]));
            curve->input[j] = (uint8_t)bin[(input_len*i)+j];
        }
        curve->textin = cl;
    }

    int curve_pts = std::numeric_limits<int>::max();
    for (int i = 0; i < ScaTool::curves->length(); i++) {
        curve_pts = std::min(curve_pts,ScaTool::curves->at(i)->getLength());
    }

    ScaTool::attackdialog->setTraceNb(ScaTool::curves->length());
    ScaTool::attackdialog->setPtsNb(curve_pts);
}

void CurveListWidget::on_opendata_pressed()
{
    QString filepath_dataset;
    filepath_dataset = QFileDialog::getOpenFileName(this, QString("Select data set file"));
    load_dataSet(filepath_dataset);
}

void CurveListWidget::on_cleardata_pressed()
{
    clear_dataSet();
}

void CurveListWidget::on_openoffsets_pressed()
{
    int buf = 0;
    if (ScaTool::curves->length() == 0)
        return;

    // Create offsets data file
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load Offsets"),QDir::currentPath(),tr("Binary (*.bin)"));
    QFile trace(fileName);
    if (trace.open(QIODevice::ReadOnly) == false)
            return;

    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        Curve* c = ScaTool::curves->at(i);
        trace.read(reinterpret_cast<char*>(&buf), sizeof(int));
        c->shift(buf-c->xoffset);
        ScaTool::statusbar->showMessage("Loading offset ... "+QString::number(i)+"/"+QString::number(ScaTool::curves->length()),0);
    }
    ScaTool::statusbar->showMessage("Load offset done",1000);
}

void CurveListWidget::on_saveoffsets_pressed()
{
    if (ScaTool::curves->length() == 0)
        return;

    // Create offsets data file
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Offsets"),QDir::currentPath(),tr("Binary (*.bin)"));
    QFile trace(fileName);
    if (trace.open(QIODevice::WriteOnly) == false)
            return;

    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        Curve* c = ScaTool::curves->at(i);
        trace.write(reinterpret_cast<const char*>(&c->xoffset), sizeof(int));
    }
}

void CurveListWidget::colorbtn_pressed(const QModelIndex &index)
{
    Curve * curve = ScaTool::curves->at(index.row());
    if ( (index.column() == 1) && (curve->displayed) ) // Color
    {
        QColorDialog qcd(0);
        qcd.setWindowTitle("Pick a color");
        qcd.exec();
        QColor color = qcd.selectedColor();

        if (qcd.result())
            // Update curve color
            curve->setColor(color);
        else
            return;

        // Update curve color on list
        if (curve->displayed)
        {
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
}

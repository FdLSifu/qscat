#include "mainwindow.h"
#include "ui_design.h"
#include <QtWidgets/QWidget>
#include <stdio.h>
#include <assert.h>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QFileDialog>
#include <QString>
#include <QLabel>
#include <QValueAxis>
#include <QAreaSeries>

MainWindow * MainWindow::instance = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    ui->downplot->hide();

    instance = this;
    ScaTool::curves = new QList<Curve*>();
    ScaTool::qlistwidget = new ListWidget();
    ScaTool::qlistwidget->setWindowTitle("Working files");

    ScaTool::synchrodialog = new SynchroDialog();
    ScaTool::main_plot = ui->mainplot;
    ScaTool::statusbar = ui->statusbar;

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->setMargins(QMargins(0,0,0,0));
    chart->createDefaultAxes();
    chart->setAcceptHoverEvents(true);

    ui->mainplot->setChart(chart);
    ui->mainplot->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ScaTool::qlistwidget;
}

MainWindow * MainWindow::getInstance()
{
    return instance;
}

void MainWindow::on_open_pressed()
{
    QStringList fnames = QFileDialog::getOpenFileNames(this);
    // Open filename

    for (QStringList::iterator it = fnames.begin();
         it != fnames.end(); ++it) {
        QString fn = *it;

        // Check file exist
        FILE *file = fopen(fn.toLatin1().data(),"rb");
        assert(file);
        fclose(file);

        int idx = ScaTool::curves->length();

        Curve *curve = new Curve(idx);
        curve->fn = fn;
        ScaTool::curves->append(curve);

        QListWidgetItem *item = new QListWidgetItem(curve->fn);
        ScaTool::qlistwidget->addItem(item);
        ScaTool::synchrodialog->addRefItem(curve->fn);
    }

    ScaTool::show_file_window();
}

void MainWindow::on_left_pressed()
{
    Curve * c = ScaTool::getSelectedCurve();

    if (c == 0)
        return;

    c->shift(-1);

}

void MainWindow::on_lleft_pressed()
{
    Curve * c = ScaTool::getSelectedCurve();

    if (c == 0)
        return;

    c->shift(-10);

}

void MainWindow::on_zero_pressed()
{
    Curve * c = ScaTool::getSelectedCurve();

    if (c == 0)
        return;

    c->shift(-c->xoffset);
}

void MainWindow::on_right_pressed()
{
    Curve * c = ScaTool::getSelectedCurve();

    if (c == 0)
        return;

    c->shift(1);
}

void MainWindow::on_rright_pressed()
{
    Curve * c = ScaTool::getSelectedCurve();

    if (c == 0)
        return;

    c->shift(10);
}

void MainWindow::on_settings_pressed()
{
    if (ScaTool::qlistwidget->isHidden())
    {
        ScaTool::qlistwidget->show();
        ScaTool::qlistwidget->activateWindow();
        ScaTool::qlistwidget->raise();


    }
    else
        ScaTool::qlistwidget->hide();
}

void MainWindow::on_synchro_pressed()
{
    ScaTool::synchrodialog->show();
}

void MainWindow::updateStatusBar()
{

    ScaTool::statusbar->showMessage(QString::number(QThreadPool::globalInstance()->activeThreadCount()));
}

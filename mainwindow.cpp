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
#include <QApplication>
#include <QDockWidget>

MainWindow * MainWindow::instance = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->showMaximized();
    ui->downplot->hide();

    MainWindow::instance = this;

    ScaTool::curves = new QList<Curve*>();
    ScaTool::dockcurves = new QDockWidget(tr("Working curves"),this);
    ScaTool::qlistwidget = new ListWidget(ScaTool::dockcurves);
    ScaTool::dockcurves->setWidget(ScaTool::qlistwidget);
    addDockWidget(Qt::RightDockWidgetArea, ScaTool::dockcurves);

    ScaTool::synchrodialog = new SynchroDialog(this);
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
    delete ScaTool::synchrodialog;

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
        // check if curves already inserted
        if (ScaTool::getCurveByName(fn) == 0)
        {
            ScaTool::curves->append(curve);

            QListWidgetItem *item = new QListWidgetItem(curve->fn);
            ScaTool::qlistwidget->addItem(item);
            ScaTool::synchrodialog->addRefItem(curve->fn);
        }
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
    if (ScaTool::dockcurves->isHidden())
    {
        ScaTool::dockcurves->show();
        ScaTool::dockcurves->activateWindow();
        ScaTool::dockcurves->raise();


    }
    else
        ScaTool::dockcurves->hide();
}

void MainWindow::on_synchro_pressed()
{
    ScaTool::synchrodialog->show();
}

void MainWindow::updateStatusBar()
{

    ScaTool::statusbar->showMessage(QString::number(QThreadPool::globalInstance()->activeThreadCount()));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    on_refresh_pressed();
    QApplication::closeAllWindows();
    QApplication::quit();
}

void MainWindow::on_refresh_pressed()
{

    qDeleteAll(ScaTool::synchrodialog->synchropasses->begin(),ScaTool::synchrodialog->synchropasses->end());
    ScaTool::qlistwidget->clear();
    if (ScaTool::curves->length() > 0)
    {
        qDeleteAll(ScaTool::curves->begin(),ScaTool::curves->end());
        ScaTool::curves->clear();
    }
}

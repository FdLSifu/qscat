#include "mainwindow.h"
#include "ui_design.h"
#include <QtWidgets/QWidget>
#include <stdio.h>
#include <assert.h>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QLabel>
#include <QValueAxis>
#include <QAreaSeries>
#include <QApplication>
#include <QDockWidget>
#include <QToolButton>
#include <QErrorMessage>

MainWindow * MainWindow::instance = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->showMaximized();
    ui->downplot->hide();

    MainWindow::instance = this;

    // Dark gray
    //this->setPalette(QPalette(QColor(30,30,30)));
    ScaTool::curves = new QList<Curve*>();
    ScaTool::dockcurves = new QDockWidget(this);
    ScaTool::curve_table = new CurveListWidget(ScaTool::dockcurves);
    ScaTool::dockcurves->setWidget(ScaTool::curve_table);
    ScaTool::dockcurves->setFeatures(QDockWidget::DockWidgetMovable);
    this->addDockWidget(Qt::BottomDockWidgetArea, ScaTool::dockcurves);

    ScaTool::synchrodialog = new SynchroDialog(this);
    ScaTool::attackdialog = new Attackwindow(this);
    ScaTool::attacklog = new AttackLog(this);
    ScaTool::main_plot = ui->mainplot;
    ScaTool::statusbar = ui->statusbar;

    Chart *chart = new Chart();
    ui->mainplot->setChart(chart);
    ui->mainplot->setRenderHint(QPainter::Antialiasing);
    //chart->setTheme(QChart::ChartThemeDark);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ScaTool::curve_table;
    delete ScaTool::synchrodialog;
    delete ScaTool::attackdialog;
    delete ScaTool::attacklog;
}

MainWindow * MainWindow::getInstance()
{
    return instance;
}

class QBinaryFileDialog : public QFileDialog
{

};

void MainWindow::on_open_pressed()
{
    QStringList fnames = QFileDialog::getOpenFileNames(this,QString("Select traces to open"));
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

        QFileInfo fileInfo(fn);
        QString cname(fileInfo.fileName());
        curve->cname = cname;

        // check if curves already inserted
        if (ScaTool::getCurveByName(cname) == 0)
        {
            // Append curve to set of all managed curves
            ScaTool::curves->append(curve);
            // Add curve to curve table display
            ScaTool::curve_table->addCurve(curve);
            // Add curve to synchro table display
            ScaTool::synchrodialog->addRefItem(curve->cname);
        }
    }

    // Ui effect to show curve list
    if (ScaTool::dockcurves->isHidden())
        ScaTool::dockcurves->show();
}

void MainWindow::on_left_pressed()
{
    QList<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

    for (int i = 0; i < clist.length() ; i++)
        clist.at(i)->shift(-1);
}

void MainWindow::on_lleft_pressed()
{
    QList<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

    for (int i = 0; i < clist.length() ; i++)
        clist.at(i)->shift(-10);
}

void MainWindow::on_zero_pressed()
{
    QList<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

    for (int i = 0; i < clist.length() ; i++)
        clist.at(i)->shift(-clist.at(i)->xoffset);
}

void MainWindow::on_right_pressed()
{
    QList<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

    for (int i = 0; i < clist.length() ; i++)
        clist.at(i)->shift(1);
}

void MainWindow::on_rright_pressed()
{
    QList<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

    for (int i = 0; i < clist.length() ; i++)
        clist.at(i)->shift(10);
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

    qDeleteAll(ScaTool::synchrodialog->synchropasses.begin(),ScaTool::synchrodialog->synchropasses.end());
    if (ScaTool::curves->length() > 0)
    {
        qDeleteAll(ScaTool::curves->begin(),ScaTool::curves->end());
    }
    ScaTool::curves->clear();
    ScaTool::curve_table->clear();
    if (ScaTool::dockcurves->isHidden())
        ScaTool::dockcurves->show();
    for (int i = 0 ; i < ScaTool::main_plot->chart()->axes(Qt::Horizontal).length(); i ++)
        ScaTool::main_plot->chart()->removeAxis(ScaTool::main_plot->chart()->axes().at(i));
    for (int i = 0 ; i < ScaTool::main_plot->chart()->axes(Qt::Vertical).length(); i ++)
        ScaTool::main_plot->chart()->removeAxis(ScaTool::main_plot->chart()->axes().at(i));
    ScaTool::curve_table->firstDisplayed = true;
}

void MainWindow::on_curves_pressed()
{
    if (ScaTool::dockcurves->isHidden())
    {
        ScaTool::dockcurves->show();
        ui->curves->setIcon(QIcon(":images/arrow-down.png"));
    }
    else
    {
        ScaTool::dockcurves->hide();
        ui->curves->setIcon(QIcon(":images/arrow-up.png"));
    }
}

void MainWindow::on_attack_pressed()
{
    if (ScaTool::attackdialog->daredevil_path.length()) {
        ScaTool::attackdialog->show();
    } else {
        QErrorMessage *error = new QErrorMessage(this);
        error->showMessage("Daredevil binary not found in the project/global path");
    }
}

void MainWindow::on_save_pressed()
{
    int curve_length = 0;
    if (ScaTool::curves->length() == 0)
        return;

    // Create traces data file
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),QDir::currentPath(),tr("Binary (*.bin)"));
    QFile trace(fileName);
    if (trace.open(QIODevice::WriteOnly) == 0)
            return;

    for (int i = 0; i < ScaTool::curves->length(); i++)
    {
        Curve* c = ScaTool::curves->at(i);
        float * buf = c->getrawdata(&curve_length, c->xoffset);
        trace.write(reinterpret_cast<const char*>(buf), curve_length<<2);
        // free buf
        free(buf);

    }

}

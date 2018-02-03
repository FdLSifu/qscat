#include "attackwindow.h"
#include "ui_attackwindow.h"
#include "scatool.h"
#include <assert.h>
#include <QFileDialog>
#include <QSpinBox>
#include <QTableWidget>
#include <QByteArray>
#include <qdebug.h>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QProcess>
#include <QStringList>
#include <QMovie>
#include <QLabel>
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QMimeData>

Attackwindow::Attackwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Attackwindow)
{
    QString daredevil_local = QDir::currentPath();

    ui->setupUi(this);

    int algo_idx = 0;
    algo_map["AES"] = algo_idx++;

    ui->algoBox->clear();
    ui->algoBox->addItems(QStringList()
                          <<"AES");

    ui->functionBox->clear();
    ui->functionBox->addItems(QStringList()
                              <<"1st round output SBOX"
                              <<"1st round output Mult Inv"
                              <<"1st XOR"
                              <<"Input");

    ui->methodBox->clear();
    ui->methodBox->addItems(QStringList() << "CPA");

    ui->byteIdxTable->resizeColumnsToContents();
    ui->byteIdxTable->resizeRowsToContents();
    ui->byteIdxTable->hasAutoScroll();

    ui->spinnb_traces->setMaximum(0);

    QHeaderView* headerh = ui->byteIdxTable->horizontalHeader();
    headerh->setSectionResizeMode(QHeaderView::Stretch);

    QHeaderView* headerw = ui->byteIdxTable->verticalHeader();
    headerw->setSectionResizeMode(QHeaderView::Stretch);

    this->daredevil_path.clear();
    if (QFileInfo(daredevil_local.toUtf8() + "/daredevil/daredevil").isExecutable())
        /* exec from qscat folder */
        this->daredevil_path.append(daredevil_local.toUtf8() + "/daredevil/");
    else if (QFileInfo(daredevil_local.toUtf8() + "/../qscat/daredevil/daredevil").isExecutable())
        /* exec from dev folder */
        this->daredevil_path.append(daredevil_local.toUtf8() + "/../qscat/daredevil/");
    this->daredevilLog = daredevil_local + "/last_daredevil.log";
}

Attackwindow::~Attackwindow()
{
    delete ui;
}

void Attackwindow::saveDaredevilLog(void)
{
    QFile log(this->daredevilLog);

    log.open(QIODevice::ReadWrite);
    log.write(this->stdout_log.toUtf8());
    log.flush();
    log.close();
    this->stdout_log.clear();
}

void Attackwindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QList<QUrl> urls = mimeData->urls();
        if (urls.size() == 1)
        {
            QUrl url = urls.at(0);
            if (url.isValid() && (url.scheme().toLower() == "file") && mimeData->hasFormat("text/uri-list"))
            {
                event->acceptProposedAction();
            }
        }
    }
}

void Attackwindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void Attackwindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void Attackwindow::on_spinpts_start_editingFinished()
{
    //Update value range
    if (ScaTool::curves->length() > 0) {
        ui->spinpts_start->setRange(0,ScaTool::curves->first()->length());
        ui->spinpts_end->setRange(0,ScaTool::curves->first()->length());
    }

    // Respect min and max
    if (ui->spinpts_start->value() > ui->spinpts_end->value())
        ui->spinpts_end->setValue(ui->spinpts_start->value());
}

void Attackwindow::on_spinpts_end_editingFinished()
{
    //Update value range
    if (ScaTool::curves->length() > 0) {
        ui->spinpts_start->setRange(0,ScaTool::curves->first()->length());
        ui->spinpts_end->setRange(0,ScaTool::curves->first()->length());
    }

    // Respect min and max
    if (ui->spinpts_start->value() > ui->spinpts_end->value())
        ui->spinpts_start->setValue(ui->spinpts_end->value());
}

void Attackwindow::processOutput()
{
    //qDebug() << this->process->readAllStandardError();
    QString line = this->process->readAllStandardOutput();
    this->stdout_log.append(line);
    ScaTool::attacklog->fillSumMaxCorr(this->stdout_log);
    ScaTool::attacklog->fillSumMaxKey(this->stdout_log);
}

void Attackwindow::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    int time = this->processTime.elapsed();

    saveDaredevilLog();
    this->tdir->remove();
    ScaTool::attacklog->updateLabelLog("Computing time: " + QString::number(time) + " ms - daredevil log saved in " + this->daredevilLog);
    ui->attackButton->setEnabled(true);
}

void Attackwindow::on_attackButton_pressed()
{
    // Get values from UI
    int sel_fun = ui->functionBox->currentIndex();
    int pts_min = ui->spinpts_start->value();
    int pts_max = ui->spinpts_end->value();
    int nb_pts = pts_max - pts_min;
    int nb_traces = ui->spinnb_traces->value();
    int length;
    //ui->byteIdxTable->selectedItems()
    QMovie *movie = new QMovie(":images/ajax-loader.gif");
    QLabel *pr = new QLabel(this);

    ui->attackButton->setEnabled(false);
    pr->setMovie(movie);
    pr->show();
    movie->start();
    qApp->processEvents();

    // Create a working directory
    this->tdir = new QTemporaryDir();
    assert(this->tdir->isValid());
    this->tdir->setAutoRemove(false);

    // Create traces data file
    QFile trace(this->tdir->path() + "/trace.bin");
    assert(trace.open(QIODevice::ReadWrite));
    for (int i = 0; i < nb_traces; i++)
    {
        Curve* c = ScaTool::curves->at(i);
        float * buf = c->getrawdata(&length, c->xoffset);
        trace.write(reinterpret_cast<const char*>(&buf[pts_min]), nb_pts<<2);
        free(buf);
    }

    QFile::copy(this->input_dataset, this->tdir->path() + "/input.bin");
    if (sel_fun == 0)
        QFile::copy(this->daredevil_path + "LUT/AES_AFTER_SBOX",
                this->tdir->path() + "/lut");
    else if (sel_fun == 1)
        QFile::copy(this->daredevil_path + "LUT/AES_AFTER_MULTINV",
                this->tdir->path() + "/lut");
    else
        QFile::copy(this->daredevil_path + "LUT/AES_BEFORE_SBOX",
                this->tdir->path() + "/lut");

    // Create config file
    QFile config(this->tdir->path() + "/CONFIG");
    assert(config.open(QIODevice::ReadWrite | QIODevice::Text));
    config.write("[Traces]\n");
    config.write("files=1\n");
    config.write("trace_type=f\n");
    config.write("transpose=false\n");
    config.write("index=0\n");
    config.write("nsamples=" + QString::number(nb_pts).toUtf8() + "\n");
    config.write("trace=" + this->tdir->path().toUtf8() + "/trace.bin "
                 + QString::number(nb_traces).toUtf8() + " "
                 + QString::number(nb_pts).toUtf8() + "\n");

    config.write("[Guesses]\n");
    config.write("files=1\n");
    config.write("guess_type=u\n");
    config.write("transpose=false\n");
    config.write("guess=" + this->tdir->path().toUtf8() +"/input.bin" + " "
                 + QString::number(nb_traces).toUtf8() + " 16\n");
    config.write("[General]\n");
    config.write("threads=8\n");
    config.write("order=1\n");
    config.write("return_type=double\n");
    config.write("algorithm=AES\n");
    config.write("position=" + this->tdir->path().toUtf8() + "/lut\n");
    config.write("round=0\n");
    config.write("bytenum=all\n");
    config.write("bitnum=none\n");
    config.write("memory=4G\n");
    config.write("top=20\n");
    config.write("corrout=t\n");

    config.flush();
    trace.flush();
    config.close();
    trace.close();

    if (ScaTool::attacklog)
	delete ScaTool::attacklog;

    ScaTool::attacklog = new AttackLog(this);
    ScaTool::attacklog->show();

    this->process = new QProcess(this);
    connect(this->process, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    connect(this->process, SIGNAL(readyReadStandardError()), this, SLOT(processOutput()));
    connect(this->process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));

    this->processTime.start();
    this->process->start(this->daredevil_path + "daredevil -c " + config.fileName());

    movie->stop();
    pr->hide();
}

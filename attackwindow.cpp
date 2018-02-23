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
#include <QtConcurrent/QtConcurrent>
#include "cpa.h"

Attackwindow::Attackwindow(QWidget *parent) :
    QWidget(parent,Qt::Window),
    ui(new Ui::Attackwindow)
{


    ui->setupUi(this);
    int algo_idx = 0;
    algo_map["AES"] = algo_idx++;

    ui->algoBox->clear();
    ui->algoBox->addItems(QStringList()
                          <<"AES");

    ui->functionBox->clear();
    ui->functionBox->addItems(QStringList()
                              <<"AES output SBOX"
                              <<"AES output Mult Inv"
                              <<"AES XOR (TextIn/TextOut)"
                              <<"AES output SBOX Inv");

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

}

Attackwindow::~Attackwindow()
{
    delete ui;
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
        ui->spinpts_start->setRange(0,ScaTool::curves->first()->getLength());
        ui->spinpts_end->setRange(0,ScaTool::curves->first()->getLength());
    }

    // Respect min and max
    if (ui->spinpts_start->value() > ui->spinpts_end->value())
        ui->spinpts_end->setValue(ui->spinpts_start->value());
}

void Attackwindow::on_spinpts_end_editingFinished()
{
    //Update value range
    if (ScaTool::curves->length() > 0) {
        ui->spinpts_start->setRange(0,ScaTool::curves->first()->getLength());
        ui->spinpts_end->setRange(0,ScaTool::curves->first()->getLength());
    }

    // Respect min and max
    if (ui->spinpts_start->value() > ui->spinpts_end->value())
        ui->spinpts_start->setValue(ui->spinpts_end->value());
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


    if (ui->attackButton->text().compare(QString("Stop Attack")) == 0)
    {
        ui->attackButton->setText(QString("Launch Attack"));
        return;
    }
    else
        ui->attackButton->setText(QString("Stop Attack"));

    QTime qt = QTime();
    qt.start();
    CPA *cpa = new CPA(ScaTool::curves,0,pts_min,pts_max);

    for (int i = 0; i < 16; i ++)
    {
        cpa->setbyteidx(i);
        cpa->run();
    }
    int el = qt.elapsed();
    qDebug("Time : %d ms\n",el);
}

void Attackwindow::setTraceNb(int t)
{
    ui->spinnb_traces->setMaximum(t);
    ui->spinnb_traces->setValue(t);
}

void Attackwindow::setPtsNb(int p)
{
    ui->spinpts_end->setMaximum(p);
    ui->spinpts_end->setValue(p);
}

void Attackwindow::on_showattack_pressed()
{

}

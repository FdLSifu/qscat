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
#include <QPlainTextEdit>
#include "cpa.h"
#include <unistd.h>

Attackwindow::Attackwindow(QWidget *parent) :
    QWidget(parent,Qt::Window),
    ui(new Ui::Attackwindow)
{


    this->cpa = 0;
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
    ui->byteIdxTable->item(0,0)->setSelected(true); // byte 0 default

    ui->spinnb_traces->setMaximum(0);

    QHeaderView* headerh = ui->byteIdxTable->horizontalHeader();
    headerh->setSectionResizeMode(QHeaderView::Stretch);

    QHeaderView* headerw = ui->byteIdxTable->verticalHeader();
    headerw->setSectionResizeMode(QHeaderView::Stretch);

    for(int i = 0; i < 17; i ++)
    {
        hideResult(i);
    }

    ui->chart->chart()->legend()->hide();

    QObject::connect(ui->pushButton,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_2,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_3,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_4,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_5,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_6,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_7,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_8,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_9,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_10,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_11,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_12,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_13,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_14,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_15,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_16,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
    QObject::connect(ui->pushButton_all,&QPushButton::toggled,this,&Attackwindow::corrcchart_toggled);
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
    int nb_traces = ui->spinnb_traces->value();

    if(cpa)
        delete cpa;

    // Clean series
    ui->chart->chart()->removeAllSeries();
    // Trick to redraw
    ui->chart->resize(ui->chart->size() + QSize(1,1));
    ui->chart->resize(ui->chart->size() - QSize(1,1));

    // Work with sub set of curves
    QVector<Curve*> *curves = new QVector<Curve*>(nb_traces);
    for (int i = 0; i < nb_traces; i ++)
        (*curves)[i] = (*ScaTool::curves)[i];

    // Create CPA object
    cpa = new CPA(curves,sel_fun,pts_min,pts_max);

    // Populate byteIdxTable with selected byte
    for(int i = 0; i < ui->byteIdxTable->selectedItems().length(); i++)
    {
        int byte = ui->byteIdxTable->selectedItems().at(i)->text().toInt();
        cpa->byteidx.append(byte);
    }

    for(int i = 0; i < 17; i ++)
    {
        hideResult(i);
    }

    QObject::connect(cpa,&CPA::finished,this,&Attackwindow::cpa_finished);

    QtConcurrent::run(CPA::run,cpa);

}

void Attackwindow::hideResult(int byte)
{
    QListView *lv = 0;
    QPushButton *pb = 0;
    QTextEdit *pe = 0;

    switch (byte) {
    case 0:
        lv = ui->listView;
        pb = ui->pushButton;
        pe = ui->textEdit;
        break;
    case 1:
        lv = ui->listView_2;
        pb = ui->pushButton_2;
        pe = ui->textEdit_2;
        break;
    case 2:
        lv = ui->listView_3;
        pb = ui->pushButton_3;
        pe = ui->textEdit_3;
        break;
    case 3:
        lv = ui->listView_4;
        pb = ui->pushButton_4;
        pe = ui->textEdit_4;
        break;
    case 4:
        lv = ui->listView_5;
        pb = ui->pushButton_5;
        pe = ui->textEdit_5;
        break;
    case 5:
        lv = ui->listView_6;
        pb = ui->pushButton_6;
        pe = ui->textEdit_6;
        break;
    case 6:
        lv = ui->listView_7;
        pb = ui->pushButton_7;
        pe = ui->textEdit_7;
        break;
    case 7:
        lv = ui->listView_8;
        pb = ui->pushButton_8;
        pe = ui->textEdit_8;
        break;
    case 8:
        lv = ui->listView_9;
        pb = ui->pushButton_9;
        pe = ui->textEdit_9;
        break;
    case 9:
        lv = ui->listView_10;
        pb = ui->pushButton_10;
        pe = ui->textEdit_10;
        break;
    case 10:
        lv = ui->listView_11;
        pb = ui->pushButton_11;
        pe = ui->textEdit_11;
        break;
    case 11:
        lv = ui->listView_12;
        pb = ui->pushButton_12;
        pe = ui->textEdit_12;
        break;
    case 12:
        lv = ui->listView_13;
        pb = ui->pushButton_13;
        pe = ui->textEdit_13;
        break;
    case 13:
        lv = ui->listView_14;
        pb = ui->pushButton_14;
        pe = ui->textEdit_14;
        break;
    case 14:
        lv = ui->listView_15;
        pb = ui->pushButton_15;
        pe = ui->textEdit_15;
        break;
    case 15:
        lv = ui->listView_16;
        pb = ui->pushButton_16;
        pe = ui->textEdit_16;
        break;
    case 16:
        lv = 0;
        pb = ui->pushButton_all;
        pe = 0;
        break;
    default:
        break;
    }

    pb->setCheckable(true);
    pb->setChecked(false);
    if ((lv != 0) && (pe != 0))
    {
        lv->hide();
        pb->hide();
        pe->hide();
        pe->setTextInteractionFlags(Qt::NoTextInteraction);
        pe->setText("<center>"+QString::number(byte)+"</center>");
    }
}
void Attackwindow::cpa_finished(int byte)
{
    QListView *lv = 0;
    QPushButton *pb = 0;
    QTextEdit *pe = 0;

    switch (byte) {
    case 0:
        lv = ui->listView;
        pb = ui->pushButton;
        pe = ui->textEdit;
        break;
    case 1:
        lv = ui->listView_2;
        pb = ui->pushButton_2;
        pe = ui->textEdit_2;
        break;
    case 2:
        lv = ui->listView_3;
        pb = ui->pushButton_3;
        pe = ui->textEdit_3;
        break;
    case 3:
        lv = ui->listView_4;
        pb = ui->pushButton_4;
        pe = ui->textEdit_4;
        break;
    case 4:
        lv = ui->listView_5;
        pb = ui->pushButton_5;
        pe = ui->textEdit_5;
        break;
    case 5:
        lv = ui->listView_6;
        pb = ui->pushButton_6;
        pe = ui->textEdit_6;
        break;
    case 6:
        lv = ui->listView_7;
        pb = ui->pushButton_7;
        pe = ui->textEdit_7;
        break;
    case 7:
        lv = ui->listView_8;
        pb = ui->pushButton_8;
        pe = ui->textEdit_8;
        break;
    case 8:
        lv = ui->listView_9;
        pb = ui->pushButton_9;
        pe = ui->textEdit_9;
        break;
    case 9:
        lv = ui->listView_10;
        pb = ui->pushButton_10;
        pe = ui->textEdit_10;
        break;
    case 10:
        lv = ui->listView_11;
        pb = ui->pushButton_11;
        pe = ui->textEdit_11;
        break;
    case 11:
        lv = ui->listView_12;
        pb = ui->pushButton_12;
        pe = ui->textEdit_12;
        break;
    case 12:
        lv = ui->listView_13;
        pb = ui->pushButton_13;
        pe = ui->textEdit_13;
        break;
    case 13:
        lv = ui->listView_14;
        pb = ui->pushButton_14;
        pe = ui->textEdit_14;
        break;
    case 14:
        lv = ui->listView_15;
        pb = ui->pushButton_15;
        pe = ui->textEdit_15;
        break;
    case 15:
        lv = ui->listView_16;
        pb = ui->pushButton_16;
        pe = ui->textEdit_16;
        break;
    default:
        break;
    }

    if (lv == 0)
            return;

    CorrListModel *clm = new CorrListModel(this, this->cpa,lv,byte);
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel();
    proxy->setSourceModel(clm);
    lv->setModel(proxy);

    proxy->sort(0,Qt::DescendingOrder);
    lv->show();
    pb->show();
    pe->show();

}

void Attackwindow::corrcchart_toggled(bool checked)
{
    bool integer;
    QPushButton * senderObj = (QPushButton*)sender();
    int v = senderObj->text().toInt(&integer);
    if(!integer)
    {
        if (checked)
        {

        }
        else
        {

        }
    }
    else
    {
        if (checked)
        {
            for(int k = 0; k < 256; k ++)
            {
                QLineSeries *ql = new QLineSeries();
                ql->setName(QString::number(v));
                ql->setUseOpenGL(true);
                for (int i = 0; i < this->cpa->samples_number; i ++)
                    ql->append(i,this->cpa->correlation[v][k][i]);
                ui->chart->chart()->addSeries(ql);
            }
            ui->chart->chart()->createDefaultAxes();

        }
        else
        {
            QListIterator<QAbstractSeries*> i(ui->chart->chart()->series());
            while(i.hasNext())
            {
                QLineSeries *ql = dynamic_cast<QLineSeries*>(i.next());
                if(ql->name().compare(QString::number(v)) == 0)
                {
                    ui->chart->chart()->removeSeries(ql);
                    delete ql;
                }
            }
            // Trick to redraw
            ui->chart->resize(ui->chart->size() + QSize(1,1));
            ui->chart->resize(ui->chart->size() - QSize(1,1));
        }
    }
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

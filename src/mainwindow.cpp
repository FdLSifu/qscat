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
#include <QMessageBox>
#include <QInputDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMenu>
#include <QDialogButtonBox>
#include <QFormLayout>

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
	ScaTool::curves = new QVector<Curve*>();
	ScaTool::dockcurves = new QDockWidget(this);
	ScaTool::curve_table = new CurveListWidget(ScaTool::dockcurves);
	ScaTool::dockcurves->setWidget(ScaTool::curve_table);
	ScaTool::dockcurves->setFeatures(QDockWidget::DockWidgetMovable);
	this->addDockWidget(Qt::BottomDockWidgetArea, ScaTool::dockcurves);

	ScaTool::synchrodialog = new SynchroDialog(this);
	ScaTool::attackdialog = new Attackwindow(this);
	ScaTool::main_plot = ui->mainplot;
	ScaTool::statusbar = ui->statusbar;

	Chart *chart = new Chart();
	ui->mainplot->setChart(chart);
	ui->mainplot->setRenderHint(QPainter::Antialiasing);

	QMenu *menu = new QMenu(this);
	QList<QAction*> qla = QList<QAction*>();

	QAction *avg = new QAction("Average");
	avg->setCheckable(true);
	qla.append(avg);

	QAction *mi = new QAction("Min");
	mi->setCheckable(true);
	qla.append(mi);

	QAction *ma = new QAction("Max");
	ma->setCheckable(true);
	qla.append(ma);

	QAction *mima = new QAction("MinMax");
	mima->setCheckable(true);
	qla.append(mima);

	// Default is minmax
	mima->setChecked(true);
	menu->addActions(qla);
	ui->menu->setMenu(menu);
}

MainWindow::~MainWindow()
{
	delete ui;
	delete ScaTool::curve_table;
	delete ScaTool::synchrodialog;
	delete ScaTool::attackdialog;
	delete ScaTool::curve_table_model;
}

MainWindow * MainWindow::getInstance()
{
	return instance;
}

class QBinaryFileDialog : public QFileDialog
{

};

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	const QMimeData* mimeData = event->mimeData();
	if (mimeData->hasUrls())
	{
		QList<QUrl> urls = mimeData->urls();
		if (urls.size() >= 1)
		{
			QUrl url = urls.at(0);
			if (url.isValid() && (url.scheme().toLower() == "file") && mimeData->hasFormat("text/uri-list"))
			{
				event->acceptProposedAction();
			}
		}
	}
}

void MainWindow::dropEvent(QDropEvent *event)
{
	const QMimeData* mimeData = event->mimeData();
	if (mimeData->hasUrls())
	{
		QStringList fnames;
		QList<QUrl> urlList = mimeData->urls();
		Q_FOREACH(QUrl url, urlList)
		{
			fnames.append(url.toLocalFile());
		}
		load_files(fnames);
	}
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void MainWindow::load_files(QStringList files)
{
	bool aisv = ScaTool::attackdialog->isVisible();
	bool sisv = ScaTool::synchrodialog->isVisible();

	if(aisv)
		ScaTool::attackdialog->hide();
	if(sisv)
		ScaTool::synchrodialog->hide();

	ui->synchro->setEnabled(false);
	ui->attack->setEnabled(false);

	if (files.length() == 1)
	{
		QString fn = files.at(0);
		QFile file(fn.toLatin1().data());
		int row = 0;
		int col = 0;
		int size = 1;

		if(file.open(QIODevice::ReadOnly) == false)
		{
			ScaTool::statusbar->showMessage("Failed to open file(s)",2000);
			ScaTool::curve_table->setCurveRangeMax();
			ui->synchro->setEnabled(true);
			ui->attack->setEnabled(true);
			return;
		}
		int file_len = file.size();
		bool bok,rok,cok,sok;
		QString rowcol;

		QFileInfo fileinfo(file);
		QStringList entry= fileinfo.fileName().split("x");
		if(entry.length() >= 4)
		{
			row = entry.at(1).toInt(&rok,10);
			col = entry.at(2).toInt(&cok,10);
			size = atoi(entry.at(3).toStdString().c_str());
			if(!(rok & cok & (size > 0) & (row*col*size == file_len)))
			{
				QMessageBox msgbox;
				msgbox.critical(0, "Error",
								"Filename params doesn't match with file size : "+
								QString::number(file_len)+
								"\nFilename format xROWxCOLxSIZE (example tracex10x32x4.bin)");
				msgbox.show();
				row = 0;
			}

		}
		if(row == 0)
		{
			do
			{
				rowcol = QInputDialog::getText(this, tr("Please indicate your trace format"),
											   tr("Enter your trace format ROWxCOL[xSIZE]"), QLineEdit::Normal,
											   "1x"+QString::number(file_len), &bok);
				// cancel
				if (bok == false){
					ui->synchro->setEnabled(true);
					ui->attack->setEnabled(true);
					return;
				}

				QStringList entry= rowcol.split("x");
				if (entry.length() >= 2)
				{
					row = entry.at(0).toInt(&rok,10);
					col = entry.at(1).toInt(&cok,10);
				}
				if (entry.length() == 3)
				{
					size = entry.at(2).toInt(&sok,10);
				}
				else
					sok = true;

				if(!(rok & cok & sok & (size > 0) & (row*col*size == file_len)))
				{
					QMessageBox msgbox;
					msgbox.critical(0,"Error","Input doesn't match with file size : "+QString::number(file_len));
					msgbox.show();
					rowcol = "";
					row = 0;
					col = 0;
					size = 1;
					continue;
				}
			} while(bok && rowcol.isEmpty());
		}

		for (int i = 0; i < row; i ++)
		{
			QCoreApplication::processEvents();
			int idx = ScaTool::curves->length();

			Curve *curve = new Curve(idx,fn,col*size,i,true);

			// Append curve to set of all managed curves
			ScaTool::curves->append(curve);
			// Add curve to synchro table display
			ScaTool::synchrodialog->addRefItem(curve->cname);
			emit ScaTool::curve_table_model->layoutChanged();

			ScaTool::statusbar->showMessage("Loading curve ... "+QString::number(i)+"/"+QString::number(row),0);
		}
	}
	else
	{
		for (QStringList::iterator it = files.begin();
			 it != files.end(); ++it) {

			QString fn = *it;

			// Check file exist
			FILE *file = fopen(fn.toLatin1().data(),"rb");
			assert(file);
			fclose(file);

			int idx = ScaTool::curves->length();

			QCoreApplication::processEvents();

			Curve *curve = new Curve(idx,fn,0,0,false);

			// Append curve to set of all managed curves
			ScaTool::curves->append(curve);
			// Add curve to synchro table display
			ScaTool::synchrodialog->addRefItem(curve->cname);
			emit ScaTool::curve_table_model->layoutChanged();

			ScaTool::statusbar->showMessage("Loading curve ... "+QString::number(idx),0);
		}
	}
	ScaTool::statusbar->showMessage("Loading curve done",1000);
	ScaTool::curve_table->setCurveRangeMax();
	ui->synchro->setEnabled(true);
	ui->attack->setEnabled(true);
	ScaTool::attackdialog;

}

void MainWindow::on_open_pressed()
{
	QStringList fnames = QFileDialog::getOpenFileNames(this,QString("Select trace(s) to open"));
	// Open/Load file(s)
	load_files(fnames);
}

void MainWindow::on_left_pressed()
{
	QVector<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

	for (int i = 0; i < clist.length() ; i++)
		clist.at(i)->shift(-1);
}

void MainWindow::on_lleft_pressed()
{
	QVector<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

	for (int i = 0; i < clist.length() ; i++)
		clist.at(i)->shift(-10);
}

void MainWindow::on_zero_pressed()
{
	QVector<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

	for (int i = 0; i < clist.length() ; i++)
		clist.at(i)->shift(-clist.at(i)->xoffset);
}

void MainWindow::on_right_pressed()
{
	QVector<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

	for (int i = 0; i < clist.length() ; i++)
		clist.at(i)->shift(1);
}

void MainWindow::on_rright_pressed()
{
	QVector<Curve *> clist = ScaTool::curve_table->getSelectedCurves();

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
	ScaTool::curve_table->hide();
	ScaTool::main_plot->hide();

	QApplication::closeAllWindows();
	QApplication::quit();
}

void MainWindow::on_refresh_pressed()
{

	qDeleteAll(ScaTool::synchrodialog->synchropasses.begin(),ScaTool::synchrodialog->synchropasses.end());

	QVectorIterator<Curve*> i(*ScaTool::curves);
	while(i.hasNext())
	{
		Curve *c = i.next();
		delete c;
	}

	ScaTool::curves->clear();
	ScaTool::synchrodialog->clearRefItem();
	if (ScaTool::dockcurves->isHidden())
		ScaTool::dockcurves->show();
	for (int i = 0 ; i < ScaTool::main_plot->chart()->axes(Qt::Horizontal).length(); i ++)
		ScaTool::main_plot->chart()->removeAxis(ScaTool::main_plot->chart()->axes().at(i));
	for (int i = 0 ; i < ScaTool::main_plot->chart()->axes(Qt::Vertical).length(); i ++)
		ScaTool::main_plot->chart()->removeAxis(ScaTool::main_plot->chart()->axes().at(i));
	ScaTool::main_plot->chart()->removeAllSeries();
	ScaTool::curve_table->firstDisplayed = true;
	emit ScaTool::curve_table_model->layoutChanged();
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
	ScaTool::attackdialog->show();
}

void MainWindow::on_save_pressed()
{
	int curve_length = 0;
	if (ScaTool::curves->length() == 0)
		return;

	// Create traces data file
	QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),QDir::currentPath(),tr("Binary (*.bin)"));
	QFile trace(fileName);
	if (trace.open(QIODevice::WriteOnly) == false)
		return;


	QDialog dialog(this);
	QFormLayout form(&dialog);
	form.addRow(new QLabel("Enter curves number you want to save:"));

	QLineEdit *samplesEdit = new QLineEdit(&dialog);
	samplesEdit->setText(QString::number(ScaTool::curves->length()));
	form.addRow(samplesEdit);

	form.addRow(new QLabel("Enter points range you want to save:"));

	QLineEdit *startEdit = new QLineEdit(&dialog);
	startEdit->setText(QString::number(0));
	form.addRow(startEdit);
	QLineEdit *endEdit = new QLineEdit(&dialog);
	endEdit->setText(QString::number(ScaTool::curves->at(0)->length));
	form.addRow(endEdit);


	// Add some standard buttons (Cancel/Ok) at the bottom of the dialog
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
							   Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);
	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	int start = 0;
	int end = 0;
	int samples = 0;
	// Show the dialog as modal
	if (dialog.exec() == QDialog::Accepted) {
		start = startEdit->text().toInt();
		end = endEdit->text().toInt();
		samples = samplesEdit->text().toInt();
		delete startEdit;
		delete endEdit;
		delete samplesEdit;
	}
	else
	{
		delete startEdit;
		delete endEdit;
		delete samplesEdit;
		trace.close();
		return;
	}

	if ((start > end) || (samples > ScaTool::curves->length()) || (samples < 0))
		samples = 0;

	for (int i = 0; i < samples; i++)
	{
		Curve* c = ScaTool::curves->at(i);
		float * buf = c->getrawdata(&curve_length, c->xoffset);
		trace.write(reinterpret_cast<const char*>(buf+start), (end-start)<<2);
		// free buf
		free(buf);

	}
	trace.close();

}

void MainWindow::on_fity_pressed()
{
	float min = std::numeric_limits<float>::max();
	float max = -std::numeric_limits<float>::max();
	for(int i = 0; i < ScaTool::curves->length() ; i ++)
	{
		if (ScaTool::curves->at(i)->displayed)
		{
			max = std::max(max,ScaTool::curves->at(i)->max);
			min = std::min(min,ScaTool::curves->at(i)->min);
		}
	}
	if (ScaTool::main_plot->chart()->axes().length() > 0)
		ScaTool::main_plot->chart()->axisY()->setRange(min,max);
}


#include <QMenuBar>
#include <QAction>
void MainWindow::on_menu_pressed()
{
}

void MainWindow::on_color_pressed()
{

	int ct = static_cast<int>(ScaTool::main_plot->chart()->theme());
	ScaTool::main_plot->chart()->setTheme(Chart::ChartTheme((ct+1)%8));

	for(int i = 0; i < ScaTool::curves->length() ; i ++)
	{
		if (ScaTool::curves->at(i)->displayed)
		{
			Curve * curve = ScaTool::curves->at(i);
			// Dirty way to update
			curve->getDisplaySeries()->hide();
			curve->getDisplaySeries()->show();
			curve->setColor(curve->getDisplaySeries()->color());
		}
	}
	emit ScaTool::curve_table_model->layoutChanged();
}

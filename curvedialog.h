#ifndef CURVEDIALOG_H
#define CURVEDIALOG_H

#include <QDialog>
#include <QWidgetItem>

namespace Ui {
class CurveDialog;
}

class ScaTool;
class CurveDialog : public QDialog
{
    Q_OBJECT

public:
    bool firstDisplayed;
    explicit CurveDialog(QWidget *parent = 0);
    ~CurveDialog();
    QWidgetItem * getItemByName(QString name);
    QStringList * getListName();
    void addItem();
public slots:
private:
    Ui::CurveDialog *ui;
};

#endif // CURVEDIALOG_H

#ifndef ATTACKWINDOW_H
#define ATTACKWINDOW_H

#include <QWidget>

namespace Ui {
class attackwindow;
}

class attackwindow : public QWidget
{
    Q_OBJECT

public:
    explicit attackwindow(QWidget *parent = 0);
    ~attackwindow();

private:
    Ui::attackwindow *ui;
};

#endif // ATTACKWINDOW_H

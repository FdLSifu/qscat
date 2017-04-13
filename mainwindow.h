#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "scatool.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow * instance;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static MainWindow * getInstance();

private slots:
    void on_open_pressed();

    void on_left_pressed();

    void on_lleft_pressed();

    void on_zero_pressed();

    void on_right_pressed();

    void on_rright_pressed();

    void on_settings_pressed();

    void on_synchro_pressed();

    void on_refresh_pressed();

public slots:
    void updateStatusBar();

private:
    Ui::MainWindow *ui;
protected:
    void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H

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

    void on_synchro_pressed();

    void on_refresh_pressed();

    void on_curves_pressed();

    void on_attack_pressed();

    void on_save_pressed();

    void on_fity_pressed();

    void on_menu_pressed();

    void on_color_pressed();

public slots:
    void updateStatusBar();

private:
    Ui::MainWindow *ui;
    void load_files(QStringList files);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
};

#endif // MAINWINDOW_H

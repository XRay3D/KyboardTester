#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSemaphore>

class ButtonModel;
class DialogConnection;
class MatrixModel;
class Worker;
struct Pins;

class MainWindow : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

signals:
    void startTester();

private slots:
    void on_hslPress_valueChanged(int value);
    void on_pbTest_clicked(bool checked);

    void on_pbAdd_clicked();
    void on_pbSub_clicked();
    void on_pbSetPos_clicked();

    void on_pbZero_clicked();

private:
    void writeSettings();
    void readSettings();
    void updatePos(const QPointF& pt);
    void setPos(double);
    void connectionSlot(int ok);
    void createMenus();

    void finish();
    void selectRow(int);

    void setMode(bool measure);
    void setData(const Pins& value);

    static constexpr auto filter = "*.kbrd";
    ButtonModel* modelButton;
    MatrixModel* modelMatrix;
    DialogConnection* connection;
    Worker* worker = nullptr;
    int tesretTimerId = 0;
    QSemaphore tesretSemaphore;

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
};



#endif // MAINWINDOW_H

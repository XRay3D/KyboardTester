#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void on_pbPing_clicked();
    void on_pushButton_clicked();
    void on_hslPress_valueChanged(int value);

private:
    void writeSettings();
    void readSettings();
    void updatePos(const QPointF& pt);
};

#endif // MAINWINDOW_H

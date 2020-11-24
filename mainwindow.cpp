#include "mainwindow.h"
#include "buttonmodel.h"
#include "hwinterface/interface.h"

#include <QDebug>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QSettings>

//QTabWidget* tw;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setupUi(this);

    for (const QSerialPortInfo& pi : QSerialPortInfo::availablePorts()) {
        cbxTester->addItem(pi.portName());
        cbxGrbl->addItem(pi.portName());
    }
    ButtonModel* model;
    tableViewButtons->setModel(model = new ButtonModel);
    connect(pbAdd, &QPushButton::clicked, model, &ButtonModel::add);
    connect(pbSub, &QPushButton::clicked, model, &ButtonModel::sub);
    connect(pbSave, &QPushButton::clicked, model, &ButtonModel::save);
    connect(pbOpen, &QPushButton::clicked, model, &ButtonModel::open);

    //    MatrixModel* mtm = static_cast<MatrixModel*>(widget_2->tableView->model());

    connect(Interface::grbl(), &GRBL::currentPos, this, &MainWindow::updatePos);

    readSettings();
    on_pbPing_clicked();
}

MainWindow::~MainWindow()
{
    writeSettings();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("splitter", splitter->saveState());
    settings.setValue("cbxTester", cbxTester->currentIndex());
    settings.setValue("cbxGrbl", cbxGrbl->currentIndex());
    settings.setValue("hslPress", hslPress->value());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    splitter->restoreState(settings.value("splitter").toByteArray());
    cbxTester->setCurrentIndex(settings.value("cbxTester").toInt());
    cbxGrbl->setCurrentIndex(settings.value("cbxGrbl").toInt());
    hslPress->setValue(settings.value("hslPress").toInt());
    settings.endGroup();
}

void MainWindow::updatePos(const QPointF& pt)
{
    dsbxX->setValue(pt.x());
    dsbxY->setValue(pt.y());
}

void MainWindow::on_pbPing_clicked()
{
    QString str;

    pushButton->setEnabled(false);

    if (Interface::tester()->Ping(cbxTester->currentText())) {
        if (Interface::grbl()->Ping(cbxGrbl->currentText())) {
            widget_2->s.release();
            widget_2->timer.start(10);
            pushButton->setEnabled(true);
        } else {
            widget_2->timer.stop();
            str += "Grbl!\n";
        }
    } else {
        widget_2->timer.stop();
        str += "Tester!\n";
    }

    if (!str.isEmpty())
        QMessageBox::critical(this, "", str);
}

void MainWindow::on_pushButton_clicked()
{
    textBrowser->clear();

    enum {
        R1 = 0,
        R2 = 20,
        R3 = 42,
        R1_2 = 13,
        R2_3 = 38,

        C1 = 0,
        C2 = 21,
        C3 = 29,
        C4 = 38,
        C5 = 60,
        C6 = 74,
        C7 = 88,
        C8 = 102,
        C9 = 124,
        DelayMs = 50,
    };
    QVector<QPair<QString, QPointF>>
        bs {
            { "ENTER", QPointF(C1, R1) }, //enter
            { "CE", QPointF(C1, R2) }, //ce
            { "BACK", QPointF(C1, R3) }, //back

            { "FIXED STEPS", QPointF(C2, R2_3) }, //fix ste
            { "PROGR", QPointF(C4, R2_3) }, //progr
            { "FAST SLOW", QPointF(C3, R1_2) }, //fs
            { "DOWN", QPointF(C2, R1) }, //slow
            { "UP", QPointF(C4, R1) }, //fst

            { "-", QPointF(C5, R2) }, //-
            { "0", QPointF(C5, R3) }, //0
            { "7", QPointF(C6, R3) }, //7
            { "4", QPointF(C7, R3) }, //4
            { "1", QPointF(C8, R3) }, //1
            { "2", QPointF(C8, R2) }, //2
            { "5", QPointF(C7, R2) }, //5
            { "8", QPointF(C6, R2) }, //8
            { "9", QPointF(C6, R1) }, //9
            { "6", QPointF(C7, R1) }, //6
            { "3", QPointF(C8, R1) }, //3

            { "REC", QPointF(C9, R1) }, //rec
            { "LIGHT", QPointF(C9, R2) }, //light
        };

    struct TH {
        QTimer* timer;
        TH(QTimer* timer)
            : timer(timer)
        {
            qDebug(__FUNCTION__);
            timer->stop();
        }
        ~TH()
        {
            qDebug(__FUNCTION__);
            timer->start(10);
        }
    } th(&widget_2->timer);

    Interface::grbl()->setPressure(hslPress->value() * 0.1);

    int i = 0;
    for (const auto& [button, pos] : bs) {
        if (Interface::grbl()->setButton(pos)) {
            do {
                Interface::grbl()->getPos();
                thread()->msleep(DelayMs);
                QApplication::processEvents();
            } while (Interface::grbl()->isRunning());
        } else {
            qDebug("setButton!");
            break;
        }

        Interface::tester()->measure();
        QApplication::processEvents(QEventLoop::EventLoopExec, 1);

        if (int val = widget_2->isOk(); 0 < val && val < 100) {
            textBrowser->setTextColor(Qt::black);
            textBrowser->append(button + " = " + QString::number(val) + " ohm");
            ++i;
        } else {
            textBrowser->setTextColor(Qt::red);
            textBrowser->append(button + " = " + QString::number(val) + " ohm");
            break;
        }
    }

    Interface::grbl()->home();
    if (i == bs.size()) {
        textBrowser->setTextColor(Qt::darkGreen);
        textBrowser->append("Ok!");
        //        QMessageBox::information(this, "", "Ok!");
    } else {
        textBrowser->setTextColor(Qt::red);
        textBrowser->append("Err!");
        //        QMessageBox::critical(this, "", "Err!");
    }
}

void MainWindow::on_hslPress_valueChanged(int value)
{
    Interface::grbl()->setPressure(value * 0.1);
}

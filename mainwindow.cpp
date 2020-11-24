#include "mainwindow.h"
#include "buttonmodel.h"
#include "delegate.h"
#include "hwinterface/interface.h"

#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QItemDelegate>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QSettings>
#include <ranges>
//QTabWidget* tw;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //    setWindowFlag(Qt::WindowStaysOnTopHint);
    setupUi(this);

    {
        auto ports(QSerialPortInfo::availablePorts());
        std::ranges::sort(ports, [](const QSerialPortInfo& r, const QSerialPortInfo& l) {
            return r.portName().mid(3).toInt() < l.portName().mid(3).toInt();
        });
        for (const QSerialPortInfo& pi : ports) {
            cbxTester->addItem(pi.portName());
            cbxGrbl->addItem(pi.portName());
        }
    }
    {
        model = new ButtonModel;
        tableViewButtons->setModel(model);

        auto header = tableViewButtons->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);
        header = tableViewButtons->verticalHeader();
        header->setSectionResizeMode(QHeaderView::Fixed);
        { //setItemDelegate
            //            tableViewButtons->setItemDelegateForColumn(1, new Delegate(tableViewButtons));
            tableViewButtons->setItemDelegate(new Delegate(tableViewButtons));
        }

        connect(pbAdd, &QPushButton::clicked, [this] {
            QString name = QInputDialog::getText(this,
                "",
                "Enter a name for the button.", QLineEdit::Normal,
                "Name", nullptr);
            model->addButton(name, { dsbxX->value(), dsbxY->value() });
        });
        connect(pbSub, &QPushButton::clicked, [this] {
            auto mIdxList { tableViewButtons->selectionModel()->selectedIndexes() };
            if (mIdxList.size()) {
                std::ranges::sort(mIdxList, [](const QModelIndex& r, const QModelIndex& l) { return r.row() > l.row(); });
                auto answer = QMessageBox::question(this, "", "Remove Selected Rows?", QMessageBox::Yes, QMessageBox::No);
                if (answer == QMessageBox::Yes) {
                    model->removeButtons(mIdxList);
                }
            }
        });
        connect(pbSave, &QPushButton::clicked, [this] {
            if (model->fileName().isEmpty())
                model->save(QFileDialog::getSaveFileName(this, "", "", "*.bin"));
            else
                model->save();
        });
        connect(pbOpen, &QPushButton::clicked, [this] {
            if (model->fileName().isEmpty())
                model->open(QFileDialog::getOpenFileName(this, "", "", "*.bin"));
            else
                model->open();
        });
    }

    connect(dsbxX, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double x) {
        Interface::grbl()->setButton({ x, dsbxY->value() });
    });
    connect(dsbxY, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double y) {
        Interface::grbl()->setButton({ dsbxX->value(), y });
    });

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
    //    textBrowser->clear();

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
        model->addButton(button, pos);
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
            //            textBrowser->setTextColor(Qt::black);
            //            textBrowser->append(button + " = " + QString::number(val) + " ohm");
            ++i;
        } else {
            //            textBrowser->setTextColor(Qt::red);
            //            textBrowser->append(button + " = " + QString::number(val) + " ohm");
            break;
        }
    }

    Interface::grbl()->home();
    if (i == bs.size()) {
        //        textBrowser->setTextColor(Qt::darkGreen);
        //        textBrowser->append("Ok!");
        //        QMessageBox::information(this, "", "Ok!");
    } else {
        //        textBrowser->setTextColor(Qt::red);
        //        textBrowser->append("Err!");
        //        QMessageBox::critical(this, "", "Err!");
    }
}

void MainWindow::on_hslPress_valueChanged(int value)
{
    Interface::grbl()->setPressure(value * 0.1);
}

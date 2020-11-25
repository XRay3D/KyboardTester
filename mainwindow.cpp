#include "mainwindow.h"
#include "buttonmodel.h"
#include "delegate.h"
#include "dialogconnection.h"
#include "hwinterface/interface.h"
#include "matrixmodel.h"
#include "worker.h"

#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QItemDelegate>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QSettings>
#include <algorithm>
#include <ranges>

enum {
    TesterTimeout = 100,
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , connection(new DialogConnection(this))
{
    //    setWindowFlag(Qt::WindowStaysOnTopHint);
    setupUi(this);

    connect(Interface::grbl(), &GRBL::currentPos, this, &MainWindow::updatePos);

    connect(Interface::tester(), &AmkTester::measureReady, this, &MainWindow::setData);
    connect(this, &MainWindow::startTester, Interface::tester(), &AmkTester::measure);

    connect(connection, &DialogConnection::connection, this, &MainWindow::connectionSlot);

    // tableViewTester
    modelMatrix = new MatrixModel;
    tableViewTester->setModel(modelMatrix);
    {
        auto header = tableViewTester->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);
        header = tableViewTester->verticalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);
    }

    // tableViewButtons
    modelButton = new ButtonModel;
    tableViewButtons->setModel(modelButton);
    tableViewButtons->setItemDelegate(new Delegate(tableViewButtons));
    {
        auto header = tableViewButtons->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);
        header = tableViewButtons->verticalHeader();
        header->setSectionResizeMode(QHeaderView::Fixed);
    }
    connect(modelButton, &ButtonModel::dataChanged, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
        if (topLeft.column() < 2 && bottomRight.column() < 2 && roles.contains(Qt::EditRole))
            setWindowModified(true);
    });

    // menu
    createMenus();

    connect(dsbxX, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double x) {
        if (chbxMove->isChecked())
            Interface::grbl()->setButton({ x, dsbxY->value() });
    });
    connect(dsbxY, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double y) {
        if (chbxMove->isChecked())
            Interface::grbl()->setButton({ dsbxX->value(), y });
    });

    readSettings();
    if (!connection->ping())
        connection->show();
}

MainWindow::~MainWindow()
{
    killTimer(tesretTimerId);
    writeSettings();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("splitter", splitter->saveState());
    settings.setValue("cbxTester", connection->cbxTester->currentIndex());
    settings.setValue("cbxGrbl", connection->cbxGrbl->currentIndex());
    settings.setValue("hslPress", hslPress->value());
    settings.setValue("fileName", modelButton->fileName());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    splitter->restoreState(settings.value("splitter").toByteArray());
    connection->cbxTester->setCurrentIndex(settings.value("cbxTester").toInt());
    connection->cbxGrbl->setCurrentIndex(settings.value("cbxGrbl").toInt());
    hslPress->setValue(settings.value("hslPress").toInt());
    modelButton->open(settings.value("fileName").toString());
    setWindowFilePath(QFileInfo(modelButton->fileName()).fileName());

    settings.endGroup();
}

void MainWindow::updatePos(const QPointF& pt)
{
    dsbxX->setValue(pt.x());
    dsbxY->setValue(pt.y());
}

void MainWindow::setPos(double)
{
}

void MainWindow::connectionSlot(int ok)
{
    switch (ok) {
    case DialogConnection::T:
        tesretTimerId = startTimer(TesterTimeout);
        tesretSemaphore.acquire(tesretSemaphore.available());
        break;
    case DialogConnection::G:
        pbTest->setEnabled(true);
        break;
    case DialogConnection::T | DialogConnection::G:
        tesretTimerId = startTimer(TesterTimeout);
        tesretSemaphore.acquire(tesretSemaphore.available());
        pbTest->setEnabled(true);
    default:
        killTimer(tesretTimerId);
        tesretTimerId = 0;
        pbTest->setEnabled(false);
        break;
    }
}

void MainWindow::createMenus()
{
    // Новый
    auto action = menu->addAction(QIcon::fromTheme(""), "Новый", [this] {
        modelButton->newOrClose();
        setWindowFilePath(QFileInfo(modelButton->fileName()).fileName());
        setWindowModified(true);
    });
    action->setShortcut(QKeySequence::New);
    // Открыть
    action = menu->addAction(QIcon::fromTheme(""), "Открыть", [this] {
        modelButton->open(QFileDialog::getOpenFileName(this, "Open File", modelButton->fileName(), filter));
        setWindowFilePath(QFileInfo(modelButton->fileName()).fileName());
        setWindowModified(false);
    });
    action->setShortcut(QKeySequence::Open);
    // Сохранить
    action = menu->addAction(QIcon::fromTheme(""), "Сохранить", [this] {
        modelButton->fileName().isEmpty()
            ? modelButton->save(QFileDialog::getSaveFileName(this, "Save File", "", filter))
            : modelButton->save();
        setWindowFilePath(QFileInfo(modelButton->fileName()).fileName());
        setWindowModified(false);
    });
    action->setShortcut(QKeySequence::Save);
    // Сохранить как
    action = menu->addAction(QIcon::fromTheme(""), "Сохранить как...", [this] {
        modelButton->save(QFileDialog::getSaveFileName(this, "Save File As", modelButton->fileName(), filter));
        setWindowFilePath(QFileInfo(modelButton->fileName()).fileName());
        setWindowModified(false);
    });
    action->setShortcut(QKeySequence::SaveAs);
    // Закрыть
    action = menu->addAction(QIcon::fromTheme(""), "Закрыть", [this] {
        modelButton->newOrClose();
        setWindowFilePath(QFileInfo(modelButton->fileName()).fileName());
        setWindowModified(true);
    });
    action->setShortcut(QKeySequence::Close);
    // Выход
    action = menu->addAction(QIcon::fromTheme(""), "Выход", [this] { close(); });
}

void MainWindow::finish()
{
    setMode(false);
    QMessageBox::information(this, "", "Проверка закончена.");
}

void MainWindow::selectRow(int row)
{
    tableViewButtons->selectionModel()->clearSelection();
    tableViewButtons->selectRow(row);
}

void MainWindow::setMode(bool measure)
{
    chbxMove->setChecked(false);
    grbxButtons->setEnabled(!measure);
    grbxRule->setEnabled(!measure);

    pbTest->setText(measure ? "Прервать тест" : "Начать тест");
    pbTest->setChecked(measure);

    if (measure) {
        killTimer(tesretTimerId);
        tesretTimerId = 0;
    } else {
        tesretSemaphore.acquire(tesretSemaphore.available());
        tesretTimerId = startTimer(TesterTimeout);
    }
}

void MainWindow::setData(const Pins& value)
{
    modelMatrix->setDataA(value);
    tesretSemaphore.acquire(tesretSemaphore.available());
}

void MainWindow::on_hslPress_valueChanged(int value)
{
    Interface::grbl()->setPressure(value * 0.1);
}

void MainWindow::on_pbTest_clicked(bool checked)
{
    setMode(checked);
    if (checked) {
        worker = new Worker(modelButton);
        connect(worker, &QThread::finished, &QObject::deleteLater);
        connect(worker, &QThread::finished, this, &MainWindow::finish);
        connect(worker, &Worker::currentRow, this, &MainWindow::selectRow);

        worker->start(QThread::NormalPriority);
    } else {
        worker->requestInterruption();
        worker->quit();
        worker->wait();
    }

    //    setMode(checked);
    //    if (checked) {
    //        class TH {
    //            // clang-format off
    //            MainWindow* const mw;
    //        public:
    //            TH(MainWindow* mw) : mw(mw) {
    //                killTimer(tesretTimerId);
    //                tesretTimerId = 0;
    //            }
    //            ~TH() {
    //                tesretSemaphore.acquire(tesretSemaphore.available());
    //                tesretTimerId = startTimer(TesretTimeout);
    //            }
    //            // clang-format on
    //        } th(this);

    //        Interface::grbl()->setPressure(hslPress->value() * 0.1);
    //        int index = 0;
    //        for (const auto& [button, pos, res] : modelButton->buttons()) {
    //            tableViewButtons->selectionModel()->clearSelection();
    //            tableViewButtons->selectRow(index);
    //            if (!pbTest->isChecked()) {
    //                Interface::grbl()->home();
    //                return;
    //            }
    //            if (Interface::grbl()->setButton(pos)) {
    //                do {
    //                    Interface::grbl()->getPos();
    //                    thread()->msleep(DelayMs);
    //                    QApplication::processEvents();
    //                } while (Interface::grbl()->isRunning());
    //            } else {
    //                qDebug("setButton!");
    //                break;
    //            }

    //            Interface::tester()->measure();
    //            QApplication::processEvents(QEventLoop::EventLoopExec, 1);

    //            int val = modelMatrix->isOk();
    //            modelButton->setResistance(index++, val);
    //        }
    //        Interface::grbl()->home();
    //        setMode(false);
    //        QMessageBox::information(this, "", "Проверка закончена.");
    //    }
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == tesretTimerId) {
        if (!tesretSemaphore.available()) {
            emit startTester();
            tesretSemaphore.release();
        }
    }
}

void MainWindow::on_pbAdd_clicked()
{
    QString name = QInputDialog::getText(this,
        "",
        "Enter a name for the button.", QLineEdit::Normal,
        "Name", nullptr);
    modelButton->addButton(name, { dsbxX->value(), dsbxY->value() });
    setWindowModified(true);
}

void MainWindow::on_pbSub_clicked()
{
    auto mIdxList { tableViewButtons->selectionModel()->selectedIndexes() };
    if (mIdxList.size()) {
        //std::ranges::sort(mIdxList, [](const QModelIndex& r, const QModelIndex& l) { return r.row() > l.row(); });
        std::sort(mIdxList.begin(), mIdxList.end(), [](const QModelIndex& r, const QModelIndex& l) { return r.row() > l.row(); });
        auto answer = QMessageBox::question(this, "", "Remove Selected Rows?", QMessageBox::Yes, QMessageBox::No);
        if (answer == QMessageBox::Yes) {
            modelButton->removeButtons(mIdxList);
            setWindowModified(true);
        }
    }
}

void MainWindow::on_pbSetPos_clicked()
{
    auto mIdxList { tableViewButtons->selectionModel()->selectedIndexes() };
    enum Flags {
        N,
        X,
        Y
    };
    int flag = (chbxX->isChecked() ? X : N) | (chbxY->isChecked() ? Y : N);
    if (flag != N) {
        for (auto& index : mIdxList) {
            if (index.column() == 1) {
                auto pos(index.data(Qt::EditRole).toPointF());
                if (flag == X)
                    modelButton->setData(index, QPoint(dsbxX->value(), pos.y()));
                else if (flag == Y)
                    modelButton->setData(index, QPoint(pos.x(), dsbxY->value()));
                else if (flag == (X | Y))
                    modelButton->setData(index, QPoint(dsbxX->value(), dsbxY->value()));
            }
            emit modelButton->dataChanged(index, index);
            setWindowModified(true);
        }
    }
}

void MainWindow::on_pbZero_clicked()
{
    Interface::grbl()->zero();
    chbxMove->setChecked(false);
    dsbxX->setValue(0);
    dsbxY->setValue(0);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (isWindowModified()) {
        switch (QMessageBox::question(this, "", "", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel)) {
        case QMessageBox::Yes:
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            break;
        };
    }
}

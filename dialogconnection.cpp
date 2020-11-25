#include "dialogconnection.h"

#include "hwinterface/interface.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QSerialPortInfo>

DialogConnection::DialogConnection(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(pbPing, &QPushButton::clicked, this, &DialogConnection::ping);
    {
        auto ports { QSerialPortInfo::availablePorts() };
        // std::ranges::sort(ports, [](const QSerialPortInfo& r, const QSerialPortInfo& l) {
        //     return r.portName().mid(3).toInt() < l.portName().mid(3).toInt();
        // });
        std::sort(ports.begin(), ports.end(), [](const QSerialPortInfo& r, const QSerialPortInfo& l) {
            return r.portName().mid(3).toInt() < l.portName().mid(3).toInt();
        });
        for (const QSerialPortInfo& pi : ports) {
            cbxTester->addItem(pi.portName());
            cbxGrbl->addItem(pi.portName());
        }
    }
}

DialogConnection::~DialogConnection() { }

bool DialogConnection::ping()
{
    QString str;
    if (Interface::tester()->Ping(cbxTester->currentText())) {
        if (Interface::grbl()->Ping(cbxGrbl->currentText())) {
            emit connection(G);
        } else {
            str += "Grbl!\n";
        }
        emit connection(T);
    } else {
        str += "Tester!\n";
    }

    if (!str.isEmpty()) {
        emit connection(0);
        QMessageBox::critical(this, "", str);
    }
    return str.isEmpty();
}


#pragma once

#include "common_interfaces.h"
#include <QSemaphore>
#include <QSerialPort>
#include <QThread>

class Port;

class GRBL : public QObject, public CommonInterfaces {
    Q_OBJECT

signals:
    void currentPos(const QPointF& pos);
    friend class Port;

    void write(const QByteArray& data);
    void open();
    void close();

public:
    GRBL();
    ~GRBL() override;

    // CommonInterfaces interface
    bool Ping(const QString& portName, int baud = 115200, int addr = 0) override;
    bool setButton(QPointF pos);
    bool setPressure(double val);
    bool home();
    QPointF getPos();
    bool isRunning() { return run; }

private:
    Port* port;
    QThread portThread;

    QSemaphore semaphore;
    QString ret;

    bool run = true;

    bool isSuccess(int timeout = 1000);

    double pressure = 0.0;
};

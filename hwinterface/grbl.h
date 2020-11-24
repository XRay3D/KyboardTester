#pragma once

#include "common_interfaces.h"
#include <QAnimationGroup>
#include <QPointF>
#include <QSemaphore>
#include <QSerialPort>
#include <QThread>

class Port;

class GRBL : public QObject, public CommonInterfaces {
    Q_OBJECT

    friend class Port;

#ifdef EMU
    Q_PROPERTY(QPointF currentPos READ getCurrentPos WRITE setCurrentPos NOTIFY currentPos2)
    QPointF m_currentPos;
    QPointF getCurrentPos() const { return m_currentPos; }
    void setCurrentPos(const QPointF& value) { m_currentPos = value, emit currentPos2(m_currentPos); }
signals:
    void currentPos2(const QPointF& pos);
#endif

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

signals:
    void currentPos(const QPointF& pos);
    void write(const QByteArray& data);
    void open();
    void close();

private:
    Port* port;
    QThread portThread;

    QSemaphore semaphore;
    QString ret;

    bool run = true;

    bool isSuccess(int timeout = 1000);

    double pressure = 0.0;
};

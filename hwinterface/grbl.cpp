#include "grbl.h"

#include <QDateTime>
#include <QDebug>
#include <QPointF>
#include <QPropertyAnimation>
#include <QTextStream>
#include <QTimerEvent>

/*

error:X: Something went wrong! Grbl did not recognize the command and did not execute anything inside that message. The X is given as a numeric error code to tell you exactly what happened. The table below decribes every one of them.

ID 	Error Code Description
1 	G-code words consist of a letter and a value. Letter was not found.
2 	Numeric value format is not valid or missing an expected value.
3 	Grbl '$' system command was not recognized or supported.
4 	Negative value received for an expected positive value.
5 	Homing cycle is not enabled via settings.
6 	Minimum step pulse time must be greater than 3usec
7 	EEPROM read failed. Reset and restored to default values.
8 	Grbl '$' command cannot be used unless Grbl is IDLE. Ensures smooth operation during a job.
9 	G-code locked out during alarm or jog state
10 	Soft limits cannot be enabled without homing also enabled.
11 	Max characters per line exceeded. Line was not processed and executed.
12 	(Compile Option) Grbl '$' setting value exceeds the maximum step rate supported.
13 	Safety door detected as opened and door state initiated.
14 	(Grbl-Mega Only) Build info or startup line exceeded EEPROM line length limit.
15 	Jog target exceeds machine travel. Command ignored.
16 	Jog command with no '=' or contains prohibited g-code.
17 	Laser mode requires PWM output.
20 	Unsupported or invalid g-code command found in block.
21 	More than one g-code command from same modal group found in block.
22 	Feed rate has not yet been set or is undefined.
23 	G-code command in block requires an integer value.
24 	Two G-code commands that both require the use of the XYZ axis words were detected in the block.
25 	A G-code word was repeated in the block.
26 	A G-code command implicitly or explicitly requires XYZ axis words in the block, but none were detected.
27 	N line number value is not within the valid range of 1 - 9,999,999.
28 	A G-code command was sent, but is missing some required P or L value words in the line.
29 	Grbl supports six work coordinate systems G54-G59. G59.1, G59.2, and G59.3 are not supported.
30 	The G53 G-code command requires either a G0 seek or G1 feed motion mode to be active. A different motion was active.
31 	There are unused axis words in the block and G80 motion mode cancel is active.
32 	A G2 or G3 arc was commanded but there are no XYZ axis words in the selected plane to trace the arc.
33 	The motion command has an invalid target. G2, G3, and G38.2 generates this error, if the arc is impossible to generate or if the probe target is the current position.
34 	A G2 or G3 arc, traced with the radius definition, had a mathematical error when computing the arc geometry. Try either breaking up the arc into semi-circles or quadrants, or redefine them with the arc offset definition.
35 	A G2 or G3 arc, traced with the offset definition, is missing the IJK offset word in the selected plane to trace the arc.
36 	There are unused, leftover G-code words that aren't used by any command in the block.
37 	The G43.1 dynamic tool length offset command cannot apply an offset to an axis other than its configured axis. The Grbl default axis is the Z-axis.
38 	Tool number greater than max supported value.


*/
const QPointF nullPt { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };

class Port : public QSerialPort {
    void readSlot()
    {
        data.append(readLine().trimmed());
        if (data.endsWith("ok") || data.endsWith("Grbl 1.1f ['$' for help]")) {
            qDebug() << data;
            grbl->ret = data;
            data.clear();
            grbl->semaphore.release();
        }
    }
    GRBL* grbl;
    QByteArray data;
    int timerId = 0;

public:
    Port(GRBL* grbl)
        : grbl(grbl)
    {
        setBaudRate(Baud115200);
        setFlowControl(NoFlowControl);
        //        connect(this, &QIODevice::readyRead, this, &Port::readSlot);
    }
    ~Port()
    {
        killTimer(timerId);
        timerId = 0;
        close();
    }
    void Write(const QByteArray& data)
    {
        this->data.clear();
        grbl->semaphore.acquire(grbl->semaphore.available());
        readAll();
        QSerialPort::write(data);
    }
    void Open()
    {
        if (open(ReadWrite)) {
            timerId = startTimer(10);
            grbl->semaphore.release();
        }
    }
    void Close()
    {
        killTimer(timerId);
        timerId = 0;
        close();
        grbl->semaphore.release();
    }

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override
    {
        if (event->timerId() == timerId)
            readSlot();
    }
};

GRBL::GRBL()
    : port(new Port(this))
{
    connect(this, &GRBL::open, port, &Port::Open, Qt::QueuedConnection);
    connect(this, &GRBL::close, port, &Port::Close, Qt::QueuedConnection);
    connect(this, &GRBL::write, port, &Port::Write, Qt::QueuedConnection);

    port->moveToThread(&portThread);
    connect(&portThread, &QThread::finished, port, &QObject::deleteLater);
    portThread.start(QThread::NormalPriority);
}

GRBL::~GRBL()
{
    portThread.quit();
    portThread.wait();
}

bool GRBL::Ping(const QString& portName, int /*baud*/, int /*addr*/)
{
#ifdef EMU
    return m_connected = true;
#endif
    m_connected = false;
    semaphore.acquire(semaphore.available());
    do {
        emit close();
        if (!isSuccess())
            break;

        port->setPortName(portName);

        emit open();
        if (!isSuccess())
            break;

        {
            emit write(QByteArray(1, 0x18));
            if (!isSuccess(2000))
                break;
            if (!ret.contains("Grbl 1.1f ['$' for help]"))
                break;
        }

        emit write("G21 G17 G90\r");
        if (!isSuccess())
            break;

        m_connected = true;
    } while (0);

    return m_connected;
}

bool GRBL::setButton(QPointF pos)
{
#ifdef EMU
    run = true;
    QPropertyAnimation* animation = new QPropertyAnimation(this, "currentPos");
    animation->moveToThread(thread());
    animation->setDuration(100);
    animation->setStartValue(m_currentPos);
    animation->setEndValue(pos + QPointF { 1, 1 });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    connect(animation, &QAbstractAnimation::finished, [this] { run = false; });
    connect(animation, &QVariantAnimation::valueChanged, [](const QVariant& value) { qDebug() << value; });
    return m_connected = true;
#endif
    if (!m_connected)
        return false;
    QString str;
    do {
        //        str = QString("G1Z%1\r").arg(0);
        //        if (!wr(str))
        //            break;

        str = QString("G1Z%1F2000\r").arg(-pressure);
        emit write(str.toLocal8Bit());
        if (!isSuccess())
            break;

        str = QString("G0X%1Y%2\r").arg(pos.x()).arg(pos.y());
        emit write(str.toLocal8Bit());
        return isSuccess();

    } while (0);
    return false;
}

bool GRBL::setPressure(double val)
{
#ifdef EMU
    return m_connected = true;
#endif

    if (!m_connected)
        return false;
    pressure = val;
    emit write(QString("G1Z%1F2000\r").arg(-pressure).toLocal8Bit());
    return isSuccess();
}

bool GRBL::home()
{
#ifdef EMU
    return m_connected = true;
#endif

    if (!m_connected)
        return false;
    emit write("G0Z0\r");
    if (isSuccess()) {
        emit write("G0X0Y0\r");
        return isSuccess();
    }

    return false;
}

bool GRBL::zero()
{
    emit write("G92X0Y0\r");
    return isSuccess();
}

QPointF GRBL::getPos()
{
#ifdef EMU
    emit currentPos(m_currentPos);
    return m_currentPos;
#endif

    if (!m_connected)
        return nullPt;
    emit write("?\r");
    if (isSuccess(10000)) {
        static const QRegExp rx("(.+):(-?\\d+\\.\\d+),(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)");
        QStringList list(ret.split('|'));
        run = true;
        if (rx.exactMatch(list.value(1))) {
            run = list.value(0).contains("Run");
            QPointF pt { rx.cap(2).toDouble(), rx.cap(3).toDouble() };
            emit currentPos(pt);
            return pt;
        }
    }
    return nullPt;
}

bool GRBL::isSuccess(int timeout) { return semaphore.tryAcquire(1, timeout); }

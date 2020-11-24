#include "amk_tester.h"
#include <QDebug>

const int id1 = qRegisterMetaType<QVector<uint16_t>>("QVector<uint16_t>");
const int id2 = qRegisterMetaType<Pins>("Pins");

AmkTester::AmkTester(QObject* parent)
    : QObject(parent)
    , port(new TesterPort(this))
    , pinsParcels {
        parcel(MEASURE_PIN, uint8_t(0x0)),
        parcel(MEASURE_PIN, uint8_t(0x1)),
        parcel(MEASURE_PIN, uint8_t(0x2)),
        parcel(MEASURE_PIN, uint8_t(0x3)),
        parcel(MEASURE_PIN, uint8_t(0x4)),
        parcel(MEASURE_PIN, uint8_t(0x5)),
        parcel(MEASURE_PIN, uint8_t(0x6)),
        parcel(MEASURE_PIN, uint8_t(0x7)),
        parcel(MEASURE_PIN, uint8_t(0x8)),
        parcel(MEASURE_PIN, uint8_t(0x9)),
        parcel(MEASURE_PIN, uint8_t(0xA)),
    }
{
    port->moveToThread(&m_portThread);
    connect(&m_portThread, &QThread::finished, port, &QObject::deleteLater);
    m_portThread.start(QThread::InheritPriority);
}

AmkTester::~AmkTester()
{
    m_portThread.quit();
    m_portThread.wait();
}

bool AmkTester::Ping(const QString& portName, int baud, int addr)
{
    Q_UNUSED(baud)
    Q_UNUSED(addr)
    QMutexLocker locker(&m_mutex);
    m_connected = false;
    reset();
    do {
        emit close();
        if (!m_semaphore.tryAcquire(1, 1000))
            break;

        if (!portName.isEmpty())
            port->setPortName(portName);

        emit open(QIODevice::ReadWrite);
        if (!m_semaphore.tryAcquire(1, 1000))
            break;

        emit write(parcel(static_cast<uint8_t>(PING)));
        if (!m_semaphore.tryAcquire(1, 100)) {
            emit close();
            break;
        }

        m_connected = true;
    } while (0);
    return m_connected;
}

bool AmkTester::measure()
{
    QMutexLocker locker(&m_mutex);
    if (!m_connected)
        return false;
    m_result = true;
    m_semaphore.acquire(m_semaphore.available());
    for (auto& pinsParcel : pinsParcels) {
        emit write(pinsParcel);
        m_result &= m_semaphore.tryAcquire(1, 1000);
        if (!m_result)
            break;
    }
    return m_result;
}

bool AmkTester::getCalibrationCoefficients(float& /*GradCoeff*/, int /*pin*/)
{
    QMutexLocker locker(&m_mutex);
    if (!m_connected)
        return false;
    return (m_result = false);
}

bool AmkTester::setCalibrationCoefficients(float& /*GradCoeff*/, int /*pin*/)
{
    QMutexLocker locker(&m_mutex);
    if (!m_connected)
        return false;
    return (m_result = false);
}

void AmkTester::reset()
{
    m_result = false;
    m_semaphore.acquire(m_semaphore.available());
}

void AmkTester::rxPing(const QByteArray& /*data*/) { qDebug() << "rxPing"; }

void AmkTester::rxMeasurePin(const QByteArray& data)
{
    enum { ColumnCount = 11 };
    static auto setDataS = [this](const QVector<int>& value) {
        QVector<int> data;
        const int row = value[ColumnCount];
        int rez[ColumnCount] { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
        int count = 0;
        for (int column = row - 1, index = row + 1;
             column > -1 || index < ColumnCount;
             --column, ++index) {

            float value1 = value[row];
            if (column > -1 && value[column] > 10) { // 10 this anti-noise
                float value2 = value[column];
                ++count;
                if (value1 > value2)
                    rez[column] = ((value1 - value2) / value2) * 1000; // 1000 опорное сопротивление
                else
                    rez[column] = ((value2 - value1) / value1) * 1000; // 1000 опорное сопротивление
                data.append(rez[column]);
                if (rez[row] < 0)
                    ++rez[row];
            }
            if (index < 11 && value[index] > 10) { // 10 this anti-noise
                float value2 = value[index];
                ++count;
                if (value1 > value2)
                    rez[index] = ((value1 - value2) / value2) * 1000; // 1000 опорное сопротивление
                else
                    rez[index] = ((value2 - value1) / value1) * 1000; // 1000 опорное сопротивление
                data.append(rez[index]);
                if (rez[row] < 0)
                    ++rez[row];
            }
        }

        for (int column = 0; column < ColumnCount; ++column) {
            if (count && rez[column] > 0 && column < row) {
                rez[column] /= count;
            }
            m_pins[row][column] = rez[column];
        }
    };

    const uint16_t* d = pValue<const uint16_t>(data);
    for (int i = 0; i < ColumnCount; ++i) {
        dataMatrix[d[ColumnCount]][i] = d[i];
    }
    if (d[ColumnCount] == 10) {
        for (int i = 0; i < ColumnCount; ++i) {
            setDataS({ //
                dataMatrix[i][0],
                dataMatrix[i][1],
                dataMatrix[i][2],
                dataMatrix[i][3],
                dataMatrix[i][4],
                dataMatrix[i][5],
                dataMatrix[i][6],
                dataMatrix[i][7],
                dataMatrix[i][8],
                dataMatrix[i][9],
                dataMatrix[i][10],
                i });
        }
        emit measureReady(m_pins);
    }
}

void AmkTester::rxGetCalibrationCoefficients(const QByteArray& /*data*/) { qDebug() << "rxGetCalibrationCoefficients"; }

void AmkTester::rxSetCalibrationCoefficients(const QByteArray& /*data*/) { qDebug() << "rxSetCalibrationCoefficients"; }

void AmkTester::rxBufferOverflow(const QByteArray& data) { qDebug() << "rxBufferOverflow" << data.toHex().toUpper(); }

void AmkTester::rxWrongCommand(const QByteArray& data) { qDebug() << "rxWrongCommand" << data.toHex().toUpper(); }

void AmkTester::rxCrcError(const QByteArray& data) { qDebug() << "rxCrcError" << data.toHex().toUpper(); }

/////////////////////////////////////////
/// \brief SerialPort::SerialPort
/// \param amkTester
///
TesterPort::TesterPort(AmkTester* t)
    : m_t(t)
    , m_f(QVector<TesterPort::func>(0x100, &AmkTester::rxWrongCommand))
{
    m_f[PING] = &AmkTester::rxPing;
    m_f[MEASURE_PIN] = &AmkTester::rxMeasurePin;
    m_f[GET_CALIBRATION_COEFFICIENTS] = &AmkTester::rxGetCalibrationCoefficients;
    m_f[SET_CALIBRATION_COEFFICIENTS] = &AmkTester::rxSetCalibrationCoefficients;
    m_f[BUFFER_OVERFLOW] = &AmkTester::rxBufferOverflow;
    m_f[WRONG_COMMAND] = &AmkTester::rxWrongCommand;
    m_f[CRC_ERROR] = &AmkTester::rxCrcError;

    setBaudRate(Baud115200);
    setDataBits(Data8);
    setFlowControl(NoFlowControl);
    setParity(NoParity);

    connect(t, &AmkTester::open, this, &TesterPort::openSlot);
    connect(t, &AmkTester::close, this, &TesterPort::closeSlot);
    connect(t, &AmkTester::write, this, &TesterPort::writeSlot);
    //    //get the virtual table pointer of object obj
    //    int* vptr = *(int**)amkTester;
    //    // we shall call the function fn, but first the following assembly code
    //    //  is required to make obj as 'this' pointer as we shall call
    //    //  function fn() directly from the virtual table
    //    //__asm mov ecx, amkTester;
    //    amkTester;
    //    //function fn is the first entry of the virtual table, so it's vptr[0]
    //    ((void (*)(const QByteArray&))vptr[0])(QByteArray("0123456789"));

    //    typedef void (AmkTester::*func)(const QByteArray&);
    //    CallBack* ptr = t;
    //    func* vptr = *(func**)(ptr);
    //    (t->*vptr[0])(QByteArray("0123456789"));
    connect(this, &QSerialPort::readyRead, this, &TesterPort::readSlot, Qt::DirectConnection);
}

void TesterPort::openSlot(int mode)
{
    QMutexLocker locker(&m_mutex);
    if (open(static_cast<OpenMode>(mode)))
        m_t->m_semaphore.release();
}

void TesterPort::closeSlot()
{
    QMutexLocker locker(&m_mutex);
    close();
    m_t->m_semaphore.release();
}

void TesterPort::writeSlot(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    counter += write(data);
    //qDebug() << counter;
}

void TesterPort::readSlot()
{
    QMutexLocker locker(&m_mutex);
    m_data.append(readAll());
    for (int i = 0; i < m_data.size() - 3; ++i) {
        const Parcel* const d = reinterpret_cast<const Parcel*>(m_data.constData() + i);
        if (d->start == RX && d->len <= m_data.size()) {
            QByteArray tmpData = m_data.mid(i, d->len);
            counter += tmpData.size();
            if (checkParcel(tmpData))
                (m_t->*m_f[d->cmd])(tmpData);
            else
                (m_t->*m_f[CRC_ERROR])(tmpData);

            m_t->m_semaphore.release();
            m_data.remove(0, i + d->len);
            i = 0;
        }
    }
}

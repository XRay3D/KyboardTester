#ifndef MY_PROTOCOL_H
#define MY_PROTOCOL_H

#include "common_interfaces.h"
#include "myprotokol.h"
#include <QElapsedTimer>
#include <QMutex>
#include <QSemaphore>
#include <QSerialPort>
#include <QThread>
#include <QVector>

enum COMMAND {
    PING,
    MEASURE_PIN,
    GET_CALIBRATION_COEFFICIENTS,
    SET_CALIBRATION_COEFFICIENTS,
    BUFFER_OVERFLOW,
    WRONG_COMMAND,
    CRC_ERROR
};

struct Pins {
    struct Row {
        int data[16] { -1 };
        int& operator[](int i) { return data[i]; }
        const int& operator[](int i) const { return data[i]; }
    } data[16];
    Row& operator[](int i) { return data[i]; }
    const Row& operator[](int i) const { return data[i]; }
};

class TesterPort;

class AmkTester : public QObject, private MyProtokol, public CommonInterfaces {
    Q_OBJECT
    friend class TesterPort;

public:
    AmkTester(QObject* parent = nullptr);
    ~AmkTester();

    bool Ping(const QString& portName = QString(), int baud = 9600, int addr = 0);

    bool measure();
    //    bool setDefaultCalibrationCoefficients(uint8_t pin);
    //    bool getCalibrationCoefficients(float& GradCoeff, int pin);
    //    bool setCalibrationCoefficients(float& GradCoeff, int pin);
    //    bool saveCalibrationCoefficients(uint8_t pin);

signals:
    void open(int mode);
    void close();
    void write(const QByteArray& data);
    void measureReady(const Pins&);

private:
    TesterPort* port;
    QMutex m_mutex;
    QSemaphore m_semaphore;
    QThread m_portThread;

    void reset();

    const QByteArray pinsParcels[11];

    Pins m_pins;
    bool m_result = false;
    int m_counter = 0;
    int dataMatrix[11][11];

    void rxPing(const QByteArray& data);
    void rxMeasurePin(const QByteArray& data);
    void rxGetCalibrationCoefficients(const QByteArray& data);
    void rxSetCalibrationCoefficients(const QByteArray& data);
    void rxBufferOverflow(const QByteArray& data);
    void rxWrongCommand(const QByteArray& data);
    void rxCrcError(const QByteArray& data);
};

class TesterPort : public QSerialPort, private MyProtokol {
    Q_OBJECT

public:
    TesterPort(AmkTester* t);
    void openSlot(int mode);
    void closeSlot();
    void writeSlot(const QByteArray& data);
    AmkTester* m_t;
    typedef void (AmkTester::*func)(const QByteArray&);
    QVector<func> m_f;

private:
    void readSlot();
    QByteArray m_data;
    QMutex m_mutex;
    qint64 counter = 0;
};

#endif // MY_PROTOCOL_H

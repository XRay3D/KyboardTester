#pragma once

#define EMU

#include <QString>

class CommonInterfaces {
public:
    virtual bool Ping(const QString& portName = QString(), int baud = 9600, int addr = 0) = 0;
    virtual bool IsConnected() const { return m_connected; }

protected:
    bool m_connected = false;
};


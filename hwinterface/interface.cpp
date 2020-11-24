#include "interface.h"

#include <QTimer>

Interface::Interface()
{
    if (!semafore.available()) {
        m_tester = new AmkTester;
        m_tester->moveToThread(&thread);
        thread.connect(&thread, &QThread::finished, m_tester, &QObject::deleteLater);

        m_grbl = new GRBL;
        m_grbl->moveToThread(&thread);
        thread.connect(&thread, &QThread::finished, m_grbl, &QObject::deleteLater);

        thread.start(QThread::NormalPriority);
    }
    semafore.release();
}

Interface::~Interface()
{
    semafore.acquire();
    if (!semafore.available()) {
        thread.quit();
        thread.wait();
    }
}

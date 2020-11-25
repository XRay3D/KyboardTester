#include "worker.h"
#include "buttonmodel.h"
#include "hwinterface/interface.h"
#include <QApplication>
#include <QDebug>

enum {
    DelayMs = 50
};

Worker::Worker(ButtonModel* modelButton, QObject* parent)
    : QThread(parent)
    , modelButton(modelButton)
{
    setObjectName("Worker");
}

Worker::~Worker() { qDebug(__FUNCTION__); }

void Worker::run()
{
    int index = 0;
    for (const auto& [button, pos, res] : modelButton->buttons()) {
        emit currentRow(index);
        if (Interface::grbl()->setButton(pos)) {
            do {
                Interface::grbl()->getPos();
                msleep(DelayMs);
                QApplication::processEvents();
                if (isInterruptionRequested()) {
                    Interface::grbl()->home();
                    return;
                }
            } while (Interface::grbl()->isRunning());
        } else {
            qDebug("setButton!");
            break;
        }

        Interface::tester()->measure();

        int val = Interface::tester()->resistance();
        modelButton->setResistance(index++, val);
    }
    Interface::grbl()->home();
}

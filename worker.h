#pragma once

#include <QThread>
class ButtonModel;

class Worker : public QThread {
    Q_OBJECT

    ButtonModel* modelButton;

public:
    explicit Worker(ButtonModel* modelButton, QObject* parent = nullptr);
    ~Worker();

signals:
    void currentRow(int);

    // QThread interface
protected:
    void run() override;
};

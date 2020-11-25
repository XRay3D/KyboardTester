#pragma once

#include "ui_dialogconnection.h"
#include <QDialog>

//namespace Ui {
//class DialogConnection;
//}

class DialogConnection : public QDialog, public Ui::DialogConnection {
    Q_OBJECT

public:
    explicit DialogConnection(QWidget* parent = nullptr);
    ~DialogConnection();
    bool ping();

    enum {
        T = 1,
        G = 2
    };

signals:
    void connection(int);
};

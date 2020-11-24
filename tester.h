#ifndef TESTER_H
#define TESTER_H

#include "matrixmodel.h"

#include <QSemaphore>
#include <QTableView>
#include <QTimer>
#include <QWidget>

#include <hwinterface/amk_tester.h>

namespace Ui {
class Tester;
}

class QPushButton;
class MyTableWidget;

class TESTER : public QWidget {
    Q_OBJECT
    friend class MainWindow;

public:
    explicit TESTER(QWidget* parent = nullptr);
    ~TESTER() override;

    int isOk() { return model->isOk(); }

signals:
    void Measure();

private:
    Ui::Tester* ui;
    QTimer timer;

    QTableView* tableView;
    MatrixModel* model;
    QSemaphore s;

    void setupUi(QWidget* Form);
    void retranslateUi(QWidget* Form);
    void SetValue(const QVector<uint16_t>& value);
    void SetValueA(const Pins& value);

    //    // QWidget interface
    //protected:
    //    void resizeEvent(QResizeEvent* event) override;
};

#endif // TESTER_H

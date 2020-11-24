#include "tester.h"
//#include "ui_tester.h"
#include "hwinterface/interface.h"

#include <QDebug>
#include <QHeaderView>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

TESTER::TESTER(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    connect(Interface::tester(), &AmkTester::measureReady, this, &TESTER::SetValueA);
    connect(this, &TESTER::Measure, Interface::tester(), &AmkTester::measure);
    connect(&timer, &QTimer::timeout, [&]() {
        if (s.tryAcquire()) {
            emit Measure();
        }
    });
}

TESTER::~TESTER()
{
    timer.stop();
}

void TESTER::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QStringLiteral("Form"));
    Form->resize(557, 301);

    tableView = new QTableView(Form);
    tableView->setObjectName(QStringLiteral("tableView"));

    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout* verticalLayout = new QVBoxLayout(Form);
    verticalLayout->setMargin(0);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->addWidget(tableView);
    model = new MatrixModel(tableView);
    tableView->setModel(model);

    retranslateUi(Form);

    QMetaObject::connectSlotsByName(Form);
}

void TESTER::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle("Form");
}

void TESTER::SetValue(const QVector<uint16_t>& /*value*/)
{
    // model->setData(value);
    s.release();
}

void TESTER::SetValueA(const Pins& value)
{
    model->setDataA(value);
    s.release();
}

//#include <QHeaderView>
//void TESTER::resizeEvent(QResizeEvent* event)
//{
//    QFont f;
//    f.setPixelSize(tableView->height() / 24);
//    tableView->setFont(f);
//    QWidget::resizeEvent(event);
//}

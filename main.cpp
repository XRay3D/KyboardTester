#include "hwinterface/interface.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("XrSoft");
    a.setApplicationName("KyboardTester");
    {
        QFont f;
        f.setPointSize(10);
        a.setFont(f);
    }
    Interface i;
    Q_UNUSED(i)

    MainWindow w;
    w.show();

    return a.exec();
}

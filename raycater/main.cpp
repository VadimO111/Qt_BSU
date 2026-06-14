#include "mainwindow.h"

#include <QApplication>
#include <QSize>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow window;
    window.setWindowTitle("Raycaster");
    window.resize(QSize(900, 600));
    window.show();
    return a.exec();
}

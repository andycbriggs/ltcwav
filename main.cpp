#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(":/stylesheet.qss");

    file.open(QFile::ReadOnly);
    QString styleSheet(file.readAll());

    a.setStyleSheet(styleSheet);

    MainWindow w;

    w.setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    w.show();

    return a.exec();
}

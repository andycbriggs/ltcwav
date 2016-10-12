#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_buttonSave_clicked();
    void on_comboFrameRate_activated(int index);

private:
    void checkAndSetDropFrameDisabledState();
    void checkAndSetMaxFrameNumber();

    double frameRate();
    int baseFrameRate();

    int getSampleRate();
    int bitRate();

    int nChannels();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

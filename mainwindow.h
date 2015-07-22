#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>

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
    void on_btnStart_clicked();
    void socketReady();
private:
    void start();
    void stop();

    Ui::MainWindow *ui;
    QUdpSocket m_socket;
    bool m_bRunning;
};

#endif // MAINWINDOW_H

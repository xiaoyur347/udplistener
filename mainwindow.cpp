#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QNetworkInterface>
#include <QDebug>
#include <stdio.h>

#define LOG_DIR "../log"

static void WriteLog(const QString &ip, const QString &now, const QByteArray &data)
{
    QString fileName = QString(LOG_DIR "/%1.txt").arg(ip);
    QFile file(fileName);
    file.open(QIODevice::Append);
    char szBuffer[128];
    snprintf(szBuffer, sizeof(szBuffer), "[%s]", now.toStdString().c_str());
    file.write(szBuffer);
    file.write(data);
    file.close();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_bRunning(false)
{
    QDir dir;
    dir.mkpath(LOG_DIR);

    ui->setupUi(this);

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("ip"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("time"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("log"));
    ui->tableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
    ui->tableWidget->setColumnWidth(0, 100);
    ui->tableWidget->setColumnWidth(1, 150);
    ui->tableWidget->setColumnWidth(2, 500);

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface i, list)
    {
        //qDebug() << i;
        if (i.flags() & QNetworkInterface::IsLoopBack)
        {
            continue;
        }
        if (!(i.flags() & QNetworkInterface::IsUp))
        {
            continue;
        }
        QString str;
        QList<QNetworkAddressEntry> lst = i.addressEntries();
        foreach (QNetworkAddressEntry addr, lst)
        {
            if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                str = addr.ip().toString() + "(" + i.hardwareAddress() + ")";
                ui->cmbIp->addItem(str);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    stop();
}

void MainWindow::on_btnStart_clicked()
{
    if (!m_bRunning)
    {
        start();
    }
    else
    {
        stop();
    }
}

void MainWindow::start()
{
    if (m_socket.bind(ui->editPort->text().toInt()))
    {
        connect(&m_socket, SIGNAL(readyRead()), this, SLOT(socketReady()));
        m_bRunning = true;
        ui->btnStart->setText("Stop");
    }
}

void MainWindow::stop()
{
    if (m_bRunning)
    {
        m_socket.close();
        m_bRunning = false;
        ui->btnStart->setText("Start");
    }
}

void MainWindow::socketReady()
{
    while (m_socket.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_socket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_socket.readDatagram(datagram.data(), datagram.size(),
                              &sender, &senderPort);

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(row+1);
        QString ip = sender.toString();
        QString now = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(ip));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(now));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(datagram.data()));
        WriteLog(ip, now, datagram);
    }
    ui->tableWidget->scrollToBottom();
}

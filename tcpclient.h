#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    static TcpClient &getInstance();
    QTcpSocket &getTcpSocket();
    QString loginName();
    QString curPath();

public slots:
    void showConnect();
    void recvMsg();
private slots:
    //void on_sendButton_clicked();

    void on_loginPushButton_clicked();

    void on_registPushButton_clicked();

    void on_cancelPushButton_clicked();

private:
    Ui::TcpClient *ui;
    QString mStrIp;
    quint16 mUsPort;
    //连接服务器和服务器进行交互
    QTcpSocket mTcpSocket;
    QString m_strLoginName;//登录的用户名

    QString m_strCurPath;
};
#endif // TCPCLIENT_H

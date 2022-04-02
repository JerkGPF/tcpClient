#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QHostAddress>
#include <QMessageBox>
#include "opewidget.h"
#include "privatechat.h"

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();
    //connect(&mTcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));
    connect(&mTcpSocket,&QTcpSocket::connected,[=](){
        this->showConnect();
    });
    //信号与槽关联，当有消息过来则调用recvMsg处理
    connect(&mTcpSocket,&QTcpSocket::readyRead,[=](){
        this->recvMsg();
    });
    //连接服务器
    mTcpSocket.connectToHost(QHostAddress(mStrIp),mUsPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

//读取配置文件
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();//转换为字符串
        file.close();//读取完毕，关闭流
        strData.replace("\r\n"," ");//替换文件中的字符
        QStringList strList = strData.split(" ");//按照空格切分；
        //        foreach (auto info, strList) {//遍历输出
        //            qDebug()<<"==>>"<<info;
        //        }
        mStrIp = strList[0];
        mUsPort = strList[1].toUShort();
        //        qDebug()<<mStrIp;
        //        qDebug()<<mUsPort;

    }
    else
    {
        QMessageBox::critical(this,"配置文件","配置文件读取失败");

    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return mTcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}
//接受服务端返回数据处理函数
void TcpClient::recvMsg()
{
    qDebug() << mTcpSocket.bytesAvailable();
    uint uiPDULen = 0;
    mTcpSocket.read((char*)&uiPDULen,sizeof (uint));
    uint uiMsgLen = uiPDULen - sizeof (PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    mTcpSocket.read((char*)pdu+sizeof (uint),uiPDULen-sizeof (uint));
    switch (pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(strcmp(pdu->caData,REGIST_OK)==0)
            QMessageBox::information(this,"注册",REGIST_OK);
        else if(strcmp(pdu->caData,REGIST_FAILED)==0)
            QMessageBox::warning(this,"注册",REGIST_FAILED);
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(strcmp(pdu->caData,LOGIN_OK)==0)
        {
            m_strCurPath = QString("./%1").arg(m_strLoginName);

            QMessageBox::information(this,"登录",LOGIN_OK);
            OpeWidget::getInstance().show();
            this->hide();
        }
        else if(strcmp(pdu->caData,LOGIN_FAILED)==0)
            QMessageBox::warning(this,"登录",LOGIN_FAILED);
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {
        OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
    {
        if(strcmp(SEARCH_USR_NO,pdu->caData)==0)
        {
            QMessageBox::information(this,"搜索",QString("%1:not exist")
                                     .arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if (strcmp(SEARCH_USR_ONLINE,pdu->caData)==0)
        {
            QMessageBox::information(this,"搜索",QString("%1:online")
                                     .arg(OpeWidget::getInstance().getFriend()->m_strSearchName));

        }
        else if (strcmp(SEARCH_USR_OFFLINE,pdu->caData)==0)
        {
            QMessageBox::information(this,"搜索",QString("%1:offline")
                                     .arg(OpeWidget::getInstance().getFriend()->m_strSearchName));

        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData+32,32);
        int ret = QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend?").arg(caName),
                                 QMessageBox::Yes,QMessageBox::No);

        PDU *respdu = mkPDU(0);
        memcpy(respdu->caData,pdu->caData,32);
        if(ret == QMessageBox::Yes)
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;//同意添加好友
        else
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;//拒绝添加好友
        mTcpSocket.write((char*)respdu,respdu->uiPDULen);//发送数据
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        char caPerName[32] = {'\0'};
        memcpy(caPerName, pdu->caData, 32);
        QMessageBox::information(this, "添加好友", QString("添加%1好友成功").arg(caPerName));
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
    {
        char caPerName[32] = {'\0'};
        memcpy(caPerName, pdu->caData, 32);
        QMessageBox::information(this, "添加好友", QString("添加%1好友失败").arg(caPerName));
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
    {

        OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {
        char caName[32] = {'\0'};
        memcpy(caName, pdu->caData, 32);
        QMessageBox::information(this, "删除好友", QString("%1 删除你作为他的好友").arg(caName));
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        QMessageBox::information(this, "删除好友", "删除好友成功");
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
            PrivateChat::getInstance().updateMsg(pdu);
        }

        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
    {
        QMessageBox::information(this,"创建文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
    {
        OpeWidget::getInstance().getBook()->updateFileList(pdu);
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;
}

#if 0
void TcpClient::on_sendButton_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        //        PDU *pdu = mkPDU(strMsg.size()+1);
        PDU *pdu = mkPDU(strMsg.size());
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        mTcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    }
    else
    {
        QMessageBox::warning(this,"信息发送","发送信息不能为空");

    }
}
#endif

void TcpClient::on_loginPushButton_clicked()
{
    QString strName = ui->nameLineEdit->text();
    QString strPwd = ui->pwdLineEdit->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        mTcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    }
    else
    {
        QMessageBox::critical(this,"登录","登录失败:用户名或密码为空");
    }
}

void TcpClient::on_registPushButton_clicked()
{
    QString strName = ui->nameLineEdit->text();
    QString strPwd = ui->pwdLineEdit->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        mTcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    }
    else
    {
        QMessageBox::critical(this,"注册","注册失败:用户名或密码为空");
    }

}

void TcpClient::on_cancelPushButton_clicked()
{

}

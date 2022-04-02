#include "online.h"
#include "ui_online.h"
#include <QDebug>
#include "tcpclient.h"
Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if(pdu==NULL)
        return;
    uint uiSize = pdu->uiMsgLen/32;
    char caTmp[32];
    for(uint i = 0;i<uiSize;i++)
    {
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        qDebug()<<caTmp;
        ui->onlineListWidget->addItem(caTmp);
    }
}

void Online::on_addFriendButton_clicked()
{
    QListWidgetItem *pItem = ui->onlineListWidget->currentItem();
    QString strPerUserName = pItem->text();
    QString strLoginName = TcpClient::getInstance().loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,strPerUserName.toStdString().c_str(),strPerUserName.size());
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

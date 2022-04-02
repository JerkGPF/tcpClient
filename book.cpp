#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
Book::Book(QWidget *parent) : QWidget(parent)
{
    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回") ;
    m_pCreaterDirPB = new QPushButton("创建文件夹") ;
    m_pDelDirPB = new QPushButton("删除文件夹") ;
    m_pRenamePB = new QPushButton("重命名文件") ;
    m_pFlushFilePB = new QPushButton("刷新文件") ;

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreaterDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    m_pUploadPB = new QPushButton("上传文件") ;
    m_pDownLoadPB = new QPushButton("下载文件") ;
    m_pDelFilePB = new QPushButton("删除文件") ;
    m_pShareFilePB = new QPushButton("共享文件") ;

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreaterDirPB,&QPushButton::clicked,[=](){createDir();});

}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","新文件夹名字");
    if(!strNewDir.isEmpty())
    {
        if(strNewDir.size()>32)
            QMessageBox::warning(this,"新建文件夹","新文件夹不能超过32个字符");
        else
        {
            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath = TcpClient::getInstance().curPath();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
    else
        QMessageBox::warning(this,"新建文件夹","新文件夹不能为空");

}
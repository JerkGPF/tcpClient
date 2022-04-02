#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
    mPListW = new QListWidget(this);
    mPListW->addItem("好友");
    mPListW->addItem("图书");

    m_pFriend = new Friend;
    m_pBook = new Book;
    m_pSW = new QStackedWidget;
    m_pSW->addWidget(m_pFriend);
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(mPListW);
    pMain->addWidget(m_pSW);
    setLayout(pMain);
    connect(mPListW,&QListWidget::currentRowChanged,m_pSW,&QStackedWidget::setCurrentIndex);
}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}
#include "echoclient.h"

EchoClient::EchoClient(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(new QTabWidget)
    , m_messages(new QTableWidget)
    , m_inputMessageEdit(new QLineEdit)
    , m_sendButton(new QPushButton(tr("Send")))
    , m_modifyButton(new QPushButton(tr("Modify")))
    , m_deleteButton(new QPushButton(tr("Delete")))
{
    //constructing user tab
    QWidget *userWidget = new QWidget;
    QGridLayout *userLayout = new QGridLayout;

    m_messages->setColumnCount(2);
    m_messages->setHorizontalHeaderLabels(
                {tr("User"), tr("Message")}
                );
    m_messages->verticalHeader()->setVisible(false);
    m_messages->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_messages->setSelectionMode(QAbstractItemView::SingleSelection);

    userLayout->addWidget(m_messages, 0, 0, 4, 4);
    userLayout->addWidget(m_modifyButton, 0, 4);
    userLayout->addWidget(m_deleteButton, 1, 4);
    userLayout->addWidget(m_inputMessageEdit, 4, 0, 1, 4);
    userLayout->addWidget(m_sendButton, 4, 4);

    userWidget->setLayout(userLayout);
    //

    m_tabWidget->addTab(userWidget, tr("Messages"));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_tabWidget);

    this->setLayout(mainLayout);
}

void EchoClient::show()
{
    ConnectDialog conDial(this);
    if (conDial.exec() != QDialog::Accepted && false)
    {
        close();
        return;
    }
    m_tcpSocket = conDial.getSocket();
    QWidget::show();
}

EchoClient::~EchoClient()
{

}

#include "echoclient.h"


EchoClient::EchoClient(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(new QTabWidget)
    , m_messages(new QTableWidget)
    , m_inputMessageEdit(new QLineEdit)
    , m_modifyButton(new QPushButton(tr("Modify")))
    , m_deleteButton(new QPushButton(tr("Delete")))
    , m_userList(new QListWidget)
    , m_moderatorList(new QListWidget)
{
    QPushButton *sendButton = new QPushButton(tr("Send"));

    //constructing user tab
    QWidget *userWidget = new QWidget;
    QGridLayout *userLayout = new QGridLayout;

    m_messages->setColumnCount(3);
    m_messages->setHorizontalHeaderLabels(
                {tr("User"), tr("_Message ID"), tr("Message")}
                );
    m_messages->verticalHeader()->setVisible(false);
    m_messages->hideColumn(1);
    m_messages->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_messages->setSelectionMode(QAbstractItemView::SingleSelection);
    m_messages->horizontalHeader()->setStretchLastSection(true);

    userLayout->addWidget(m_messages, 0, 0, 4, 4);
    userLayout->addWidget(m_modifyButton, 0, 4);
    userLayout->addWidget(m_deleteButton, 1, 4);
    userLayout->addWidget(m_inputMessageEdit, 4, 0, 1, 4);
    userLayout->addWidget(sendButton, 4, 4);

    userWidget->setLayout(userLayout);
    //

    //construct admin tab
    QWidget *adminWidget = new QWidget;
    QHBoxLayout *moderatorLayout = new QHBoxLayout;

    const QStyle *style = QApplication::style();

    QPushButton *toModButton = new QPushButton;
    toModButton->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
    QPushButton *toUserButton = new QPushButton;
    toUserButton->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
    QVBoxLayout *buttonsLayout = new QVBoxLayout;

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(toModButton);
    buttonsLayout->addWidget(toUserButton);
    buttonsLayout->addStretch();

    QVBoxLayout *modsL = new QVBoxLayout;
    modsL->addWidget(new QLabel(tr("Moderators")));
    modsL->addWidget(m_moderatorList);

    QVBoxLayout *usersL = new QVBoxLayout;
    usersL->addWidget(new QLabel(tr("Users")));
    usersL->addWidget(m_userList);

    moderatorLayout->addLayout(modsL, 1);
    moderatorLayout->addLayout(buttonsLayout);
    moderatorLayout->addLayout(usersL, 1);

    adminWidget->setLayout(moderatorLayout);
    //

    m_tabWidget->addTab(userWidget, tr("Messages"));
    m_tabWidget->addTab(adminWidget, tr("Moderators"));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_tabWidget);

    this->setLayout(mainLayout);


    connect(sendButton, SIGNAL(clicked(bool)), SLOT(sendMessage()));
}

void EchoClient::show()
{
    ConnectDialog conDial(this);
    if (conDial.exec() != QDialog::Accepted)
    {
        close();
        return;
    }

    m_tcpSocket = conDial.getSocket();
    connect(m_tcpSocket, SIGNAL(readyRead()), SLOT(readServerResponse()));
    connect(m_tcpSocket, SIGNAL(disconnected()), SLOT(serverDisconected()));
    QWidget::show();

}

EchoClient::~EchoClient()
{

}

void EchoClient::sendMessage()
{
    qDebug() << "Client: send new message";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::ADD_MESSAGE << m_inputMessageEdit->text();
    m_tcpSocket->write(block);
    m_tcpSocket->flush();
}

void EchoClient::serverDisconected()
{
    QMessageBox::information(this, tr("Disconnected"),
            tr("Disconnected from the server"));
    close();
}

void EchoClient::readServerResponse()
{
    QDataStream in(m_tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    QChar ind;

    in.startTransaction();
    if (!in.commitTransaction())
        return;

    in >> ind;

    if(ind == PROTOCOL::ADD_MESSAGE)
    {
        QString text, name;
        int messageId;
        in >> messageId >> name >> text;
        addMessage(messageId, name, text);
    }

}

void EchoClient::addMessage(int messageId,
                            const QString& user, const QString& text)
{
    qDebug() << "Client: get new message";
    int rowNum = m_messages->rowCount();
    m_messages->insertRow(rowNum);

    QTableWidgetItem* items[3];
    items[0] = new QTableWidgetItem(user);
    items[1] = new QTableWidgetItem(QString::number(messageId));
    items[2] = new QTableWidgetItem(text);

    for (int i = 0; i < 3; ++i)
    {
        items[i]->setFlags(items[i]->flags() & (~Qt::ItemIsEditable));
        m_messages->setItem(rowNum, i, items[i]);
    }
}


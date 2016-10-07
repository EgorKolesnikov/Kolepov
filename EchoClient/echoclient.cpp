#include "echoclient.h"


EchoClient::EchoClient(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(new QTabWidget)
    , m_messages(new QTableWidget)
    , m_inputMessageEdit(new QLineEdit)
    , m_modifyButton(new QPushButton(tr("Modify")))
    , m_deleteButton(new QPushButton(tr("Delete")))
    , m_sendButton(new QPushButton(tr("Send")))
    , m_userList(new QListWidget)
    , m_moderatorList(new QListWidget)
    , m_adminWidget(new QWidget)
{
    m_sendButton->setEnabled(false);

    //constructing user tab
    QWidget *userWidget = new QWidget;
    QVBoxLayout *userLayout = new QVBoxLayout;

    m_messages->setColumnCount(3);
    m_messages->setRowCount(0);
    m_messages->setHorizontalHeaderLabels(
                {tr("User"), tr("_Message ID"), tr("Message")}
                );
    m_messages->verticalHeader()->setVisible(false);
    m_messages->hideColumn(1);
    m_messages->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_messages->setSelectionMode(QAbstractItemView::SingleSelection);
    m_messages->horizontalHeader()->setStretchLastSection(true);

    QVBoxLayout *btns = new QVBoxLayout;
    QPixmap modifyIcon(":/icons/modify.png");
    m_modifyButton->setIcon(modifyIcon);
    m_modifyButton->setIconSize(modifyIcon.size());
    btns->addWidget(m_modifyButton);

    m_deleteButton->setIcon(QPixmap(":/icons/delete.png"));
    m_deleteButton->setIconSize(modifyIcon.size());
    btns->addWidget(m_deleteButton);
    btns->addStretch();

    QHBoxLayout *firstRow = new QHBoxLayout;
    firstRow->addWidget(m_messages, 1);
    firstRow->addLayout(btns);

    QHBoxLayout *secondRow = new QHBoxLayout;
    secondRow->addWidget(m_inputMessageEdit, 1);
    m_sendButton->setIcon(QPixmap(":/icons/send_message.png"));
    m_sendButton->setIconSize(modifyIcon.size());
    secondRow->addWidget(m_sendButton);

    userLayout->addLayout(firstRow, 1);
    userLayout->addLayout(secondRow);

//    userLayout->addWidget(m_messages, 0, 0, 4, 4);
//    userLayout->addWidget(m_modifyButton, 0, 4);
//    userLayout->addWidget(m_deleteButton, 1, 4);
//    userLayout->addWidget(m_inputMessageEdit, 4, 0, 1, 4);
//    userLayout->addWidget(m_sendButton, 4, 4);

    m_deleteButton->setVisible(false);
    m_modifyButton->setVisible(false);

    userWidget->setLayout(userLayout);
    //

    //construct admin tab
    QHBoxLayout *moderatorLayout = new QHBoxLayout;

    QPushButton *toModButton = new QPushButton;
    QPixmap leftArrowIcon(":/icons/left_arrow.png");
    toModButton->setIcon(leftArrowIcon);
    toModButton->setIconSize(leftArrowIcon.size());
    QPushButton *toUserButton = new QPushButton;
    toUserButton->setIcon(QPixmap(":/icons/right_arrow.png"));
    toUserButton->setIconSize(leftArrowIcon.size());
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

    m_adminWidget->setLayout(moderatorLayout);
    //

    m_tabWidget->addTab(userWidget,
                        QPixmap(":/icons/message.png"), tr("Messages"));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_tabWidget);

    this->setLayout(mainLayout);

    resize(776, 480);
    m_inputMessageEdit->setFocus();


    connect(m_sendButton, SIGNAL(clicked(bool)),
            SLOT(sendMessage())
            );
    connect(m_inputMessageEdit, SIGNAL(returnPressed()),
            SLOT(sendMessage())
            );
    connect(m_inputMessageEdit, SIGNAL(textChanged(QString)),
            SLOT(enableSendButton())
            );
    connect(m_deleteButton, SIGNAL(clicked(bool)),
            SLOT(deleteMessageRequest())
            );
    connect(m_modifyButton, SIGNAL(clicked(bool)),
            SLOT(modifyRequest())
            );
    connect(toModButton, SIGNAL(clicked(bool)),
            SLOT(addModeratorRequest())
            );
    connect(toUserButton, SIGNAL(clicked(bool)),
            SLOT(deleteModeratorRequest())
            );
}

void EchoClient::enableSendButton()
{
    m_sendButton->setEnabled(!m_inputMessageEdit->text().isEmpty());
}

void EchoClient::show()
{
    ConnectDialog *dialog = new ConnectDialog(this);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(dialogResult(int)),
            Qt::QueuedConnection);
    dialog->exec();
    dialog->deleteLater();
}

void EchoClient::dialogResult(int code){
    ConnectDialog *dialog = (ConnectDialog*)QObject::sender();

    if(dialog->result() == QDialog::Accepted){
        m_tcpSocket = dialog->socket();
        connect(m_tcpSocket, SIGNAL(readyRead()), SLOT(readServerResponse()));
        connect(m_tcpSocket, SIGNAL(disconnected()), SLOT(serverDisconected()));
        setWindowTitle(tr("Client - %1").arg(dialog->username()));
        QWidget::show();
    }
    else{
        QApplication::quit();
    }
}

EchoClient::~EchoClient()
{

}

void EchoClient::sendMessage()
{
    if (m_inputMessageEdit->text().isEmpty())
        return;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::ADD_MESSAGE << m_inputMessageEdit->text();

    m_inputMessageEdit->clear();

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
    else if(ind == PROTOCOL::SEND_INIT_MESSAGES)
    {
        QString text, name;
        int messageId;
        qint32 numOfMessages;
        in >> numOfMessages;
        for(int i = 0; i < numOfMessages; ++i)
        {
            in >> messageId >> name >> text;
            addMessage(messageId, name, text);
        }

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out << PROTOCOL::GOT_MESSAGE_LIST;
        m_tcpSocket->write(block);
        m_tcpSocket->flush();

    }
    else if (ind == PROTOCOL::DELETE_MESSAGE)
    {
        int messageId;
        in >> messageId;
        deleteMessageResponse(messageId);
    }
    else if (ind == PROTOCOL::MODIFY_MESSAGE)
    {
        int messageId;
        QString newText;
        in >> messageId >> newText;
        modifyResponse(messageId, newText);
    }
    else if (ind == PROTOCOL::USER_MODERATOR_LIST)
    {
        qint32 numOfMembers;
        QString role;
        QString name;
        in >> numOfMembers;
        for (int i = 0; i < numOfMembers; ++i)
        {
            in >> name >> role;
            if (role == "m")
                m_moderatorList->addItem(name);
            else if (role == "u")
                m_userList->addItem(name);
        }

        enableAdminMode();
    }
    else if (ind == PROTOCOL::SET_NEW_MODERATOR)
    {
        enableModeratorMode();
        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->setWindowTitle(tr("Moderation"));
        msgBox->setText(tr("Now you are a moderator"));
        msgBox->setIcon(QMessageBox::Information);
        msgBox->show();
    }
    else if (ind == PROTOCOL::DELETE_MODERATOR)
    {
        disableModeratorMode();

        QMessageBox* msgBox = new QMessageBox(this);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->setWindowTitle(tr("Moderation"));
        msgBox->setText(tr("Now you are not a moderator"));
        msgBox->setIcon(QMessageBox::Information);
        msgBox->show();

    }
    else if (ind == PROTOCOL::INIT_MODERATOR)
    {
        enableModeratorMode();
    }


}

void EchoClient::addMessage(int messageId,
                            const QString& user, const QString& text)
{
    int rowNum = m_messages->rowCount();
    m_messages->insertRow(rowNum);
    m_messageIdRowNum[messageId] = rowNum;

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

void EchoClient::deleteMessageRequest()
{
    int curRow = m_messages->currentRow();
    if (curRow == -1)
        return;

    int messageId = m_messages->
            item(curRow, 1)->data(QVariant::Int).toInt();

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::DELETE_MESSAGE << messageId;
    m_tcpSocket->write(block);
    m_tcpSocket->flush();
}

void EchoClient::deleteMessageResponse(int messageId)
{
    int deleteRowNum = m_messageIdRowNum[messageId];
    m_messages->removeRow(deleteRowNum);
    m_messageIdRowNum.remove(messageId);
    for (auto it = m_messageIdRowNum.begin();
         it != m_messageIdRowNum.end(); ++it)
    {
        if (it.value() > deleteRowNum)
            --it.value();
    }
}

void EchoClient::modifyRequest()
{
    int curRow = m_messages->currentRow();
    if (curRow == -1)
        return;

    int messageId = m_messages->
            item(curRow, 1)->data(QVariant::Int).toInt();
    const QString& text = m_messages->
            item(curRow, 2)->text();

    bool ok;
    QString newText = QInputDialog::getText(this,
                                            tr("Modify"),
                                            tr("New text:"),
                                            QLineEdit::Normal,
                                            text,
                                            &ok);
    if (ok && !newText.isEmpty() && newText != text)
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out << PROTOCOL::MODIFY_MESSAGE
            << messageId
            << newText;

        m_tcpSocket->write(block);
        m_tcpSocket->flush();
    }
}

void EchoClient::modifyResponse(int messageId, QString text)
{
    if (!m_messageIdRowNum.contains(messageId))
        return;

    m_messages->item(m_messageIdRowNum[messageId], 2)->
            setText(text);

}

//for admin only
void EchoClient::addModeratorRequest()
{
    if (m_userList->currentItem() == Q_NULLPTR)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::SET_NEW_MODERATOR
        << m_userList->currentItem()->text();

    m_tcpSocket->write(block);
    m_tcpSocket->flush();

    m_moderatorList->addItem(m_userList->currentItem()->text());
    m_userList->takeItem(m_userList->currentRow());

}

void EchoClient::deleteModeratorRequest()
{
    if (m_moderatorList->currentItem() == Q_NULLPTR)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    out << PROTOCOL::DELETE_MODERATOR
        << m_moderatorList->currentItem()->text();

    m_tcpSocket->write(block);
    m_tcpSocket->flush();

    m_userList->addItem(m_moderatorList->currentItem()->text());
    m_moderatorList->takeItem(m_moderatorList->currentRow());
}

void EchoClient::enableAdminMode()
{
    enableModeratorMode();
    m_tabWidget->addTab(m_adminWidget,
                        QPixmap(":/icons/moderator.png"), tr("Moderators"));
}

void EchoClient::enableModeratorMode()
{
    m_deleteButton->setVisible(true);
    m_modifyButton->setVisible(true);
}

void EchoClient::disableModeratorMode()
{
    m_deleteButton->setVisible(false);
    m_modifyButton->setVisible(false);
}



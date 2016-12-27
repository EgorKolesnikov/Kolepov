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
    , m_notesWidget(new QWidget)
    , m_notes(new QTableWidget)
    , m_inputNoteEdit(new QLineEdit)
    , m_addNoteButton(new QPushButton(tr("Add")))
    , m_deleteNoteButton(new QPushButton(tr("Delete")))
    , m_modifyNoteButton(new QPushButton(tr("Modify")))
    , m_refreshNotesButton(new QPushButton(tr("Refresh")))
    , database_password(QString(""))
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


    // construct notes tab
    QVBoxLayout *notesLayout = new QVBoxLayout;

    m_notes->setColumnCount(2);
    m_notes->setRowCount(0);
    m_notes->setHorizontalHeaderLabels({tr("ID"), tr("Note")});
    m_notes->verticalHeader()->setVisible(false);
    m_notes->hideColumn(0);
    m_notes->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_notes->setSelectionMode(QAbstractItemView::SingleSelection);
    m_notes->horizontalHeader()->setStretchLastSection(true);

    QVBoxLayout *notes_btns = new QVBoxLayout;
    m_modifyNoteButton->setIcon(modifyIcon);
    m_modifyNoteButton->setIconSize(modifyIcon.size());
    notes_btns->addWidget(m_modifyNoteButton);

    m_deleteNoteButton->setIcon(QPixmap(":/icons/delete.png"));
    m_deleteNoteButton->setIconSize(modifyIcon.size());
    notes_btns->addWidget(m_deleteNoteButton);
    notes_btns->addStretch();

    notes_btns->addWidget(m_refreshNotesButton);
    notes_btns->addStretch();

    QHBoxLayout *firstNotesRow = new QHBoxLayout;
    firstNotesRow->addWidget(m_notes, 1);
    firstNotesRow->addLayout(notes_btns);

    QHBoxLayout *secondNotesRow = new QHBoxLayout;
    secondNotesRow->addWidget(m_inputNoteEdit, 1);
    m_addNoteButton->setIcon(QPixmap(":/icons/send_message.png"));
    m_addNoteButton->setIconSize(modifyIcon.size());
    secondNotesRow->addWidget(m_addNoteButton);

    notesLayout->addLayout(firstNotesRow, 1);
    notesLayout->addLayout(secondNotesRow);
    m_notesWidget->setLayout(notesLayout);


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
    m_tabWidget->addTab(userWidget, QPixmap(":/icons/message.png"), tr("Messages"));
    m_tabWidget->addTab(m_notesWidget, QPixmap(":/icons/modify.png"), tr("Notes"));

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
    connect(m_addNoteButton, SIGNAL(clicked(bool)),
            SLOT(addNoteRequest())
            );
    connect(m_deleteNoteButton, SIGNAL(clicked(bool)),
            SLOT(deleteNoteRequest())
            );
    connect(m_modifyNoteButton, SIGNAL(clicked(bool)),
            SLOT(modifyNoteRequest())
            );
    connect(m_refreshNotesButton, SIGNAL(clicked(bool)),
            SLOT(refreshNotesRequest())
            );
    connect(m_tabWidget, SIGNAL(tabBarClicked(int)),
            SLOT(tabBarTabChanged(int))
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

        username = dialog->username();
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

    m_tcpSocket->writeBlock(block);
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
    QDataStream con(m_tcpSocket);
    con.setVersion(QDataStream::Qt_4_0);

    QChar ind;

    con.startTransaction();
    if (!con.commitTransaction())
        return;

    while (!m_tcpSocket->peek(1).isEmpty())
    {
        QDataStream in(m_tcpSocket->readBlock().second);
        in.setVersion(QDataStream::Qt_4_0);
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
            m_tcpSocket->writeBlock(block);
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
        else if(ind == PROTOCOL::USER_GET_PASSWORD_HALF){
            QString half;
            in >> half;
            connect_to_client_database(half);
        }
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

    try{
        QSqlQuery query;
        query.prepare("INSERT INTO user_notes (message) "
                      "VALUES(:m_txt);");
        query.bindValue(":m_txt", text);
        query.exec();
    }
    catch(...){

    }

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
    m_tcpSocket->writeBlock(block);
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

        m_tcpSocket->writeBlock(block);
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

    m_tcpSocket->writeBlock(block);
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

    m_tcpSocket->writeBlock(block);
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


/*
* Notes widget
*/

int EchoClient::get_available_id(){
    QSqlQuery query;
    query.prepare("SELECT seq FROM sqlite_sequence");
    query.exec();
    query.next();
    return query.value(0).toInt() + 1;
}

void EchoClient::addNoteRequest(){
    int rowNum = m_notes->rowCount();
    if (m_inputNoteEdit->text().isEmpty())
        return;

    QSqlQuery query;
    int id_ = this->get_available_id();
    query.prepare("INSERT INTO user_notes (id, message) VALUES(:m_id, :m_txt);");
    query.bindValue(":m_txt", m_inputNoteEdit->text());
    query.bindValue(":m_id", id_);
    query.exec();

    QTableWidgetItem* items[2];
    items[0] = new QTableWidgetItem(QString::number(id_));
    items[1] = new QTableWidgetItem(m_inputNoteEdit->text());

    m_notes->insertRow(rowNum);
    for (int i = 0; i < 2; ++i){
        items[i]->setFlags(items[i]->flags() & (~Qt::ItemIsEditable));
        m_notes->setItem(rowNum, i, items[i]);
    }
}

void EchoClient::deleteNoteRequest(){
    int curRow = m_notes->currentRow();
    if (curRow == -1)
        return;

    int noteId = m_notes->item(curRow, 0)->data(QVariant::Int).toInt();

    QSqlQuery query;
    query.prepare("DELETE FROM user_notes WHERE id=:m_id;");
    query.bindValue(":m_id", noteId);
    query.exec();

    this->refreshNotesRequest();
}

void EchoClient::modifyNoteRequest(){
    int curRow = m_notes->currentRow();
    if (curRow == -1)
        return;

    int noteId = m_notes->item(curRow, 0)->data(QVariant::Int).toInt();
    const QString& text = m_notes->item(curRow, 1)->text();

    bool ok;
    QString newText = QInputDialog::getText(
        this, tr("Modify"), tr("New text:"), QLineEdit::Normal, text, &ok
    );
    if (ok && !newText.isEmpty() && newText != text)
    {
        QSqlQuery query;
        query.prepare("UPDATE user_notes SET message=:m_txt WHERE id=:m_id;");
        query.bindValue(":m_id", noteId);
        query.bindValue(":m_txt", newText);
        query.exec();
        this->refreshNotesRequest();
    }
}

void EchoClient::refreshNotesRequest(){
    QSqlQuery query;
    query.prepare("SELECT * FROM user_notes;");
    query.exec();

    int row_num = 0;
    m_notes->setRowCount(row_num);
    if(query.isSelect()){
        while(query.next()){
            QTableWidgetItem* items[2];
            items[0] = new QTableWidgetItem(query.value(0).toString());
            items[1] = new QTableWidgetItem(query.value(1).toString());

            m_notes->insertRow(row_num);
            for (int i = 0; i < 2; ++i){
                items[i]->setFlags(items[i]->flags() & (~Qt::ItemIsEditable));
                m_notes->setItem(row_num, i, items[i]);
            }
            row_num += 1;
        }
    }
}

void EchoClient::tabBarTabChanged(int index){
    if(index == 1){
        bool ok;
        QString users_half = QInputDialog::getText(
            this, tr("Password"), tr("Password:"), QLineEdit::Normal, "", &ok
        );
        database_password += users_half;

        if(ok){
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);

            out << PROTOCOL::USER_GET_PASSWORD_HALF << username;
            m_tcpSocket->writeBlock(block);
            m_tcpSocket->flush();
        }
    }
    else{
        database_password = "";
    }
}

void EchoClient::connect_to_client_database(QString &server_password_half){
    QString db_path = "/home/kolegor/Code/Kolepov/EchoClient/db.sqlite";
    // QString db_path = somewhere_on_windows

    db_connection_ = QSqlDatabase::addDatabase("QSQLITE");
    db_connection_.setDatabaseName(db_path);
    db_connection_.setPassword(kdf(database_password, server_password_half));
    db_connection_.open();

    if (!db_connection_.isOpen()) {
        qDebug() << '!!!' << database_password << "!!!\n";
        QMessageBox::critical(0, qApp->tr("Cannot open database."),
            qApp->tr("Invalid database password.\n"
                     "%1").arg(db_connection_.lastError().text()),
                              QMessageBox::Cancel);
        m_tabWidget->setCurrentIndex(0);
        return;
    }
    else{
        qDebug() << "???" << database_password << "???\n";
    }
}

QString EchoClient::kdf(QString &string_password, QString &salt_str){
    QByteArray array(string_password.toStdString().c_str());
    byte* password = reinterpret_cast<byte*>(array.data());
    size_t plen = strlen((const char*)password);

    QByteArray array_salt(salt_str.toStdString().c_str());
    byte* salt = reinterpret_cast<byte*>(array_salt.data());
    size_t slen = strlen((const char*)salt);

    size_t dlen = 128;
    byte derived[dlen];

    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA1> pbkdf2;
    pbkdf2.DeriveKey(derived, sizeof(derived), 0, password, plen, salt, slen, 1);
    return QString((const char*)derived);
}


#include "echoclient.h"

EchoClient::EchoClient(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(new QTabWidget)
    , m_messages(new QTableWidget)
    , m_inputMessageEdit(new QLineEdit)
    , m_sendButton(new QPushButton(tr("Send")))
    , m_modifyButton(new QPushButton(tr("Modify")))
    , m_deleteButton(new QPushButton(tr("Delete")))
    , m_userList(new QListWidget)
    , m_moderatorList(new QListWidget)
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
    m_messages->horizontalHeader()->setStretchLastSection(true);

    userLayout->addWidget(m_messages, 0, 0, 4, 4);
    userLayout->addWidget(m_modifyButton, 0, 4);
    userLayout->addWidget(m_deleteButton, 1, 4);
    userLayout->addWidget(m_inputMessageEdit, 4, 0, 1, 4);
    userLayout->addWidget(m_sendButton, 4, 4);

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
    QWidget::show();
}

EchoClient::~EchoClient()
{

}

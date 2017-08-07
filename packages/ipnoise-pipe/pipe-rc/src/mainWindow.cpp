#include "mainWindow.hpp"

MainWindow::MainWindow()
    :   QWidget()
{
    m_start_time            = 0;
    m_started               = 0;
    m_app                   = NULL;
    m_ipnoise               = NULL;
    m_init_timer            = NULL;
    m_api                   = new Api(this);
    m_config                = NULL;

    setupUi(this);

    connect(
        m_api,  SIGNAL(apiConnected()),
        this,   SLOT(apiConnectedSlot())
    );

    connect(
        m_api,  SIGNAL(apiDisconnected()),
        this,   SLOT(apiDisconnectedSlot())
    );

    connect(
        m_connect_widget,
        SIGNAL(doConnect()),
        this,
        SLOT(doConnectSlot())
    );

    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventInit *)),
        this,
        SLOT(apiEvent(ApiEventInit *))
    );

    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventLoginSuccess *)),
        this,
        SLOT(apiEvent(ApiEventLoginSuccess *))
    );

    connect(
        m_api,
        SIGNAL(apiEvent(ApiEventLoginFailed *)),
        this,
        SLOT(apiEvent(ApiEventLoginFailed *))
    );

    m_connect_widget->rotate();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi(QWidget *a_parent)
{
    m_vbox                  = new QVBoxLayout(a_parent);
    m_status_label          = new QLabel;
    m_connect_widget        = new ConnectWidget;
    m_contact_list_widget   = NULL;

    setLayout(m_vbox);
    showConnectWidget();
}

void MainWindow::doConnectSlot()
{
    ApiCommandArgs  args;
    QString         login;
    QString         password;
    bool            new_login;

    login       = m_connect_widget->getLogin();
    password    = m_connect_widget->getPassword();
    new_login   = m_connect_widget->isNewLogin();

    statusMsg("");

    if (!login.size()){
        statusMsg("Error: Empty login");
        goto out;
    }
    args["login"] = login;

    if (!password.size()){
        statusMsg("Error: Empty password");
        goto out;
    }
    args["password"] = password;

    if (new_login){
        args["register"] = "true";
    }

    statusMsg("Login..");
    m_api->processCommand("login", args);

out:
    return;
}

void MainWindow::initTimerSlot()
{
    m_connect_widget->rotate();

    // update start time
    m_start_time++;

    if (1 == m_start_time){
        // loading
        statusMsg("Loading..");
        load();
        // api connect
        m_api->tcp_connect();
        goto out;
    }

    if (m_start_time > MAX_START_TIME_SEC){
        statusMsg("Failed");
        // delete timer
        delete m_init_timer;
        m_init_timer = NULL;
        // little wait
        usleep(2*1e6); // 2 seconds
        // close app
        m_app->exit();
        goto out;
    }

    if (!m_started){
        // update status
        statusDot();
        // trying to connect
        m_api->tcp_connect();
    } else {
        // delete timer
        delete m_init_timer;
        m_init_timer = NULL;
        // show main window
    }

out:
    return;
}

void MainWindow::_statusMsg(const QString &a_msg)
{
    m_status_label->setText(a_msg);
}

void MainWindow::statusMsg(const QString &a_msg)
{
    m_last_status_msg       = a_msg;
    m_last_status_dot_msg   = a_msg;
    _statusMsg(a_msg);
}

void MainWindow::statusDot()
{
    m_last_status_dot_msg += ".";
    if ((m_last_status_dot_msg.size()
        - m_last_status_msg.size()) > MAX_STATUS_DOTS)
    {
        m_last_status_dot_msg = m_last_status_msg;
    }
    _statusMsg(m_last_status_dot_msg);
}

int MainWindow::load()
{
    int err = 0;

    QString     program = "./ipnoise";
    QStringList arguments;

    m_ipnoise = new QProcess(m_app);
    m_ipnoise->start(program, arguments);

    return err;
}

int MainWindow::init(
    QApplication    *a_app,
    Config          *a_config)
{
    int err = 0;

    // store app
    m_app       = a_app;
    m_config    = a_config;

    // setup config for api
    m_api->setConfig(m_config);

    m_init_timer = new QTimer(this);
    connect(
        m_init_timer,
        SIGNAL(timeout()),
        this,
        SLOT(initTimerSlot())
    );
    m_init_timer->start(1e3);

    return err;
}

void MainWindow::apiConnectedSlot()
{
    ApiCommandArgs args;
    m_started = 1;
    statusMsg("Connecting..");
    m_connect_widget->setDisabled(true);
    m_api->processCommand("init", args);
}

void MainWindow::apiDisconnectedSlot()
{
    statusMsg("Connecting..");
    usleep(1*1e6);
    m_api->tcp_connect();
}

void MainWindow::apiEvent(
    ApiEventInit *a_sess)
{
    PWARN("------------ ApiEventInit ------------\n");
    if (a_sess->isNew()){
        statusMsg("Ready for login");
        showConnectWidget();
        m_connect_widget->setDisabled(false);
    }
}

void MainWindow::removeAll()
{
    PWARN("void MainWindow::removeAll\n");
    if (m_connect_widget){
        m_connect_widget->hide();
        m_vbox->removeWidget(m_connect_widget);
    }
    if (m_contact_list_widget){
        m_contact_list_widget->hide();
        m_vbox->removeWidget(m_contact_list_widget);
    }
    if (m_status_label){
        m_status_label->hide();
        m_vbox->removeWidget(m_status_label);
    }
}

void MainWindow::showConnectWidget()
{
    removeAll();

    if (not m_connect_widget){
        m_connect_widget = new ConnectWidget;
    }
    m_connect_widget->show();
    m_vbox->addWidget(m_connect_widget);
    m_status_label->show();
    m_vbox->addWidget(m_status_label);
}

void MainWindow::showContactListWidget()
{
    removeAll();

    if (not m_contact_list_widget){
        m_contact_list_widget = new ContactListWidget(
            m_api,
            this
        );
    }
    m_contact_list_widget->show();
    m_vbox->addWidget(m_contact_list_widget);
    m_status_label->show();
    m_vbox->addWidget(m_status_label);
}

void MainWindow::apiEvent(
    ApiEventLoginSuccess *)
{
    statusMsg("Logged successful");
    showContactListWidget();
    statusMsg(VERSION);
}

void MainWindow::apiEvent(
    ApiEventLoginFailed *)
{
    statusMsg("Logged failed");
}

void MainWindow::closeEvent(
    QCloseEvent *)
{
    ApiCommandArgs args;
    m_api->processCommand("logout", args);
}


#include "wizard.hpp"
#include "install.hpp"

WizardInstallPageInstall::WizardInstallPageInstall(
    Config      *a_config,
    QWidget     *a_parent)
    :   QWizardPage(a_parent)
{
    m_config        = a_config;
    m_install_state = INSTALL_ST_READY;

    m_ipnoise_process   = NULL;
    m_timer             = NULL;
    m_tcp_socket        = NULL;

    setupUi(this);
}

WizardInstallPageInstall::~WizardInstallPageInstall()
{
    if (m_timer){
        delete m_timer;
        m_timer = NULL;
    }

    if (m_ipnoise_process){
        delete m_ipnoise_process;
        m_ipnoise_process = NULL;
    }

    if (m_tcp_socket){
        delete m_tcp_socket;
        m_tcp_socket = NULL;
    }
}

void WizardInstallPageInstall::setupUi(
    QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/wizard_install_page_install.png"
    );

    setTitle(tr("Select install dir"));
    setSubTitle(tr("Please select install dir"));

    setPixmap(
        QWizard::LogoPixmap,
        QPixmap(pixmap_str)
    );

    QString install_dir = (*m_config)["install_dir"];

    m_vbox          = new QVBoxLayout(a_parent);
    m_dir_hbox      = new QHBoxLayout;
    m_dir_combo     = createComboBox(install_dir);
    m_dir_browse    = new QPushButton(tr("Browse..."));
    connect(
        m_dir_browse,   SIGNAL(clicked()),
        this,           SLOT(browseSlot())
    );

    m_dir_combo->setDisabled(true);
    m_dir_browse->setDisabled(true);

    // install dir
    m_dir_hbox->addWidget(m_dir_combo);
    m_dir_hbox->addWidget(m_dir_browse);
    m_vbox->addLayout(m_dir_hbox);

    // add info box
    m_info_vbox = new QVBoxLayout();
    m_info_vbox->addItem(new QSpacerItem(
        20,
        40,
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    ));

    m_vbox->addLayout(m_info_vbox);

    // progress
    m_progress_label = new QLabel;
    m_progress       = new QProgressBar;

    m_progress_label->setText(tr("Installing files.."));

    m_vbox->addWidget(m_progress_label);
    m_vbox->addWidget(m_progress);

    setLayout(m_vbox);
}

bool WizardInstallPageInstall::validatePage()
{
    bool ret = false;

    if (    INSTALL_ST_FAILED  >= m_install_state
        ||  INSTALL_ST_SUCCESS == m_install_state)
    {
        ret = true;
    }

    PWARN("validatePage: '%d'\n", ret);

    return ret;
}

void WizardInstallPageInstall::initializePage()
{
    // setup timer
    m_timer = new QTimer;
    connect(
        m_timer,    SIGNAL(timeout()),
        this,       SLOT(timerSlot())
    );
    m_timer->start(0);
}

QString WizardInstallPageInstall::getInstallDir()
{
    return m_dir_combo->currentText();
}

QComboBox *WizardInstallPageInstall::createComboBox(
    const QString &a_text)
{
    QComboBox *combo_box = new QComboBox;
    combo_box->setEditable(true);
    combo_box->addItem(a_text);
    combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(Q_WS_SIMULATOR)
    combo_box->setMinimumContentsLength(3);
#endif

    return combo_box;
}

void WizardInstallPageInstall::install()
{
    int     res;
    Packer  packer;
    Path    stage1;
    int     sections_count = -1;
    Path    old_dir = Path::getCwd();
    Path    install_dir;

    if (INSTALL_ST_UNPACK == m_install_state){
        goto out;
    }
    m_install_state = INSTALL_ST_UNPACK;

    install_dir = (*m_config)["install_dir"].toStdString();
    stage1      = (*m_config)["stage1"].toStdString();

    packer.setDecompressPercentCb(
        WizardInstallPageInstall::decompressPercentCb,
        (void *)this
    );

    // setup packer's root dir
    packer.setDir(install_dir);

    // start parse
    res = packer.parse(stage1);
    if (res){
        PERROR("Cannot parse: '%s'\n",
            stage1.path().c_str()
        );
        goto fail;
    }

    // get sections count
    sections_count = packer.getSectionsCount();
    if (sections_count > 0){
        // -1 section, because 'boot' section will not extracted
        sections_count -= 1;
    }

    // setup progress bar
    m_progress->setMinimum(0);
    m_progress->setMaximum(100 * sections_count);

    // change directory to up
    chdir("..");

    // start unpack
    res = packer.saveAll(1);
    if (res){
        PERROR("Install failed\n");
        goto fail;
    }

    // go back
    old_dir.cd();

    // all ok
    m_install_state = INSTALL_ST_UNPACK_SUCCESS;

    // next page
    // wizard()->next();

out:
    return;

fail:
    m_install_state = INSTALL_ST_UNPACK_FAILED;
    goto out;
}

void WizardInstallPageInstall::decompressPercentCb(
    Packer      *a_packer,
    int         a_total_percent,
    const Path  &a_cur_file,
    int         a_cur_file_percent,
    void        *a_ctx)
{
    WizardInstallPageInstall *page = NULL;
    page = (WizardInstallPageInstall *)a_ctx;

    page->m_progress->setValue(a_total_percent);

    QCoreApplication::processEvents();
}

void WizardInstallPageInstall::connectedSlot()
{
    PWARN("ipnoise connected\n");
    m_install_state = INSTALL_ST_CHECKING_SUCCESS;
}

void WizardInstallPageInstall::errorSlot(
     QAbstractSocket::SocketError)  // socketError
{
    PWARN("ipnoise connect error\n");
    delete m_tcp_socket;
    m_tcp_socket = NULL;
}

void WizardInstallPageInstall::timerSlot()
{
    int stop = 0;

    {
        QAbstractButton *button = NULL;
        // disable all
        button = wizard()->button(QWizard::BackButton);
        if (button){ button->setEnabled(false); }
        button = wizard()->button(QWizard::NextButton);
        if (button){ button->setEnabled(false); }
    }

    switch (m_install_state){
        case INSTALL_ST_READY:
            install();
            break;

        case INSTALL_ST_UNPACK_SUCCESS:
            firstStart();
            break;

        case INSTALL_ST_CHECKING:
            {
                int cur_val     = m_progress->value();
                int new_val     = cur_val + 1;

                m_progress->setValue(new_val);

                if (IPNOISE_CHECK_TIMEOUT <= cur_val){
                    // timeout :(
                    m_progress_label->setText(
                        tr("Configuration failed")
                    );
                    m_install_state = INSTALL_ST_FAILED;
                    // stop timer
                    stop = 1;
                    break;
                }

                // setup tcp socket
                if (m_tcp_socket){
                    // already done
                    break;
                }
                m_tcp_socket = new QTcpSocket;
                connect(
                    m_tcp_socket,   SIGNAL(connected()),
                    this,           SLOT(connectedSlot())
                );
                connect(
                    m_tcp_socket, SIGNAL(
                        error(QAbstractSocket::SocketError)
                    ),
                    this, SLOT(
                        errorSlot(QAbstractSocket::SocketError)
                    )
                );
                m_tcp_socket->connectToHost(
                    IPNOISE_CONNECT_HOST,
                    IPNOISE_CONNECT_PORT
                );
            }
            break;

        case INSTALL_ST_CHECKING_SUCCESS:
            m_progress_label->setText(tr("Configuration success"));
            m_progress->setValue(IPNOISE_CHECK_TIMEOUT);
            m_install_state = INSTALL_ST_SUCCESS;
            // stop timer
            stop = 1;
            break;

        default:
            break;
    }

    if (    INSTALL_ST_FAILED  >= m_install_state
        ||  INSTALL_ST_SUCCESS == m_install_state)
    {
        QAbstractButton *button = NULL;
        button = wizard()->button(QWizard::NextButton);
        if (button){ button->setEnabled(true); }
        // stop timer
        stop = 1;
    }

    if (stop){
        if (m_timer){
            delete m_timer;
            m_timer = NULL;
        }
        if (m_tcp_socket){
            delete m_tcp_socket;
            m_tcp_socket = NULL;
        }
    }

    if (m_timer){
        m_timer->start(1000);
    }
}

void WizardInstallPageInstall::firstStart()
{
    Path cur_dir;

    m_ipnoise_process = new QProcess;

    if (INSTALL_ST_CHECKING == m_install_state){
        goto out;
    }

    m_install_state = INSTALL_ST_CHECKING;

    // setup progress bar
    m_progress_label->setText(tr("Configuration.."));
    m_progress->setValue(0);
    m_progress->setMinimum(0);
    m_progress->setMaximum(IPNOISE_CHECK_TIMEOUT);

    cur_dir = Path::getCwd();
    PWARN("cur_dir: '%s'\n", cur_dir.path().c_str());

#if defined(WIN32) || defined(WIN64)
    m_ipnoise_process->start("ipnoise.exe");
#else
    m_ipnoise_process->start("./ipnoise");
#endif

out:
    return;
}


#include "wizard.hpp"
#include "selectInstallDir.hpp"

WizardInstallPageSelectInstallDir::WizardInstallPageSelectInstallDir(
    Config      *a_config,
    QWidget     *a_parent)
    :   QWizardPage(a_parent)
{
    m_config = a_config;

    setupUi(this);
}

WizardInstallPageSelectInstallDir::~WizardInstallPageSelectInstallDir()
{
}

void WizardInstallPageSelectInstallDir::setupUi(
    QWidget *a_parent)
{
    QString pixmap_str;

    pixmap_str = absolutePathFromUnixPath(
        "resources/wizard_install_page_select_install_dir.png"
    );

    setTitle(tr("Select install dir"));
    setSubTitle(tr("Please select install dir"));

    setPixmap(
        QWizard::LogoPixmap,
        QPixmap(pixmap_str)
    );

    setButtonText(QWizard::NextButton, tr("Install"));

    QString install_dir = (*m_config)["install_dir"];

    m_vbox          = new QVBoxLayout(a_parent);
    m_dir_hbox      = new QHBoxLayout;
    m_dir_combo     = createComboBox(install_dir);
    m_dir_browse    = new QPushButton(tr("Browse..."));
    connect(
        m_dir_browse,   SIGNAL(clicked()),
        this,           SLOT(browseSlot())
    );

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

    setLayout(m_vbox);
}

void WizardInstallPageSelectInstallDir::installDirChangedSlot(
    const QString &a_install_dir)
{
    (*m_config)["install_dir"] = a_install_dir;
}

QString WizardInstallPageSelectInstallDir::getInstallDir()
{
    return m_dir_combo->currentText();
}

void WizardInstallPageSelectInstallDir::dirSelectedSlot(
    const QString &a_dir)
{
    int text_index = -1;

    text_index = m_dir_combo->findText(a_dir);
    if (0 > text_index){
        m_dir_combo->addItem(a_dir);
        text_index = m_dir_combo->findText(a_dir);
    }

    if (0 <= text_index){
        m_dir_combo->setCurrentIndex(text_index);
    }
}

void WizardInstallPageSelectInstallDir::browseSlot()
{
    m_file_dialog = new QFileDialog;
    m_file_dialog->setFileMode(QFileDialog::Directory);

    connect(
        m_file_dialog,  SIGNAL(fileSelected(const QString &)),
        this,           SLOT(dirSelectedSlot(const QString &))
    );

    m_file_dialog->show();
}

QComboBox *WizardInstallPageSelectInstallDir::createComboBox(
    const QString &a_text)
{
    QComboBox *combo_box = new QComboBox;
    combo_box->setEditable(true);
    combo_box->addItem(a_text);
    combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(Q_WS_SIMULATOR)
    combo_box->setMinimumContentsLength(3);
#endif

    connect(
        combo_box,  SIGNAL(currentIndexChanged(const QString &)),
        this,       SLOT(installDirChangedSlot(const QString &))
    );

    return combo_box;
}


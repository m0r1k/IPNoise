#include "mainWindow.hpp"

MainWindow::MainWindow()
    :   QWidget()
{
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi(QWidget *a_parent)
{
    m_vbox              = new QVBoxLayout(a_parent);
    m_wizard_install    = new WizardInstall(m_config);

    connect(
        m_wizard_install,   SIGNAL(close()),
        this,               SLOT(close())
    );

    m_vbox->addWidget(m_wizard_install);

    setLayout(m_vbox);
}

int MainWindow::init(
    QApplication    *,
    Config          *a_config)
{
    int err = 0;
    m_config = a_config;
    setupUi(this);
    return err;
}


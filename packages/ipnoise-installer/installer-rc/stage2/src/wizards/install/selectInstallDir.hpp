#ifndef WIDGET_INSTALL_PAGE_SELECT_INSTALL_DIR_HPP
#define WIDGET_INSTALL_PAGE_SELECT_INSTALL_DIR_HPP

#include <QWidget>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QFileDialog>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QProgressDialog>
#include <QTimer>
#include <QCoreApplication>
#include <QProcess>
#include <QByteArray>
#include <QTcpSocket>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include <ipnoise-packer/packer/src/packer.hpp>
#include "config.hpp"

class WizardInstallPageSelectInstallDir
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardInstallPageSelectInstallDir(
            Config      *a_config,
            QWidget     *a_parent = 0
        );
        virtual ~WizardInstallPageSelectInstallDir();

        void            setupUi(QWidget *a_parent);
        QComboBox *     createComboBox(const QString &);
        QString         getInstallDir();

    protected slots:
        void    browseSlot();
        void    dirSelectedSlot(const QString &);
        void    installDirChangedSlot(const QString &);

    private:
        Config          *m_config;
        QFileDialog     *m_file_dialog;
        QVBoxLayout     *m_vbox;
        QHBoxLayout     *m_dir_hbox;
        QComboBox       *m_dir_combo;
        QPushButton     *m_dir_browse;

        QVBoxLayout     *m_info_vbox;
};

#endif


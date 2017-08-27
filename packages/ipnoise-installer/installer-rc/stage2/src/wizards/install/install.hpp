#ifndef WIDGET_INSTALL_PAGE_INSTALL_HPP
#define WIDGET_INSTALL_PAGE_INSTALL_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QWizardPage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QByteArray>
#include <QtNetwork/QTcpSocket>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>
#include <ipnoise-packer/packer/src/packer.hpp>
#include "config.hpp"

#define IPNOISE_CHECK_TIMEOUT   300
#define IPNOISE_CONNECT_HOST    "127.0.0.1"
#define IPNOISE_CONNECT_PORT    2210

class WizardInstallPageInstall
    :   public QWizardPage
{
    Q_OBJECT

    public:
        WizardInstallPageInstall(
            Config      *a_config,
            QWidget     *a_parent = 0
        );
        virtual ~WizardInstallPageInstall();

        enum InstallState {
            INSTALL_ST_CHECKING_FAILED      = -3,
            INSTALL_ST_UNPACK_FAILED        = -2,
            INSTALL_ST_FAILED               = -1,
            INSTALL_ST_READY                = 0,
            INSTALL_ST_UNPACK,
            INSTALL_ST_UNPACK_SUCCESS,
            INSTALL_ST_CHECKING,
            INSTALL_ST_CHECKING_SUCCESS,
            INSTALL_ST_SUCCESS
        };

        virtual void    initializePage();
        virtual bool    validatePage();

        void            setupUi(QWidget *a_parent);
        QComboBox *     createComboBox(const QString &);
        QString         getInstallDir();
        void            install();

        void            firstStart();

        static void decompressPercentCb(
            Packer      *a_packer,
            int         a_total_percent,
            const Path  &a_cur_file,
            int         a_cur_file_percent,
            void        *a_ctx
        );

    protected slots:
        void    timerSlot();
        void    connectedSlot();
        void    errorSlot(QAbstractSocket::SocketError);

    private:
        Config          *m_config;
        QFileDialog     *m_file_dialog;
        QVBoxLayout     *m_vbox;
        QHBoxLayout     *m_dir_hbox;
        QComboBox       *m_dir_combo;
        QPushButton     *m_dir_browse;

        QVBoxLayout     *m_info_vbox;

        QProgressDialog *m_pd;
        QProgressBar    *m_progress;
        QLabel          *m_progress_label;
        QTimer          *m_timer;
        QProcess        *m_ipnoise_process;

        InstallState    m_install_state;
        QTcpSocket      *m_tcp_socket;
};

#endif


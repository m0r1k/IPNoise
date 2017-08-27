class MainWindow;

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>

#include "wizards/install.hpp"

#include "config.hpp"
#include "main.hpp"

class MainWindow
    :   public QWidget
{
    Q_OBJECT

    public:
        MainWindow();
        virtual ~MainWindow();

        void    setupUi(QWidget *a_parent);
        int     init(QApplication *, Config *);

    private:
        Config          *m_config;
        QVBoxLayout     *m_vbox;
        WizardInstall   *m_wizard_install;
};

#endif


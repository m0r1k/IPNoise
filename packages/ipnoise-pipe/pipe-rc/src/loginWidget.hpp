#ifndef LOGIN_WIDGET_HPP
#define LOGIN_WIDGET_HPP

#include <QObject>
#include <QWidget>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

class LoginWidget
    :   public QWidget
{
    Q_OBJECT

    public:
        LoginWidget(QWidget *parent = 0);
        virtual ~LoginWidget();

        void setupUi(QWidget *a_parent);
        void retranslateUi(QWidget *a_parent);

        QCheckBox       *m_login_new;
        QLineEdit       *m_login_edit;
        QLineEdit       *m_password_edit;
        QPushButton     *m_login_submit;

    public slots:
        void setDisabled(bool disable);

    private:
        QVBoxLayout     *m_verticalLayout_3;
        QVBoxLayout     *m_verticalLayout_2;
        QHBoxLayout     *m_horizontalLayout;
        QFormLayout     *m_formLayout_2;
        QLabel          *m_login_label;
        QLabel          *m_password_label;
        QVBoxLayout     *m_verticalLayout;
        QCheckBox       *m_password_store;
        QHBoxLayout     *m_horizontalLayout_2;
        QLabel          *m_status_label;
        QSpacerItem     *m_verticalSpacer;
};

#endif


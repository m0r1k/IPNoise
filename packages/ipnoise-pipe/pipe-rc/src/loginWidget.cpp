#include "loginWidget.hpp"

LoginWidget::LoginWidget(QWidget *parent)
    :   QWidget(parent)
{
    setupUi(this);
    m_status_label->setText("");
    setDisabled(true);
}

LoginWidget::~LoginWidget()
{
}

void LoginWidget::setupUi(QWidget *a_parent)
{
    a_parent->setEnabled(true);
    a_parent->resize(370, 106);
    m_verticalLayout_3  = new QVBoxLayout(a_parent);
    m_verticalLayout_2  = new QVBoxLayout();
    m_horizontalLayout  = new QHBoxLayout();
    m_formLayout_2      = new QFormLayout();
    m_login_label       = new QLabel(a_parent);

    m_formLayout_2->setWidget(
        0,
        QFormLayout::LabelRole,
        m_login_label
    );

    m_login_edit = new QLineEdit(a_parent);
    m_formLayout_2->setWidget(
        0,
        QFormLayout::FieldRole,
        m_login_edit
    );

    m_password_label = new QLabel(a_parent);
    m_formLayout_2->setWidget(
        1,
        QFormLayout::LabelRole,
        m_password_label
    );

    m_password_edit = new QLineEdit(a_parent);
    m_formLayout_2->setWidget(
        1,
        QFormLayout::FieldRole,
        m_password_edit
    );

    m_horizontalLayout->addLayout(m_formLayout_2);

    m_verticalLayout        = new QVBoxLayout();
    m_login_new             = new QCheckBox(a_parent);
    m_verticalLayout->addWidget(m_login_new);
    m_password_store        = new QCheckBox(a_parent);
    m_verticalLayout->addWidget(m_password_store);
    m_horizontalLayout->addLayout(m_verticalLayout);
    m_verticalLayout_2->addLayout(m_horizontalLayout);
    m_horizontalLayout_2    = new QHBoxLayout();
    m_login_submit          = new QPushButton(a_parent);
    m_horizontalLayout_2->addWidget(m_login_submit);
    m_status_label          = new QLabel(a_parent);

    QSizePolicy sizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Preferred
    );
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(
        m_status_label->sizePolicy().hasHeightForWidth()
    );
    m_status_label->setSizePolicy(sizePolicy);
    m_status_label->setAlignment(Qt::AlignCenter);

    m_horizontalLayout_2->addWidget(m_status_label);


    m_verticalLayout_2->addLayout(m_horizontalLayout_2);


    m_verticalLayout_3->addLayout(m_verticalLayout_2);

    m_verticalSpacer = new QSpacerItem(
        20, 40,
        QSizePolicy::Minimum,
        QSizePolicy::Expanding
    );

    m_verticalLayout_3->addItem(m_verticalSpacer);

    retranslateUi(a_parent);

    QMetaObject::connectSlotsByName(a_parent);
}

void LoginWidget::retranslateUi(QWidget *a_parent)
{
    a_parent->setWindowTitle(QApplication::translate(
        "LoginWidget", "Form", 0//, QApplication::UnicodeUTF8
    ));
    m_login_label->setText(QApplication::translate(
        "LoginWidget", "Login", 0//, QApplication::UnicodeUTF8
    ));
    m_password_label->setText(QApplication::translate(
        "LoginWidget", "Password", 0//, QApplication::UnicodeUTF8
    ));
    m_login_new->setText(QApplication::translate(
        "LoginWidget", "New user", 0//, QApplication::UnicodeUTF8
    ));
    m_password_store->setText(QApplication::translate(
        "LoginWidget", "Store", 0//, QApplication::UnicodeUTF8
    ));
    m_login_submit->setText(QApplication::translate(
        "LoginWidget", "Login", 0//, QApplication::UnicodeUTF8
    ));
    m_status_label->setText(QApplication::translate(
        "LoginWidget", "Status line", 0//, QApplication::UnicodeUTF8
    ));
}

void LoginWidget::setDisabled(bool disable)
{
    m_login_label->setDisabled(disable);
    m_login_edit->setDisabled(disable);
    m_login_new->setDisabled(disable);

    m_password_label->setDisabled(disable);
    m_password_edit->setDisabled(disable);
    m_password_store->setDisabled(disable);

    m_login_submit->setDisabled(disable);
}


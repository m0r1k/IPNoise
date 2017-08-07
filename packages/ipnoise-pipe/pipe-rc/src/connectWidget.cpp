#include "connectWidget.hpp"

ConnectWidget::ConnectWidget(QWidget *a_parent)
    :   QWidget(a_parent)
{
    m_status_icon_degrees = 0;

    setupUi(this);

    connect(
        m_login_widget->m_login_submit,
        SIGNAL(clicked(bool)),
        this,
        SLOT(loginButtonSlot(bool))
    );
}

ConnectWidget::~ConnectWidget()
{
}

void ConnectWidget::setupUi(QWidget *a_parent)
{
    m_vbox                  = new QVBoxLayout(a_parent);
    m_status_icon_hbox      = new QHBoxLayout;
    m_status_icon           = new QLabel;
    m_login_widget          = new LoginWidget;

    m_status_icon_hbox->addItem(new QSpacerItem(
        20,
        40,
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    ));
    m_status_icon_hbox->addWidget(m_status_icon);
    m_status_icon_hbox->addItem(new QSpacerItem(
        20,
        40,
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    ));

    m_vbox_spacer_1 = new QSpacerItem(
        20,
        40,
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
    m_vbox->addItem(m_vbox_spacer_1);
    m_vbox->addLayout(m_status_icon_hbox);
    m_vbox_spacer_2 = new QSpacerItem(
        20,
        40,
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
    m_vbox->addItem(m_vbox_spacer_2);
    m_vbox->addWidget(m_login_widget);

    setLayout(m_vbox);
}

void ConnectWidget::loginButtonSlot(bool)
{
    emit doConnect();
}

QString ConnectWidget::getLogin()
{
    return m_login_widget->m_login_edit->text();
}

QString ConnectWidget::getPassword()
{
    return m_login_widget->m_password_edit->text();
}

bool ConnectWidget::isNewLogin()
{
    return m_login_widget->m_login_new->isChecked();
}

void ConnectWidget::rotate()
{
    char buffer[1024];

    QString pixmap_str;

    m_status_icon_degrees += 45;
    if (360 <= m_status_icon_degrees){
        m_status_icon_degrees = 0;
    }
    snprintf(buffer, sizeof(buffer),
        "resources/default/images/loading_001_%d.png",
        m_status_icon_degrees
    );

    pixmap_str = absolutePathFromUnixPath(buffer);

    QImage image(pixmap_str);
    m_status_icon->setPixmap(QPixmap::fromImage(image));
}

void ConnectWidget::setDisabled(const bool &a_val)
{
    m_login_widget->setDisabled(a_val);
}


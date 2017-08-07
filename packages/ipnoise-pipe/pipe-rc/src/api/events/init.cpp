#include "api.hpp"
#include "init.hpp"

ApiEventInit::ApiEventInit(
    Api *a_api)
    :   ApiEvent(a_api, "init")
{
    m_is_new = false;
}

ApiEventInit::~ApiEventInit()
{
}

bool ApiEventInit::isNew()
{
    return m_is_new;
}

void ApiEventInit::process()
{
    QString cur_sessid  = getApi()->getDstSessId();
    QString sessid      = m_element.attribute("src_sessid");
    if (cur_sessid != sessid){
        m_is_new = true;
    }
    getApi()->setDstSessId(sessid);
}


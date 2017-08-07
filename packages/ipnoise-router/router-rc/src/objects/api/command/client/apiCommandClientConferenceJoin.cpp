/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "apiCommandClientConferenceJoin.hpp"

ApiCommandClientConferenceJoin::ApiCommandClientConferenceJoin()
    :   ApiCommandClient(API_COMMAND_CLIENT_CONFERENCE_JOIN)
{
}

ApiCommandClientConferenceJoin::~ApiCommandClientConferenceJoin()
{
}

int ApiCommandClientConferenceJoin::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = -1;
    UserObject          *user           = NULL;
    SessionObject       *session        = NULL;
    ConferenceObject    *conference     = NULL;
    string              sessid          = "";
    string              conf_id         = "";
    string              conf_topic      = "";
    string              flags_str       = "";
    string              conf_flags_str  = "";
    string              huid            = "";
    string              answer          = "";

    Splitter2           flags;
    Splitter2           conf_flags;

    vector <DomElement *>           items;
    vector <DomElement *>::iterator items_i;

    // search session
    session = nc->getSession(skb);

    if (!session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    // get session ID
    sessid = session->getSessId();

    // search user for this session
    user = session->getOwner();
    if (!user){
        err = nc->errorNotLogged(skb);
        goto out;
    }

    // read command attributes
    conf_id         = nc->getAttribute("conf_id");
    conf_topic      = nc->getAttribute("conf_topic");
    flags_str       = nc->getAttribute("flags");
    conf_flags_str  = nc->getAttribute("conf_flags");
    huid            = nc->getAttribute("huid");

    PDEBUG_OBJ(nc, 5, "ApiCommandClientConferenceJoin::process\n"
        " conference:       '%s'@'%s'\n"
        " conference name:  '%s'\n"
        " join flags:       '%s'\n"
        " conference flags: '%s'\n",
        conf_id.c_str(),
        huid.c_str(),
        conf_topic.c_str(),
        flags_str.c_str(),
        conf_flags_str.c_str()
    );

    // parse flags
    flags       = Splitter2(flags_str,      ", ");
    conf_flags  = Splitter2(conf_flags_str, ", ");

    if (not huid.empty() && huid != user->getHuid()){
        // join to remote conferences yet unsupported now
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=   "<events>";
        answer +=     "<event";
        answer +=       " type=\"conferenceJoinFailed\"";
        answer +=       " ver=\"0.01\"";
        answer +=       " conf_id=\"" + conf_id + "\"";
        answer +=     ">";
        answer +=       "<description";
        answer +=         " msg_id=\"conferenceJoinFailed.10\"";
        answer +=       ">";
        answer +=           "join to remote conferences"
                            " yet unsupported now";
        answer +=       "</description>";
        answer +=     "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
        goto out;
    }

    // process local conference

    if (not conf_id.empty()){
        conference = user->getConferenceById(conf_id);
    }

    if (flags.find("create")){
        if (conference){
            if (flags.find("excl")){
                answer  = "<ipnoise";
                answer +=   " ver=\"0.01\"";
                answer +=   ">";
                answer +=   "<events>";
                answer +=     "<event";
                answer +=       " type=\"conferenceJoinFailed\"";
                answer +=       " ver=\"0.01\"";
                answer +=       " conf_id=\""
                                    + conference->getId() +"\"";
                answer +=     ">";
                answer +=       "<description";
                answer +=         " msg_id=\"conferenceJoinFailed.2\"";
                answer +=       ">";
                answer +=           "conference create failed"
                                    " (conference exist)";
                answer +=       "</description>";
                answer +=     "</event>";
                answer +=   "</events>";
                answer += "</ipnoise";

                // send answer only to current client
                session->queue(answer);
                goto out;
            }
        } else {
            conference = user->createConferenceById(
                conf_id,
                conf_flags_str,
                conf_topic
            );
            if (!conference){
                PERROR_OBJ(this,
                    "Cannot create conference with id: '%s'"
                    " for user with huid: '%s'\n",
                    conf_id.c_str(),
                    user->getHuid().c_str()
                );
                answer  = "<ipnoise";
                answer +=   " ver=\"0.01\"";
                answer +=   ">";
                answer +=   "<events>";
                answer +=     "<event";
                answer +=       " type=\"conferenceJoinFailed\"";
                answer +=       " ver=\"0.01\"";
                answer +=       " conf_id=\""
                                    + conf_flags_str + "\"";
                answer +=     ">";
                answer +=       "<description";
                answer +=         " msg_id=\"conferenceJoinFailed.0\"";
                answer +=       ">";
                answer +=           "internal error";
                answer +=       "</description>";
                answer +=     "</event>";
                answer +=   "</events>";
                answer += "</ipnoise>";

                // send answer only to current client
                session->queue(answer);
                goto out;
            }
        }
    }

    if (!conference){
        answer  = "<ipnoise";
        answer +=   " ver=\"0.01\"";
        answer +=   ">";
        answer +=   "<events>";
        answer +=     "<event";
        answer +=       " type=\"conferenceJoinFailed\"";
        answer +=       " ver=\"0.01\"";
        answer +=       " conf_id=\"" + conference->getId() + "\"";
        answer +=     ">";
        answer +=       "<description";
        answer +=         " msg_id=\"conferenceJoinFailed.1\"";
        answer +=       ">";
        answer +=         "conference not found";
        answer +=       "</description>";
        answer +=     "</event>";
        answer +=   "</events>";
        answer += "</ipnoise>";

        // send answer only to current client
        session->queue(answer);
    }

out:
    return err;
}


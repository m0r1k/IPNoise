/*
 * Copyright 1997,1999,2000 Andi Kleen. Subject to the GPL. 
 * $Id: statistics.c,v 1.17 2002/04/28 15:41:01 ak Exp $
 * 19980630 - i18n - Arnaldo Carvalho de Melo <acme@conectiva.com.br> 
 * 19981113 - i18n fixes - Arnaldo Carvalho de Melo <acme@conectiva.com.br> 
 * 19990101 - added net/netstat, -t, -u, -w supprt - Bernd Eckenfels 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "intl.h"

/* #define WARN 1 */

#ifdef WARN
#define UFWARN(x) x
#else
#define UFWARN(x)
#endif

int print_static,f_raw,f_tcp,f_udp,f_unknown = 1;

enum State {
    number = 0, opt_number, i_forward, i_inp_icmp, i_outp_icmp, i_rto_alg,
    MaxState
};

#define normal number

struct entry {
    char *title;
    char *out;
    enum State type;
};

struct statedesc { 
    int indent;
    char *title; 
}; 

struct statedesc states[] = { 
    [number] = { 4, NULL },
    [opt_number] = { 4, NULL }, 
    [i_forward] = { 4, NULL },
    [i_inp_icmp] = { 8, N_("ICMP input histogram:") },
    [i_outp_icmp] = { 8, N_("ICMP output histogram:") },
    [MaxState] = {0},
}; 

static enum State state;

#define I_STATIC (1<<16)	/* static configuration option. */
#define I_TITLE  (1<<17)

/* 
 * XXX check against the snmp mib rfc.
 *
 * Don't mark the first field as translatable! It's a snmp MIB standard.
 * - acme
 */
struct entry Iptab[] =
{
    {"Forwarding", N_("Forwarding is %s"), i_forward | I_STATIC},
    {"DefaultTTL", N_("Default TTL is %lu"), number | I_STATIC},
    {"InReceives", N_("%lu total packets received"), number},
    {"InHdrErrors", N_("%lu with invalid headers"), opt_number},
    {"InAddrErrors", N_("%lu with invalid addresses"), opt_number},
    {"ForwDatagrams", N_("%lu forwarded"), number},
    {"InUnknownProtos", N_("%lu with unknown protocol"), opt_number},
    {"InDiscards", N_("%lu incoming packets discarded"), number},
    {"InDelivers", N_("%lu incoming packets delivered"), number},
    {"OutRequests", N_("%lu requests sent out"), number},	/*? */
    {"OutDiscards", N_("%lu outgoing packets dropped"), opt_number},
    {"OutNoRoutes", N_("%lu dropped because of missing route"), opt_number},
    {"ReasmTimeout", N_("%lu fragments dropped after timeout"), opt_number},
    {"ReasmReqds", N_("%lu reassemblies required"), opt_number},	/* ? */
    {"ReasmOKs", N_("%lu packets reassembled ok"), opt_number},
    {"ReasmFails", N_("%lu packet reassembles failed"), opt_number},
    {"FragOKs", N_("%lu fragments received ok"), opt_number},
    {"FragFails", N_("%lu fragments failed"), opt_number},
    {"FragCreates", N_("%lu fragments created"), opt_number}
};

struct entry Icmptab[] =
{
    {"InMsgs", N_("%lu ICMP messages received"), number},
    {"InErrors", N_("%lu input ICMP message failed."), number},
    {"InDestUnreachs", N_("destination unreachable: %lu"), i_inp_icmp | I_TITLE},
    {"InTimeExcds", N_("timeout in transit: %lu"), i_inp_icmp | I_TITLE},
    {"InParmProbs", N_("wrong parameters: %lu"), i_inp_icmp | I_TITLE},	/*? */
    {"InSrcQuenchs", N_("source quenches: %lu"), i_inp_icmp | I_TITLE},
    {"InRedirects", N_("redirects: %lu"), i_inp_icmp | I_TITLE},
    {"InEchos", N_("echo requests: %lu"), i_inp_icmp | I_TITLE},
    {"InEchoReps", N_("echo replies: %lu"), i_inp_icmp | I_TITLE},
    {"InTimestamps", N_("timestamp request: %lu"), i_inp_icmp | I_TITLE},
    {"InTimestampReps", N_("timestamp reply: %lu"), i_inp_icmp | I_TITLE},
    {"InAddrMasks", N_("address mask request: %lu"), i_inp_icmp | I_TITLE},	/*? */
    {"InAddrMaskReps", N_("address mask replies: %lu"), i_inp_icmp | I_TITLE},	/*? */
    {"OutMsgs", N_("%lu ICMP messages sent"), number},
    {"OutErrors", N_("%lu ICMP messages failed"), number},
    {"OutDestUnreachs", N_("destination unreachable: %lu"), i_outp_icmp | I_TITLE},
    {"OutTimeExcds", N_("time exceeded: %lu"), i_outp_icmp | I_TITLE},
    {"OutParmProbs", N_("wrong parameters: %lu"), i_outp_icmp | I_TITLE},	/*? */
    {"OutSrcQuenchs", N_("source quench: %lu"), i_outp_icmp | I_TITLE},
    {"OutRedirects", N_("redirect: %lu"), i_outp_icmp | I_TITLE},
    {"OutEchos", N_("echo request: %lu"), i_outp_icmp | I_TITLE},
    {"OutEchoReps", N_("echo replies: %lu"), i_outp_icmp | I_TITLE},
    {"OutTimestamps", N_("timestamp requests: %lu"), i_outp_icmp | I_TITLE},
    {"OutTimestampReps", N_("timestamp replies: %lu"), i_outp_icmp | I_TITLE},
    {"OutAddrMasks", N_("address mask requests: %lu"), i_outp_icmp | I_TITLE},
    {"OutAddrMaskReps", N_("address mask replies: %lu"), i_outp_icmp | I_TITLE},
};

struct entry Tcptab[] =
{
    {"RtoAlgorithm", N_("RTO algorithm is %s"), i_rto_alg | I_STATIC},
    {"RtoMin", "", number},
    {"RtoMax", "", number},
    {"MaxConn", "", number},
    {"ActiveOpens", N_("%lu active connections openings"), number},
    {"PassiveOpens", N_("%lu passive connection openings"), number},
    {"AttemptFails", N_("%lu failed connection attempts"), number},
    {"EstabResets", N_("%lu connection resets received"), number},
    {"CurrEstab", N_("%lu connections established"), number},
    {"InSegs", N_("%lu segments received"), number},
    {"OutSegs", N_("%lu segments send out"), number},
    {"RetransSegs", N_("%lu segments retransmited"), number},
    {"InErrs", N_("%lu bad segments received."), number},
    {"OutRsts", N_("%lu resets sent"), number},
};

struct entry Udptab[] =
{
    {"InDatagrams", N_("%lu packets received"), number},
    {"NoPorts", N_("%lu packets to unknown port received."), number},
    {"InErrors", N_("%lu packet receive errors"), number},
    {"OutDatagrams", N_("%lu packets sent"), number},
};

struct entry Tcpexttab[] =
{
    {"SyncookiesSent", N_("%lu SYN cookies sent"), opt_number},
    {"SyncookiesRecv", N_("%lu SYN cookies received"), opt_number},
    {"SyncookiesFailed", N_("%lu invalid SYN cookies received"), opt_number},

    { "EmbryonicRsts", N_("%lu resets received for embryonic SYN_RECV sockets"),
      opt_number },  
    { "PruneCalled", N_("%lu packets pruned from receive queue because of socket"
			" buffer overrun"), opt_number },  
    /* obsolete: 2.2.0 doesn't do that anymore */
    { "RcvPruned", N_("%lu packets pruned from receive queue"), opt_number },
    { "OfoPruned", N_("%lu packets dropped from out-of-order queue because of"
		      " socket buffer overrun"), opt_number }, 
    { "OutOfWindowIcmps", N_("%lu ICMP packets dropped because they were "
			     "out-of-window"), opt_number }, 
    { "LockDroppedIcmps", N_("%lu ICMP packets dropped because"
			     " socket was locked"), opt_number },
    { "TW", N_("%lu TCP sockets finished time wait in fast timer"), opt_number },
    { "TWRecycled", N_("%lu time wait sockets recycled by time stamp"), opt_number }, 
    { "TWKilled", N_("%lu TCP sockets finished time wait in slow timer"), opt_number },
    { "PAWSPassive", N_("%lu passive connections rejected because of"
			" time stamp"), opt_number },
    { "PAWSActive", N_("%lu active connections rejected because of "
		       "time stamp"), opt_number },
    { "PAWSEstab", N_("%lu packets rejects in established connections because of"
		      " timestamp"), opt_number },
    { "DelayedACKs", N_("%lu delayed acks sent"), opt_number },
    { "DelayedACKLocked", N_("%lu delayed acks further delayed because of"
			     " locked socket"), opt_number },
    { "DelayedACKLost", N_("Quick ack mode was activated %lu times"), opt_number },
    { "ListenOverflows", N_("%lu times the listen queue of a socket overflowed"),
      opt_number },
    { "ListenDrops", N_("%lu SYNs to LISTEN sockets ignored"), opt_number },
    { "TCPPrequeued", N_("%lu packets directly queued to recvmsg prequeue."), 
      opt_number },
    { "TCPDirectCopyFromBacklog", N_("%lu packets directly received"
				     " from backlog"), opt_number },
    { "TCPDirectCopyFromPrequeue", N_("%lu packets directly received"
				      " from prequeue"), opt_number },
    { "TCPPrequeueDropped", N_("%lu packets dropped from prequeue"), opt_number },
    { "TCPHPHits", N_("%lu packets header predicted"), number },
    { "TCPHPHitsToUser", N_("%lu packets header predicted and "
			    "directly queued to user"), opt_number },
    { "SockMallocOOM", N_("Ran %lu times out of system memory during " 
			  "packet sending"), opt_number }, 
     { "TCPPureAcks", N_("%u acknowledgments not containing data received"), opt_number },
     { "TCPHPAcks", N_("%u predicted acknowledgments"), opt_number },
     { "TCPRenoRecovery", N_("%u times recovered from packet loss due to fast retransmit"), opt_number },
     { "TCPSackRecovery", N_("%u times recovered from packet loss due to SACK data"), opt_number },
     { "TCPSACKReneging", N_("%u bad SACKs received"), opt_number },
     { "TCPFACKReorder", N_("Detected reordering %u times using FACK"), opt_number },
     { "TCPSACKReorder", N_("Detected reordering %u times using SACK"), opt_number },
     { "TCPTSReorder", N_("Detected reordering %u times using time stamp"), opt_number },
     { "TCPRenoReorder", N_("Detected reordering %u times using reno fast retransmit"), opt_number },
     { "TCPFullUndo", N_("%u congestion windows fully recovered"), opt_number }, 
     { "TCPPartialUndo", N_("%u congestion windows partially recovered using Hoe heuristic"), opt_number },
     { "TCPDSackUndo", N_("%u congestion window recovered using DSACK"), opt_number },
     { "TCPLossUndo", N_("%u congestion windows recovered after partial ack"), opt_number },
     { "TCPLostRetransmits", N_("%u retransmits lost"), opt_number },
     { "TCPRenoFailures",  N_("%u timeouts after reno fast retransmit"), opt_number },
     { "TCPSackFailures",  N_("%u timeouts after SACK recovery"), opt_number },
     { "TCPLossFailures",  N_("%u timeouts in loss state"), opt_number },
     { "TCPFastRetrans", N_("%u fast retransmits"), opt_number },
     { "TCPForwardRetrans", N_("%u forward retransmits"), opt_number }, 
     { "TCPSlowStartRetrans", N_("%u retransmits in slow start"), opt_number },
     { "TCPTimeouts", N_("%u other TCP timeouts"), opt_number },
     { "TCPRenoRecoveryFailed", N_("%u reno fast retransmits failed"), opt_number },
     { "TCPSackRecoveryFail", N_("%u sack retransmits failed"), opt_number },
     { "TCPSchedulerFailed", N_("%u times receiver scheduled too late for direct processing"), opt_number },
     { "TCPRcvCollapsed", N_("%u packets collapsed in receive queue due to low socket buffer"), opt_number },
     { "TCPDSACKOldSent", N_("%u DSACKs sent for old packets"), opt_number },
     { "TCPDSACKOfoSent", N_("%u DSACKs sent for out of order packets"), opt_number },
     { "TCPDSACKRecv", N_("%u DSACKs received"), opt_number },
     { "TCPDSACKOfoRecv", N_("%u DSACKs for out of order packets received"), opt_number },
     { "TCPAbortOnSyn", N_("%u connections reset due to unexpected SYN"), opt_number },
     { "TCPAbortOnData", N_("%u connections reset due to unexpected data"), opt_number },
     { "TCPAbortOnClose", N_("%u connections reset due to early user close"), opt_number },
     { "TCPAbortOnMemory", N_("%u connections aborted due to memory pressure"), opt_number },
     { "TCPAbortOnTimeout", N_("%u connections aborted due to timeout"), opt_number },
     { "TCPAbortOnLinger", N_("%u connections aborted after user close in linger timeout"), opt_number },
     { "TCPAbortFailed", N_("%u times unabled to send RST due to no memory"), opt_number }, 
     { "TCPMemoryPressures", N_("TCP ran low on memory %u times"), opt_number }, 
     { "TCPLoss", N_("%u TCP data loss events") },
};

struct tabtab {
    char *title;
    struct entry *tab;
    size_t size;
    int *flag; 
};

struct tabtab snmptabs[] =
{
    {"Ip", Iptab, sizeof(Iptab), &f_raw},
    {"Icmp", Icmptab, sizeof(Icmptab), &f_raw},
    {"Tcp", Tcptab, sizeof(Tcptab), &f_tcp},
    {"Udp", Udptab, sizeof(Udptab), &f_udp},
    {"TcpExt", Tcpexttab, sizeof(Tcpexttab), &f_tcp},
    {NULL}
};

/* XXX IGMP */

int cmpentries(const void *a, const void *b)
{
    return strcmp(((struct entry *) a)->title, ((struct entry *) b)->title);
}

void printval(struct tabtab *tab, char *title, int val)
{
    struct entry *ent = NULL, key;
    int type;
    char buf[512];

    key.title = title;
	if (tab->tab) 
	    ent = bsearch(&key, tab->tab, tab->size / sizeof(struct entry),
			  sizeof(struct entry), cmpentries);
    if (!ent) {			/* try our best */
	if (val) 
		printf("%*s%s: %d\n", states[state].indent, "", title, val);
	return;
    }
    type = ent->type;
    if (type & I_STATIC) {
	type &= ~I_STATIC;
	if (!print_static)
	    return;
    }
    if (*ent->out == '\0')
	return;

    if (type & I_TITLE) {
	type &= ~I_TITLE;
	if (state != type)
	    printf("%*s%s\n", states[state].indent, "", _(states[type].title));
    }
    buf[0] = '\0';
    switch (type) {
    case opt_number:
	if (val == 0) 
	    break;
	/*FALL THOUGH*/
    case number:
	snprintf(buf, sizeof(buf), _(ent->out), val);
	break;
    case i_forward:
	type = normal;
	snprintf(buf, sizeof(buf), _(ent->out), val == 2 ? _("enabled") : _("disabled"));
	break;
    case i_outp_icmp:
    case i_inp_icmp:
	if (val > 0)
	    snprintf(buf, sizeof(buf), _(ent->out), val);
	break;
    case i_rto_alg:		/* XXXX */
	break;
    default:
	abort();
    }
    if (buf[0])
	printf("%*s%s\n", states[type].indent, "", buf);

    state = type;
}

struct tabtab *newtable(struct tabtab *tabs, char *title)
{
    struct tabtab *t;
	static struct tabtab dummytab;
	
    for (t = tabs; t->title; t++) {
		if (!strcmp(title, t->title)) {
	    	if (*(t->flag))
				printf("%s:\n", _(title));
		    state = normal;
	   		return t;
		}
	}
	if (!f_unknown) 
		return NULL; 
	printf("%s:\n", _(title));
	dummytab.title = title;
	dummytab.flag = &f_unknown; 
	return &dummytab;
}

void process_fd(FILE *f,int file_desc)   // added file_desc to show propriate error mesg
{
    char buf1[1024], buf2[1024];
    char *sp, *np, *p;
    while (fgets(buf1, sizeof buf1, f)) {
	int endflag;
	struct tabtab *tab;

	if (strcmp(buf1,"\n") == 0)  // cut leading break
	    if (!fgets(buf1, sizeof buf1, f))
	    break;
	if (!fgets(buf2, sizeof buf2, f))
	    break;

	sp = strchr(buf1, ':');
	np = strchr(buf2, ':'); 
	if (!np || !sp) 
	    goto formaterr; 
	*sp = '\0';

	tab = newtable(snmptabs, buf1);
	if (tab == NULL) {
		printf("unknown %s\n", buf1);
		continue;
	}
	np++;
	sp++;

	endflag = 0;
	while (!endflag) {
	    sp += strspn(sp, " \t\n"); 
	    np += strspn(np, " \t\n"); 
	    /*if (*np == '\0') goto formaterr; */

	    p = sp+strcspn(sp, " \t\n");
	    if (*p == '\0')
		endflag = 1;
	    *p = '\0';

	    if (*sp != '\0' && *(tab->flag)) 	
		printval(tab, sp, strtoul(np, &np, 10));

	    sp = p + 1;
	}
    }
  return;
  
formaterr:
 switch(file_desc) {
    case 0: perror(_("error parsing /proc/net/snmp"));
	break;
    case 1: perror(_("error parsing /proc/net/netstat"));
	break;
  }
  return;
}


int parsesnmp(int flag_raw, int flag_tcp, int flag_udp)
{
    FILE *f;

    f_raw = flag_raw; f_tcp = flag_tcp; f_udp = flag_udp;

    f = fopen("/proc/net/snmp", "r");
    if (!f) {
	perror(_("cannot open /proc/net/snmp"));
	return(1);
    }
    process_fd(f,0);

    if (ferror(f)) {
	perror("/proc/net/snmp");
	return(1);
    }

    fclose(f);

    f = fopen("/proc/net/netstat", "r");

    if (f) {
    	process_fd(f,1);

        if (ferror(f)) {
	  perror("/proc/net/netstat");
	  return(1);
	}
    
        fclose(f);
    }
    return(0);
}
    

void inittab(void)
{
    struct tabtab *t;

    /* we sort at runtime because I'm lazy ;) */
    for (t = snmptabs; t->title; t++)
	qsort(t->tab, t->size / sizeof(struct entry),
	      sizeof(struct entry), cmpentries);
}
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
#define DBG_MIN_LEVEL 0
#define LWIP_COMPAT_SOCKETS 1
#define TAPIF_DEBUG DBG_OFF //DBG_ON
#define TUNIF_DEBUG DBG_OFF
#define UNIXIF_DEBUG DBG_OFF
#define DELIF_DEBUG DBG_OFF
#define SIO_FIFO_DEBUG DBG_OFF
#define TCPDUMP_DEBUG DBG_OFF //DBG_ON

#define LWIP_PROVIDE_ERRNO 1


#define PPP_DEBUG               DBG_OFF

#ifdef LWIP_DEBUG
/*#define MEM_DEBUG               DBG_ON
#define MEMP_DEBUG              DBG_ON
#define PBUF_DEBUG              DBG_ON
#define API_LIB_DEBUG   DBG_ON
#define API_MSG_DEBUG   DBG_ON */
   #define TCPIP_DEBUG             DBG_ON
   #define NETIF_DEBUG             DBG_ON
   #define SOCKETS_DEBUG   DBG_ON
   #define DEMO_DEBUG              DBG_ON
   #define IP_DEBUG                DBG_ON
   #define IP_REASS_DEBUG  DBG_ON
   #define ICMP_DEBUG              DBG_ON
   #define UDP_DEBUG               DBG_ON
//#define TCP_DEBUG               DBG_ON
   #define TCP_INPUT_DEBUG         DBG_ON
   #define TCP_OUTPUT_DEBUG        DBG_ON
/*#define TCP_RTO_DEBUG   DBG_ON
#define TCP_CWND_DEBUG  DBG_ON
#define TCP_WND_DEBUG   DBG_ON
#define TCP_FR_DEBUG    DBG_ON
#define TCP_QLEN_DEBUG  DBG_ON
#define TCP_RST_DEBUG   DBG_ON*/
#endif

#define DBG_TYPES_ON    (DBG_ON|DBG_TRACE|DBG_STATE|DBG_FRESH|DBG_HALT)

#define NO_SYS 0 //1 //phlb

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           1

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE               10240

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           16
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        4
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        5
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 8
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        16
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    3

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         2
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        10
/* MEMP_NUM_APIMSG: the number of struct api_msg, used for
   communication between the TCP/IP stack and the sequential
   programs. */
#define MEMP_NUM_API_MSG        16
/* MEMP_NUM_TCPIPMSG: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG      16


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          100

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       128

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. */
#define PBUF_LINK_HLEN          16

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT           1

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 1024

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             2048

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN       4 * TCP_SND_BUF/TCP_MSS

/* TCP writable space (bytes). This must be less than or equal
   to TCP_SND_BUF. It is the amount of space which must be
   available in the tcp snd_buf for select to return writable */
#define TCP_SNDLOWAT            (TCP_SND_BUF/2)

/* TCP receive window. */
#define TCP_WND                 8096

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           4

/* ---------- ARP options ---------- */
#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              1


/* IP reassembly and segmentation.These are orthogonal even
 * if they both deal with IP fragments */
#define IP_REASSEMBLY     1
#define IP_FRAG           1

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               0

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK     1

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255


/* ---------- Statistics options ---------- */

#ifdef LWIP_STATS
   #define LINK_STATS
   #define IP_STATS
   #define ICMP_STATS
   #define UDP_STATS
   #define TCP_STATS
   #define MEM_STATS
   #define MEMP_STATS
   #define PBUF_STATS
   #define SYS_STATS
#endif /* LWIP_STATS */

/* ---------- PPP options ---------- */

#define PPP_SUPPORT      1      /* Set > 0 for PPP */

#if PPP_SUPPORT > 0

   #define NUM_PPP 1        /* Max PPP sessions. */


/* Select modules to enable.  Ideally these would be set in the makefile but
 * we're limited by the command line length so you need to modify the settings
 * in this file.
 */
   #define PAP_SUPPORT      1   /* Set > 0 for PAP. */
   #define CHAP_SUPPORT     1   /* Set > 0 for CHAP. */
   #define MSCHAP_SUPPORT   0   /* Set > 0 for MSCHAP (NOT FUNCTIONAL!) */
   #define CBCP_SUPPORT     0   /* Set > 0 for CBCP (NOT FUNCTIONAL!) */
   #define CCP_SUPPORT      0   /* Set > 0 for CCP (NOT FUNCTIONAL!) */
   #define VJ_SUPPORT       1   /* Set > 0 for VJ header compression. */
   #define MD5_SUPPORT      1   /* Set > 0 for MD5 (see also CHAP) */


/*
 * Timeouts.
 */
   #define FSM_DEFTIMEOUT          6    /* Timeout time in seconds */
   #define FSM_DEFMAXTERMREQS      2    /* Maximum Terminate-Request transmissions */
   #define FSM_DEFMAXCONFREQS      10   /* Maximum Configure-Request transmissions */
   #define FSM_DEFMAXNAKLOOPS      5    /* Maximum number of nak loops */

   #define UPAP_DEFTIMEOUT         6    /* Timeout (seconds) for retransmitting req */
   #define UPAP_DEFREQTIME         30   /* Time to wait for auth-req from peer */

   #define CHAP_DEFTIMEOUT         6    /* Timeout time in seconds */
   #define CHAP_DEFTRANSMITS       10   /* max # times to send challenge */


/* Interval in seconds between keepalive echo requests, 0 to disable. */
   #if 1
      #define LCP_ECHOINTERVAL 0
   #else
      #define LCP_ECHOINTERVAL 10
   #endif

/* Number of unanswered echo requests before failure. */
   #define LCP_MAXECHOFAILS 3

/* Max Xmit idle time (in jiffies) before resend flag char. */
   #define PPP_MAXIDLEFLAG 100

/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *    limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *    of living in lcp.h)
 */
   #define PPP_MTU     1500  /* Default MTU (size of Info field) */
   #if 0
      #define PPP_MAXMTU  65535 - (PPP_HDRLEN + PPP_FCSLEN)
   #else
      #define PPP_MAXMTU  1500 /* Largest MTU we allow */
   #endif
   #define PPP_MINMTU  64
   #define PPP_MRU     1500  /* default MRU = max length of info field */
   #define PPP_MAXMRU  1500  /* Largest MRU we allow */
   #define PPP_DEFMRU      296          /* Try for this */
   #define PPP_MINMRU      128          /* No MRUs below this */


   #define MAXNAMELEN      256  /* max length of hostname or name for auth */
   #define MAXSECRETLEN    256  /* max length of password or secret */

#endif /* PPP_SUPPORT > 0 */

#endif /* __LWIPOPTS_H__ */

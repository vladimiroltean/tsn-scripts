/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright 2019 NXP Semiconductors */
/* This file contains code snippets from:
 * - libnfnetlink.h
 * - The Linux kernel
 * - The linuxptp project
 */
#ifndef _COMMON_H
#define _COMMON_H

#include <sys/queue.h>

#define PTP_VERSION 2

/* Values for the messageType field */
#define SYNC			0x0
#define DELAY_REQ		0x1
#define PDELAY_REQ		0x2
#define PDELAY_RESP		0x3
#define CUSTOM			0x4
#define FOLLOW_UP		0x8
#define DELAY_RESP		0x9
#define PDELAY_RESP_FOLLOW_UP	0xA
#define ANNOUNCE		0xB
#define SIGNALING		0xC
#define MANAGEMENT		0xD

#define PACKED __attribute__((packed))

struct ClockIdentity {
	__u8			id[8];
};

struct PortIdentity {
	struct ClockIdentity	clockIdentity;
	__u16			portNumber;
} PACKED;

struct ptp_header {
	__u8			tsmt; /* transportSpecific | messageType */
	__u8			ver;  /* reserved          | versionPTP  */
	__u16			messageLength;
	__u8			domainNumber;
	__u8			reserved1;
	__u8			flagField[2];
	__s64			correction;
	__u32			reserved2;
	struct PortIdentity	sourcePortIdentity;
	__u16			sequenceId;
	__u8			control;
	__s8			logMessageInterval;
} PACKED;

#define NSEC_PER_SEC	1000000000LL
#define ETH_P_TSN	0x22F0		/* TSN (IEEE 1722) packet	*/

#define TIMESPEC_BUFSIZ	32
#define MACADDR_BUFSIZ	32

#define TXTSTAMP_TIMEOUT_MS	10

/* From include/uapi/linux/net_tstamp.h */
#ifndef SOF_TIMESTAMPING_OPT_TX_SWHW
#define SOF_TIMESTAMPING_OPT_TX_SWHW	(1<<14)
#endif

typedef _Bool		bool;
enum {
	false	= 0,
	true	= 1
};

#define ARRAY_SIZE(array) \
	(sizeof(array) / sizeof(*array))

#ifndef LIST_FOREACH_SAFE
#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = LIST_FIRST((head));				\
	    (var) && ((tvar) = LIST_NEXT((var), field), 1);		\
	    (var) = (tvar))
#endif

/* Copied from libnfnetlink.h */

/* Pablo: What is the equivalence of be64_to_cpu in userspace?
 *
 * Harald: Good question.  I don't think there's a standard way [yet?],
 * so I'd suggest manually implementing it by "#if little endian" bitshift
 * operations in C (at least for now).
 *
 * All the payload of any nfattr will always be in network byte order.
 * This would allow easy transport over a real network in the future
 * (e.g. jamal's netlink2).
 *
 * Pablo: I've called it __be64_to_cpu instead of be64_to_cpu, since maybe
 * there will one in the userspace headers someday. We don't want to
 * pollute POSIX space naming,
 */
#include <byteswap.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#  ifndef __be64_to_cpu
#  define __be64_to_cpu(x)	(x)
#  endif
#  ifndef __cpu_to_be64
#  define __cpu_to_be64(x)	(x)
#  endif
# else
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  ifndef __be64_to_cpu
#  define __be64_to_cpu(x)	__bswap_64(x)
#  endif
#  ifndef __cpu_to_be64
#  define __cpu_to_be64(x)	__bswap_64(x)
#  endif
# endif
#endif

/**
 *	struct vlan_ethhdr - vlan ethernet header (ethhdr + vlan_hdr)
 *	@h_dest: destination ethernet address
 *	@h_source: source ethernet address
 *	@h_vlan_proto: ethernet protocol
 *	@h_vlan_TCI: priority and VLAN ID
 *	@h_vlan_encapsulated_proto: packet type ID or len
 */
struct vlan_ethhdr {
	unsigned char	h_dest[ETH_ALEN];
	unsigned char	h_source[ETH_ALEN];
	__be16		h_vlan_proto;
	__be16		h_vlan_TCI;
	__be16		h_vlan_encapsulated_proto;
};

struct isochron_log {
	int buf_len;
	char *buf;
};

int isochron_log_init(struct isochron_log *log);
void isochron_log_data(struct isochron_log *log, void *data, int len);
int isochron_log_xmit(struct isochron_log *log, int fd);
int isochron_log_recv(struct isochron_log *log, int fd);
void isochron_log_teardown(struct isochron_log *log);
void isochron_rcv_log_print(struct isochron_log *log);
void isochron_send_log_print(struct isochron_log *log);
void isochron_log_remove(struct isochron_log *log, void *p, int len);

#define ISOCHRON_STATS_PORT	5000

#define VLAN_PRIO_MASK		0xe000 /* Priority Code Point */
#define VLAN_PRIO_SHIFT		13
#define VLAN_CFI_MASK		0x1000 /* Canonical Format Indicator / Drop Eligible Indicator */
#define VLAN_VID_MASK		0x0fff /* VLAN Identifier */
#define VLAN_N_VID		4096

enum prog_arg_type {
	PROG_ARG_MAC_ADDR,
	PROG_ARG_LONG,
	PROG_ARG_TIME,
	PROG_ARG_STRING,
	PROG_ARG_BOOL,
};

struct prog_arg_string {
	char *buf;
	int size;
};

struct prog_arg_time {
	clockid_t clkid;
	__s64 *ns;
};

struct prog_arg_long {
	long *ptr;
};

struct prog_arg_mac_addr {
	char *buf;
};

struct prog_arg_boolean {
	bool *ptr;
};

struct prog_arg {
	const char *short_opt;
	const char *long_opt;
	bool optional;
	enum prog_arg_type type;
	union {
		struct prog_arg_string string;
		struct prog_arg_time time;
		struct prog_arg_long long_ptr;
		struct prog_arg_mac_addr mac;
		struct prog_arg_boolean boolean_ptr;
	};
};

int prog_parse_np_args(int argc, char **argv,
		       struct prog_arg *prog_args,
		       int prog_args_size);
void prog_usage(char *prog_name, struct prog_arg *prog_args,
		int prog_args_size);

struct timestamp {
	struct timespec		hw;
	struct timespec		sw;
};

struct isochron_send_pkt_data {
	__s64 tx_time;
	__s64 hwts;
	__s64 swts;
	short seqid;
};

struct isochron_rcv_pkt_data {
	char smac[ETH_ALEN];
	char dmac[ETH_ALEN];
	__s64 tx_time;
	__s64 hwts;
	__s64 swts;
	__u16 etype;
	__u16 seqid;
};

struct isochron_stat_entry {
	LIST_ENTRY(isochron_stat_entry) list;
	__s64 hw_tx_deadline_delta;
	__s64 sw_tx_deadline_delta;
	__s64 hw_rx_deadline_delta;
	__s64 sw_rx_deadline_delta;
	__s64 path_delay;
};

struct isochron_stats {
	LIST_HEAD(stats_head, isochron_stat_entry) entries;
	int frame_count;
	int hw_tx_deadline_misses;
	int sw_tx_deadline_misses;
	double tx_sync_offset_mean;
	double rx_sync_offset_mean;
};

int mac_addr_from_string(__u8 *to, char *from);
int sk_timestamping_init(int fd, const char *if_name, int on);
int sk_receive(int fd, void *buf, int buflen, struct timestamp *tstamp,
	       int flags, int timeout);
__s64 timespec_to_ns(const struct timespec *ts);
struct timespec ns_to_timespec(__s64 ns);
void mac_addr_sprintf(char *buf, __u8 *addr);
void ns_sprintf(char *buf, __s64 ns);
__s64 utc_to_tai(__s64 utc);

/**
 * ether_addr_to_u64 - Convert an Ethernet address into a u64 value.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return a u64 value of the address
 */
static inline __u64 ether_addr_to_u64(const unsigned char *addr)
{
	__u64 u = 0;
	int i;

	for (i = 0; i < ETH_ALEN; i++)
		u = u << 8 | addr[i];

	return u;
}

#endif
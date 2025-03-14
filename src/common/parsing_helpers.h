/* Copyright (C) 2021 Intel Corporation */
/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * This file contains parsing functions that are used in the packetXX XDP
 * programs. The functions are marked as __always_inline, and fully defined in
 * this header file to be included in the BPF program.
 *
 * Each helper parses a packet header, including doing bounds checking, and
 * returns the type of its contents if successful, and -1 otherwise.
 *
 * For Ethernet and IP headers, the content type is the type of the payload
 * (h_proto for Ethernet, nexthdr for IPv6), for ICMP it is the ICMP type field.
 * All return values are in host byte order.
 *
 * The versions of the functions included here are slightly expanded versions of
 * the functions in the packet01 lesson. For instance, the Ethernet header
 * parsing has support for parsing VLAN tags.
 */

#ifndef __PARSING_HELPERS_H
#define __PARSING_HELPERS_H

#include <stddef.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/udp.h>
#include <linux/tcp.h>

#include "intmd_headers.h"
#include "intmx_headers.h"

/* Header cursor to keep track of current parsing position */
struct hdr_cursor {
    void *pos;
};

/*
 *	struct vlan_hdr - vlan header
 *	@h_vlan_TCI: priority and VLAN ID
 *	@h_vlan_encapsulated_proto: packet type ID or len
 */
struct vlan_hdr {
    __be16 h_vlan_TCI;
    __be16 h_vlan_encapsulated_proto;
};

/*
 * Struct icmphdr_common represents the common part of the icmphdr and icmp6hdr
 * structures.
 */
struct icmphdr_common {
    __u8 type;
    __u8 code;
    __sum16 cksum;
};

/* Allow users of header file to redefine VLAN max depth */
#ifndef VLAN_MAX_DEPTH
#define VLAN_MAX_DEPTH 4
#endif

static __always_inline int proto_is_vlan(__u16 h_proto) {
    return !!(h_proto == bpf_htons(ETH_P_8021Q) ||
              h_proto == bpf_htons(ETH_P_8021AD));
}

/* Notice, parse_ethhdr() will skip VLAN tags, by advancing nh->pos and returns
 * next header EtherType, BUT the ethhdr pointer supplied still points to the
 * Ethernet header. Thus, caller can look at eth->h_proto to see if this was a
 * VLAN tagged packet.
 */
static __always_inline int parse_ethhdr(struct hdr_cursor *nh, void *data_end,
                                        struct ethhdr **ethhdr) {
    struct ethhdr *eth = nh->pos;
    int hdrsize = sizeof(*eth);
    struct vlan_hdr *vlh;
    __u16 h_proto;
    int i;

    /* Byte-count bounds check; check if current pointer + size of header
     * is after data_end.
     */
    if (nh->pos + hdrsize > data_end)
        return -1;

    nh->pos += hdrsize;
    *ethhdr = eth;
    vlh = nh->pos;
    h_proto = eth->h_proto;

/* Use loop unrolling to avoid the verifier restriction on loops;
 * support up to VLAN_MAX_DEPTH layers of VLAN encapsulation.
 */
#pragma unroll
    for (i = 0; i < VLAN_MAX_DEPTH; i++) {
        if (!proto_is_vlan(h_proto))
            break;

        if (vlh + 1 > data_end)
            break;

        h_proto = vlh->h_vlan_encapsulated_proto;
        vlh++;
    }

    nh->pos = vlh;
    return h_proto; /* network-byte-order */
}

static __always_inline int parse_ip6hdr(struct hdr_cursor *nh, void *data_end,
                                        struct ipv6hdr **ip6hdr) {
    struct ipv6hdr *ip6h = nh->pos;

    /* Pointer-arithmetic bounds check; pointer +1 points to after end of
     * thing being pointed to. We will be using this style in the remainder
     * of the tutorial.
     */
    if (ip6h + 1 > data_end)
        return -1;

    nh->pos = ip6h + 1;
    *ip6hdr = ip6h;

    return ip6h->nexthdr;
}

static __always_inline int parse_iphdr(struct hdr_cursor *nh, void *data_end,
                                       struct iphdr **iphdr) {
    struct iphdr *iph = nh->pos;
    int hdrsize;

    if (iph + 1 > data_end)
        return -1;

    hdrsize = iph->ihl * 4;

    /* Variable-length IPv4 header, need to use byte-based arithmetic */
    if (nh->pos + hdrsize > data_end)
        return -1;

    nh->pos += hdrsize;
    *iphdr = iph;

    return iph->protocol;
}

static __always_inline int parse_icmp6hdr(struct hdr_cursor *nh, void *data_end,
                                          struct icmp6hdr **icmp6hdr) {
    struct icmp6hdr *icmp6h = nh->pos;

    if (icmp6h + 1 > data_end)
        return -1;

    nh->pos = icmp6h + 1;
    *icmp6hdr = icmp6h;

    return icmp6h->icmp6_type;
}

static __always_inline int parse_icmphdr(struct hdr_cursor *nh, void *data_end,
                                         struct icmphdr **icmphdr) {
    struct icmphdr *icmph = nh->pos;

    if (icmph + 1 > data_end)
        return -1;

    nh->pos = icmph + 1;
    *icmphdr = icmph;

    return icmph->type;
}

static __always_inline int
parse_icmphdr_common(struct hdr_cursor *nh, void *data_end,
                     struct icmphdr_common **icmphdr) {
    struct icmphdr_common *h = nh->pos;

    if (h + 1 > data_end)
        return -1;

    nh->pos = h + 1;
    *icmphdr = h;

    return h->type;
}

/*
 * parse_tcphdr: parse the udp header and return the length of the udp payload
 */
static __always_inline int parse_udphdr(struct hdr_cursor *nh, void *data_end,
                                        struct udphdr **udphdr) {
    int len;
    struct udphdr *h = nh->pos;

    if (h + 1 > data_end)
        return -1;

    nh->pos = h + 1;
    *udphdr = h;

    len = bpf_ntohs(h->len) - sizeof(struct udphdr);
    if (len < 0)
        return -1;

    return len;
}

/*
 * parse_tcphdr: parse and return the length of the tcp header
 */
static __always_inline int parse_tcphdr(struct hdr_cursor *nh, void *data_end,
                                        struct tcphdr **tcphdr) {
    int len;
    struct tcphdr *h = nh->pos;

    if (h + 1 > data_end)
        return -1;

    len = h->doff * 4;
    if ((void *)h + len > data_end)
        return -1;

    nh->pos = h + 1;
    *tcphdr = h;

    return len;
}
// Parse INT MD 0.5 headers
static __always_inline int parse_int_md_hdr(
    struct hdr_cursor *nh, void *data_end, struct int_shim_hdr **intshimh,
    struct int_metadata_hdr **intmdh, struct int_metadata_entry **intmdsrc,
    __u32 **seq_num, __u32 **extra_bytes) {
    int len;

    // parse int_shim_hdr
    struct int_shim_hdr *tmp_intshimh = nh->pos;
    if (tmp_intshimh + 1 > data_end)
        return -1;

    len = sizeof(struct int_shim_hdr);
    if ((void *)tmp_intshimh + len > data_end)
        return -1;

    nh->pos = tmp_intshimh + 1;
    *intshimh = tmp_intshimh;

    // parse int_metadata_hdr
    struct int_metadata_hdr *tmp_intmdh = nh->pos;
    if (tmp_intmdh + 1 > data_end)
        return -1;

    len = sizeof(struct int_metadata_hdr);
    if ((void *)tmp_intmdh + len > data_end)
        return -1;

    nh->pos = tmp_intmdh + 1;
    *intmdh = tmp_intmdh;

    // parse int_metadata_entry
    struct int_metadata_entry *tmp_intmdsrc = nh->pos;
    if (tmp_intmdsrc + 1 > data_end)
        return -1;

    len = sizeof(struct int_metadata_entry);
    if ((void *)tmp_intmdsrc + len > data_end)
        return -1;

    nh->pos = tmp_intmdsrc + 1;
    *intmdsrc = tmp_intmdsrc;

    // parse seq_num
    __u32 *tmp_seqnum = nh->pos;
    if (tmp_seqnum + 1 > data_end)
        return -1;

    len = sizeof(__u32);
    if ((void *)tmp_seqnum + len > data_end)
        return -1;

    nh->pos = tmp_seqnum + 1;
    *seq_num = tmp_seqnum;

    // parse extra_bytes;
    __u32 *tmp_extra_bytes = nh->pos;
    if (tmp_extra_bytes + 1 > data_end)
        return -1;

    len = sizeof(__u32);
    if ((void *)tmp_extra_bytes + len > data_end)
        return -1;

    nh->pos = tmp_extra_bytes + 1;
    *extra_bytes = tmp_extra_bytes;

    return 0;
}

// Parse INT MX 2.1 headers
static __always_inline int parse_int_mx_hdr(
    struct hdr_cursor *nh, void *data_end, struct int_mx_shim_hdr **intshimh,
    struct int_mx_md_hdr **intmdh, struct int_mx_ds_src_md **intdsmdh) {
    int len;

    // parse int_mx_shim_hdr
    struct int_mx_shim_hdr *tmp_intshimh = nh->pos;
    if (tmp_intshimh + 1 > data_end)
        return -1;

    len = sizeof(struct int_mx_shim_hdr);
    if ((void *)tmp_intshimh + len > data_end)
        return -1;

    nh->pos = tmp_intshimh + 1;
    *intshimh = tmp_intshimh;

    // parse int_mx_md_hdr
    struct int_mx_md_hdr *tmp_intmdh = nh->pos;
    if (tmp_intmdh + 1 > data_end)
        return -1;

    len = sizeof(struct int_mx_md_hdr);
    if ((void *)tmp_intmdh + len > data_end)
        return -1;

    nh->pos = tmp_intmdh + 1;
    *intmdh = tmp_intmdh;

    // parse int_mx_ds_src_md
    struct int_mx_ds_src_md *tmp_intdsmdh = nh->pos;
    if (tmp_intdsmdh + 1 > data_end)
        return -1;

    len = sizeof(struct int_mx_ds_src_md);
    if ((void *)tmp_intdsmdh + len > data_end)
        return -1;

    nh->pos = tmp_intdsmdh + 1;
    *intdsmdh = tmp_intdsmdh;

    return 0;
}

#endif /* __PARSING_HELPERS_H */

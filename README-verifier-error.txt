If you want to reproduce the verifier error I have seen, first follow
all instructions in this project's top level README for installing build
dependencies, and installing `hostintd` code, at least up to `sudo make
install` step.

Then edit the following line in the file `/etc/hostintd.cfg` that the
`sudo make install` installs on your system, and replace
`v4` with an interface name that exists on your system.

```
DEV=v4
```

Try to start the hostintd service, which attempts to load the
`intmd_xdp_ksink.o` EBPF program:

```bash
$ sudo systemctl start hostintd
```

To see the error messages from the attempt, most of which are from
the verifier when trying but failing to load the EBPF program, see the
command and sample output from my system below.

Why the verifier error?  My best guess is that it is because I have not
satisfied one of the requirements of using EBPF spin locks listed in the
`bpf_spin_lock` section of the bpf-helpers man page:

    https://man7.org/linux/man-pages/man7/bpf-helpers.7.html

That requirement is: "BTF description of the map is mandatory."

My question is, precisely what steps can I follow on an Ubuntu 20.04
development system to satisfy this requirement?  I can try adapting
instructions from other Linux distributions to my preferred Ubuntu
20.04 development system, but so far my attempts to try to compile the
samples/bpf programs in the Linux kernel fail to compile at least some
of the sample programs, for example.

```bash
$ systemctl status --lines=1000 hostintd

● hostintd.service - HostINT Daemon
     Loaded: loaded (/lib/systemd/system/hostintd.service; disabled; vendor preset: enabled)
     Active: failed (Result: exit-code) since Tue 2021-08-03 08:27:06 PDT; 25min ago
    Process: 4933 ExecStartPre=/bin/sh -c /bin/mount | /bin/grep bpf || /bin/mount -t bpf bpf /sys/fs/bpf/ (code=exited, status=0/SUCCESS)
    Process: 4936 ExecStart=/sbin/hostintd -d $DEV -n $NODEID $OPT (code=exited, status=40)
   Main PID: 4936 (code=exited, status=40)

Aug 03 08:27:06 andy-vm systemd[1]: Starting HostINT Daemon...
Aug 03 08:27:06 andy-vm sh[4935]: none on /sys/fs/bpf type bpf (rw,nosuid,nodev,noexec,relatime,mode=700)
Aug 03 08:27:06 andy-vm systemd[1]: Started HostINT Daemon.
Aug 03 08:27:06 andy-vm hostintd[4936]: Will send report to file /var/log/hostintd_report.log
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf: elf: skipping unrecognized data section(6) .rodata.str1.1
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf: load bpf program failed: Permission denied
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf: -- BEGIN DUMP LOG ---
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf:
Aug 03 08:27:06 andy-vm hostintd[4936]: btf_vmlinux is malformed
Aug 03 08:27:06 andy-vm hostintd[4936]: Unrecognized arg#0 type PTR
Aug 03 08:27:06 andy-vm hostintd[4936]: ; int sink_func(struct xdp_md *ctx) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 0: (bf) r8 = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; void *data = (void *)(long)ctx->data;
Aug 03 08:27:06 andy-vm hostintd[4936]: 1: (61) r6 = *(u32 *)(r8 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; void *data_end = (void *)(long)ctx->data_end;
Aug 03 08:27:06 andy-vm hostintd[4936]: 2: (61) r7 = *(u32 *)(r8 +4)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; unsigned long curr_ts = get_bpf_timestamp();
Aug 03 08:27:06 andy-vm hostintd[4936]: 3: (85) call bpf_ktime_get_ns#5
Aug 03 08:27:06 andy-vm hostintd[4936]: 4: (bf) r9 = r0
Aug 03 08:27:06 andy-vm hostintd[4936]: 5: (b7) r1 = 8
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u16 time_offset_key = CONFIG_MAP_KEY_TIME_OFFSET;
Aug 03 08:27:06 andy-vm hostintd[4936]: 6: (6b) *(u16 *)(r10 -58) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 7: (bf) r2 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 8: (07) r2 += -58
Aug 03 08:27:06 andy-vm hostintd[4936]: ; bpf_map_lookup_elem(&sink_config_map, &time_offset_key);
Aug 03 08:27:06 andy-vm hostintd[4936]: 9: (18) r1 = 0xffff9af5f4c16000
Aug 03 08:27:06 andy-vm hostintd[4936]: 11: (85) call bpf_map_lookup_elem#1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (time_offset != NULL) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 12: (15) if r0 == 0x0 goto pc+3
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R6=pkt(id=0,off=0,r=0,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=inv(id=0) R10=fp0 fp-64=mm??????
Aug 03 08:27:06 andy-vm hostintd[4936]: ; curr_ts += *time_offset;
Aug 03 08:27:06 andy-vm hostintd[4936]: 13: (79) r1 = *(u64 *)(r0 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R6=pkt(id=0,off=0,r=0,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=inv(id=0) R10=fp0 fp-64=mm??????
Aug 03 08:27:06 andy-vm hostintd[4936]: ; curr_ts += *time_offset;
Aug 03 08:27:06 andy-vm hostintd[4936]: 14: (0f) r1 += r9
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 15: (bf) r9 = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 16: (7b) *(u64 *)(r10 -368) = r9
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (nh->pos + hdrsize > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 17: (bf) r9 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 18: (07) r9 += 14
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (nh->pos + hdrsize > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 19: (2d) if r9 > r7 goto pc+252
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1_w=inv(id=0) R6=pkt(id=0,off=0,r=14,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=14,r=14,imm=0) R10=fp0 fp-64=mm?????? fp-368_w=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 20: (61) r4 = *(u32 *)(r8 +12)
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 21: (71) r1 = *(u8 *)(r6 +13)
Aug 03 08:27:06 andy-vm hostintd[4936]: 22: (67) r1 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 23: (71) r2 = *(u8 *)(r6 +12)
Aug 03 08:27:06 andy-vm hostintd[4936]: 24: (4f) r1 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 25: (15) if r1 == 0xa888 goto pc+1
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv(id=0) R2=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=14,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=14,r=14,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 26: (55) if r1 != 0x81 goto pc+41
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=14,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=14,r=14,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 27: (bf) r2 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 28: (07) r2 += 18
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 29: (2d) if r2 > r7 goto pc+38
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2_w=pkt(id=0,off=18,r=18,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=18,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=14,r=18,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 30: (71) r3 = *(u8 *)(r6 +16)
Aug 03 08:27:06 andy-vm hostintd[4936]: 31: (71) r1 = *(u8 *)(r6 +17)
Aug 03 08:27:06 andy-vm hostintd[4936]: 32: (67) r1 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 33: (4f) r1 |= r3
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 34: (15) if r1 == 0xa888 goto pc+2
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv(id=0) R2=pkt(id=0,off=18,r=18,imm=0) R3=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=18,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=14,r=18,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 35: (bf) r9 = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 36: (55) if r1 != 0x81 goto pc+31
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2=pkt(id=0,off=18,r=18,imm=0) R3=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=18,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=18,r=18,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 37: (bf) r3 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 38: (07) r3 += 22
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 39: (bf) r9 = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 40: (2d) if r3 > r7 goto pc+27
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2=pkt(id=0,off=18,r=22,imm=0) R3_w=pkt(id=0,off=22,r=22,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=22,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=18,r=22,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 41: (71) r2 = *(u8 *)(r6 +20)
Aug 03 08:27:06 andy-vm hostintd[4936]: 42: (71) r1 = *(u8 *)(r6 +21)
Aug 03 08:27:06 andy-vm hostintd[4936]: 43: (67) r1 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 44: (4f) r1 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 45: (15) if r1 == 0xa888 goto pc+2
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv(id=0) R2=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R3=pkt(id=0,off=22,r=22,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=22,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=18,r=22,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 46: (bf) r9 = r3
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 47: (55) if r1 != 0x81 goto pc+20
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R3=pkt(id=0,off=22,r=22,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=22,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=22,r=22,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 48: (bf) r2 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 49: (07) r2 += 26
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 50: (bf) r9 = r3
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 51: (2d) if r2 > r7 goto pc+16
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2_w=pkt(id=0,off=26,r=26,imm=0) R3=pkt(id=0,off=22,r=26,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=26,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=22,r=26,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 52: (71) r3 = *(u8 *)(r6 +24)
Aug 03 08:27:06 andy-vm hostintd[4936]: 53: (71) r1 = *(u8 *)(r6 +25)
Aug 03 08:27:06 andy-vm hostintd[4936]: 54: (67) r1 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 55: (4f) r1 |= r3
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 56: (15) if r1 == 0xa888 goto pc+2
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv(id=0) R2=pkt(id=0,off=26,r=26,imm=0) R3=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=26,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=22,r=26,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 57: (bf) r9 = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (!proto_is_vlan(h_proto))
Aug 03 08:27:06 andy-vm hostintd[4936]: 58: (55) if r1 != 0x81 goto pc+9
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2=pkt(id=0,off=26,r=26,imm=0) R3=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=26,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=26,r=26,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 59: (bf) r3 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 60: (07) r3 += 30
Aug 03 08:27:06 andy-vm hostintd[4936]: 61: (bf) r9 = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (vlh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 62: (2d) if r3 > r7 goto pc+5
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1=inv129 R2=pkt(id=0,off=26,r=30,imm=0) R3_w=pkt(id=0,off=30,r=30,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=30,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=0,off=26,r=30,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 63: (71) r2 = *(u8 *)(r6 +28)
Aug 03 08:27:06 andy-vm hostintd[4936]: 64: (71) r1 = *(u8 *)(r6 +29)
Aug 03 08:27:06 andy-vm hostintd[4936]: 65: (67) r1 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 66: (4f) r1 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 67: (bf) r9 = r3
Aug 03 08:27:06 andy-vm hostintd[4936]: 68: (b7) r0 = 2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (eth_type == bpf_htons(ETH_P_IP)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 69: (57) r1 &= 65535
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (eth_type == bpf_htons(ETH_P_IP)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 70: (55) if r1 != 0x8 goto pc+315
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0_w=inv2 R1_w=inv8 R2=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R3=pkt(id=0,off=30,r=30,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=30,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=30,r=30,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (iph + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 71: (bf) r1 = r9
Aug 03 08:27:06 andy-vm hostintd[4936]: 72: (07) r1 += 20
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (iph + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 73: (2d) if r1 > r7 goto pc+247
Aug 03 08:27:06 andy-vm systemd[1]: hostintd.service: Main process exited, code=exited, status=40/n/a
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0_w=inv2 R1_w=pkt(id=0,off=50,r=50,imm=0) R2=inv(id=0,umax_value=255,var_off=(0x0; 0xff)) R3=pkt(id=0,off=30,r=50,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=30,r=50,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; hdrsize = iph->ihl * 4;
Aug 03 08:27:06 andy-vm hostintd[4936]: 74: (71) r1 = *(u8 *)(r9 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; hdrsize = iph->ihl * 4;
Aug 03 08:27:06 andy-vm hostintd[4936]: 75: (67) r1 <<= 2
Aug 03 08:27:06 andy-vm hostintd[4936]: 76: (57) r1 &= 60
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (nh->pos + hdrsize > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 77: (bf) r2 = r9
Aug 03 08:27:06 andy-vm hostintd[4936]: 78: (0f) r2 += r1
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 78 first_idx 68
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 77: (bf) r2 = r9
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 76: (57) r1 &= 60
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 75: (67) r1 <<= 2
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 74: (71) r1 = *(u8 *)(r9 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (nh->pos + hdrsize > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 79: (2d) if r2 > r7 goto pc+241
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1=invP(id=0,umax_value=60,var_off=(0x0; 0x3c)) R2=pkt(id=2,off=30,r=30,umax_value=60,var_off=(0x0; 0x3c)) R3=pkt(id=0,off=30,r=50,imm=0) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=30,r=50,imm=0) R10=fp0 fp-64=mm?????? fp-368=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 80: (7b) *(u64 *)(r10 -392) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 81: (7b) *(u64 *)(r10 -400) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: ; return iph->protocol;
Aug 03 08:27:06 andy-vm hostintd[4936]: 82: (71) r1 = *(u8 *)(r9 +9)
Aug 03 08:27:06 andy-vm hostintd[4936]: 83: (7b) *(u64 *)(r10 -384) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 84: (b7) r1 = 2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u16 dscp_val_key = CONFIG_MAP_KEY_DSCP_VAL;
Aug 03 08:27:06 andy-vm hostintd[4936]: 85: (6b) *(u16 *)(r10 -60) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 86: (bf) r2 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 87: (07) r2 += -60
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u32 *dscp_val_ptr = bpf_map_lookup_elem(&sink_config_map, &dscp_val_key);
Aug 03 08:27:06 andy-vm hostintd[4936]: 88: (18) r1 = 0xffff9af5f4c16000
Aug 03 08:27:06 andy-vm hostintd[4936]: 90: (85) call bpf_map_lookup_elem#1
Aug 03 08:27:06 andy-vm hostintd[4936]: 91: (7b) *(u64 *)(r10 -376) = r0
Aug 03 08:27:06 andy-vm hostintd[4936]: 92: (b7) r1 = 3
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u16 dscp_mask_key = CONFIG_MAP_KEY_DSCP_MASK;
Aug 03 08:27:06 andy-vm hostintd[4936]: 93: (6b) *(u16 *)(r10 -62) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 94: (bf) r2 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 95: (07) r2 += -62
Aug 03 08:27:06 andy-vm hostintd[4936]: ; bpf_map_lookup_elem(&sink_config_map, &dscp_mask_key);
Aug 03 08:27:06 andy-vm hostintd[4936]: 96: (18) r1 = 0xffff9af5f4c16000
Aug 03 08:27:06 andy-vm hostintd[4936]: 98: (85) call bpf_map_lookup_elem#1
Aug 03 08:27:06 andy-vm hostintd[4936]: 99: (79) r2 = *(u64 *)(r10 -376)
Aug 03 08:27:06 andy-vm hostintd[4936]: 100: (b7) r4 = 4
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 101: (b7) r1 = 4
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (dscp_val_ptr != NULL && dscp_mask_ptr != NULL) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 102: (15) if r2 == 0x0 goto pc+5
Aug 03 08:27:06 andy-vm systemd[1]: hostintd.service: Failed with result 'exit-code'.
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0_w=map_value_or_null(id=4,off=0,ks=2,vs=8,imm=0) R1_w=inv4 R2_w=map_value(id=0,off=0,ks=2,vs=8,imm=0) R4_w=inv4 R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=30,r=50,imm=0) R10=fp0 fp-64=mmmmmm?? fp-368=mmmmmmmm fp-376_w=map_value fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 103: (b7) r1 = 4
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (dscp_val_ptr != NULL && dscp_mask_ptr != NULL) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 104: (55) if r0 != 0x0 goto pc+1
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv0 R1=inv4 R2=map_value(id=0,off=0,ks=2,vs=8,imm=0) R4=inv4 R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=30,r=50,imm=0) R10=fp0 fp-64=mmmmmm?? fp-368=mmmmmmmm fp-376=map_value fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 105: (05) goto pc+2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((iph->tos & dscp_mask) != (dscp_val & dscp_mask)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 108: (71) r2 = *(u8 *)(r9 +1)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((iph->tos & dscp_mask) != (dscp_val & dscp_mask)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 109: (bf) r3 = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 110: (5f) r3 &= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 111: (7b) *(u64 *)(r10 -376) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((iph->tos & dscp_mask) != (dscp_val & dscp_mask)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 112: (bf) r2 = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 113: (5f) r2 &= r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((iph->tos & dscp_mask) != (dscp_val & dscp_mask)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 114: (57) r2 &= 255
Aug 03 08:27:06 andy-vm hostintd[4936]: 115: (b7) r0 = 2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((iph->tos & dscp_mask) != (dscp_val & dscp_mask)) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 116: (5d) if r3 != r2 goto pc+269
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1=inv4 R2=inv4 R3=inv4 R4=inv4 R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=0,off=30,r=50,imm=0) R10=fp0 fp-64=mmmmmm?? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 117: (b7) r1 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: ; struct flow_key key = {};
Aug 03 08:27:06 andy-vm hostintd[4936]: 118: (63) *(u32 *)(r10 -72) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 118 first_idx 116
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 117: (b7) r1 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.saddr = bpf_ntohl(iph->saddr);
Aug 03 08:27:06 andy-vm hostintd[4936]: 119: (61) r2 = *(u32 *)(r9 +12)
Aug 03 08:27:06 andy-vm hostintd[4936]: 120: (dc) r2 = be32 r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.saddr = bpf_ntohl(iph->saddr);
Aug 03 08:27:06 andy-vm hostintd[4936]: 121: (63) *(u32 *)(r10 -80) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.daddr = bpf_ntohl(iph->daddr);
Aug 03 08:27:06 andy-vm hostintd[4936]: 122: (61) r2 = *(u32 *)(r9 +16)
Aug 03 08:27:06 andy-vm hostintd[4936]: 123: (dc) r2 = be32 r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.daddr = bpf_ntohl(iph->daddr);
Aug 03 08:27:06 andy-vm hostintd[4936]: 124: (63) *(u32 *)(r10 -76) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.proto = iph->protocol;
Aug 03 08:27:06 andy-vm hostintd[4936]: 125: (71) r2 = *(u8 *)(r9 +9)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.proto = iph->protocol;
Aug 03 08:27:06 andy-vm hostintd[4936]: 126: (73) *(u8 *)(r10 -68) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __builtin_memcpy(&eth_cpy, eth, sizeof(eth_cpy));
Aug 03 08:27:06 andy-vm hostintd[4936]: 127: (71) r2 = *(u8 *)(r6 +9)
Aug 03 08:27:06 andy-vm hostintd[4936]: 128: (67) r2 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 129: (71) r3 = *(u8 *)(r6 +8)
Aug 03 08:27:06 andy-vm hostintd[4936]: 130: (4f) r2 |= r3
Aug 03 08:27:06 andy-vm hostintd[4936]: 131: (71) r3 = *(u8 *)(r6 +11)
Aug 03 08:27:06 andy-vm hostintd[4936]: 132: (67) r3 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 133: (71) r4 = *(u8 *)(r6 +10)
Aug 03 08:27:06 andy-vm hostintd[4936]: 134: (4f) r3 |= r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 135: (67) r3 <<= 16
Aug 03 08:27:06 andy-vm hostintd[4936]: 136: (4f) r3 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 137: (71) r4 = *(u8 *)(r6 +1)
Aug 03 08:27:06 andy-vm hostintd[4936]: 138: (67) r4 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 139: (71) r2 = *(u8 *)(r6 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: 140: (4f) r4 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 141: (71) r2 = *(u8 *)(r6 +3)
Aug 03 08:27:06 andy-vm hostintd[4936]: 142: (67) r2 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 143: (71) r5 = *(u8 *)(r6 +2)
Aug 03 08:27:06 andy-vm hostintd[4936]: 144: (4f) r2 |= r5
Aug 03 08:27:06 andy-vm hostintd[4936]: 145: (71) r5 = *(u8 *)(r6 +13)
Aug 03 08:27:06 andy-vm hostintd[4936]: 146: (67) r5 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 147: (71) r0 = *(u8 *)(r6 +12)
Aug 03 08:27:06 andy-vm hostintd[4936]: 148: (4f) r5 |= r0
Aug 03 08:27:06 andy-vm hostintd[4936]: 149: (6b) *(u16 *)(r10 -84) = r5
Aug 03 08:27:06 andy-vm hostintd[4936]: 150: (63) *(u32 *)(r10 -88) = r3
Aug 03 08:27:06 andy-vm hostintd[4936]: 151: (67) r2 <<= 16
Aug 03 08:27:06 andy-vm hostintd[4936]: 152: (4f) r2 |= r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 153: (71) r3 = *(u8 *)(r6 +5)
Aug 03 08:27:06 andy-vm hostintd[4936]: 154: (67) r3 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 155: (71) r4 = *(u8 *)(r6 +4)
Aug 03 08:27:06 andy-vm hostintd[4936]: 156: (4f) r3 |= r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 157: (71) r4 = *(u8 *)(r6 +6)
Aug 03 08:27:06 andy-vm hostintd[4936]: 158: (71) r5 = *(u8 *)(r6 +7)
Aug 03 08:27:06 andy-vm hostintd[4936]: 159: (67) r5 <<= 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 160: (4f) r5 |= r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 161: (67) r5 <<= 16
Aug 03 08:27:06 andy-vm hostintd[4936]: 162: (4f) r5 |= r3
Aug 03 08:27:06 andy-vm hostintd[4936]: 163: (67) r5 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 164: (4f) r5 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 165: (7b) *(u64 *)(r10 -96) = r5
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __builtin_memcpy(&iph_cpy, iph, sizeof(iph_cpy));
Aug 03 08:27:06 andy-vm hostintd[4936]: 166: (61) r2 = *(u32 *)(r9 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: 167: (61) r3 = *(u32 *)(r9 +4)
Aug 03 08:27:06 andy-vm hostintd[4936]: 168: (61) r4 = *(u32 *)(r9 +8)
Aug 03 08:27:06 andy-vm hostintd[4936]: 169: (61) r5 = *(u32 *)(r9 +12)
Aug 03 08:27:06 andy-vm hostintd[4936]: 170: (61) r0 = *(u32 *)(r9 +16)
Aug 03 08:27:06 andy-vm hostintd[4936]: 171: (b7) r9 = 1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u16 node_id_key = CONFIG_MAP_KEY_NODE_ID;
Aug 03 08:27:06 andy-vm hostintd[4936]: 172: (6b) *(u16 *)(r10 -146) = r9
Aug 03 08:27:06 andy-vm hostintd[4936]: ; struct tcphdr tcph_cpy = {};
Aug 03 08:27:06 andy-vm hostintd[4936]: 173: (63) *(u32 *)(r10 -128) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 174: (7b) *(u64 *)(r10 -136) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 175: (7b) *(u64 *)(r10 -144) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __builtin_memcpy(&iph_cpy, iph, sizeof(iph_cpy));
Aug 03 08:27:06 andy-vm hostintd[4936]: 176: (63) *(u32 *)(r10 -104) = r0
Aug 03 08:27:06 andy-vm hostintd[4936]: 177: (67) r5 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 178: (4f) r5 |= r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 179: (7b) *(u64 *)(r10 -112) = r5
Aug 03 08:27:06 andy-vm hostintd[4936]: 180: (67) r3 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 181: (4f) r3 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 182: (7b) *(u64 *)(r10 -120) = r3
Aug 03 08:27:06 andy-vm hostintd[4936]: 183: (bf) r2 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 184: (07) r2 += -146
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u32 *node_id = bpf_map_lookup_elem(&sink_config_map, &node_id_key);
Aug 03 08:27:06 andy-vm hostintd[4936]: 185: (18) r1 = 0xffff9af5f4c16000
Aug 03 08:27:06 andy-vm hostintd[4936]: 187: (85) call bpf_map_lookup_elem#1
Aug 03 08:27:06 andy-vm hostintd[4936]: 188: (b7) r4 = 3
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (node_id != NULL) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 189: (15) if r0 == 0x0 goto pc+1
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R4_w=inv3 R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=inv1 R10=fp0 fp-64=mmmmmm?? fp-72=???m0000 fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; sink_node_id = *node_id;
Aug 03 08:27:06 andy-vm hostintd[4936]: 190: (61) r4 = *(u32 *)(r0 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R4_w=inv3 R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=inv1 R10=fp0 fp-64=mmmmmm?? fp-72=???m0000 fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (ip_type == IPPROTO_UDP) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 191: (79) r1 = *(u64 *)(r10 -384)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (ip_type == IPPROTO_UDP) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 192: (55) if r1 != 0x11 goto pc+194
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=map_value(id=0,off=0,ks=2,vs=8,imm=0) R1_w=inv17 R4_w=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=inv1 R10=fp0 fp-64=mmmmmm?? fp-72=???m0000 fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 193: (79) r2 = *(u64 *)(r10 -392)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (h + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 194: (bf) r9 = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 195: (07) r9 += 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 196: (b7) r0 = 2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (h + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 197: (2d) if r9 > r7 goto pc+3
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1=inv17 R2=pkt(id=2,off=30,r=38,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=38,r=38,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???m0000 fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; len = bpf_ntohs(h->len) - sizeof(struct udphdr);
Aug 03 08:27:06 andy-vm hostintd[4936]: 198: (69) r1 = *(u16 *)(r2 +4)
Aug 03 08:27:06 andy-vm hostintd[4936]: 199: (dc) r1 = be16 r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (len < 0)
Aug 03 08:27:06 andy-vm hostintd[4936]: 200: (25) if r1 > 0x7 goto pc+258
Aug 03 08:27:06 andy-vm hostintd[4936]: from 200 to 459: R0=inv2 R1_w=inv(id=0,umin_value=8) R2=pkt(id=2,off=30,r=38,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=38,r=38,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???m0000 fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.sport = bpf_ntohs(udph->source);
Aug 03 08:27:06 andy-vm hostintd[4936]: 459: (69) r1 = *(u16 *)(r2 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: 460: (dc) r1 = be16 r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.sport = bpf_ntohs(udph->source);
Aug 03 08:27:06 andy-vm hostintd[4936]: 461: (6b) *(u16 *)(r10 -72) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.dport = bpf_ntohs(udph->dest);
Aug 03 08:27:06 andy-vm hostintd[4936]: 462: (69) r1 = *(u16 *)(r2 +2)
Aug 03 08:27:06 andy-vm hostintd[4936]: 463: (dc) r1 = be16 r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; key.dport = bpf_ntohs(udph->dest);
Aug 03 08:27:06 andy-vm hostintd[4936]: 464: (6b) *(u16 *)(r10 -70) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_intshimh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 465: (07) r9 += 4
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_intshimh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 466: (2d) if r9 > r7 goto pc+10
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1=inv(id=0) R2=pkt(id=2,off=30,r=42,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=42,r=42,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_intmdh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 467: (07) r9 += 8
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_intmdh + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 468: (2d) if r9 > r7 goto pc+8
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1=inv(id=0) R2=pkt(id=2,off=30,r=50,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=2,off=50,r=50,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_intmdsrc + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 469: (bf) r1 = r9
Aug 03 08:27:06 andy-vm hostintd[4936]: 470: (07) r1 += 16
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_intmdsrc + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 471: (2d) if r1 > r7 goto pc+5
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1_w=pkt(id=2,off=66,r=66,umax_value=60,var_off=(0x0; 0x3c)) R2=pkt(id=2,off=30,r=66,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9_w=pkt(id=2,off=50,r=66,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_seqnum + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 472: (bf) r2 = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 473: (07) r2 += 4
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_seqnum + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 474: (2d) if r2 > r7 goto pc+2
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1=pkt(id=2,off=66,r=70,umax_value=60,var_off=(0x0; 0x3c)) R2=pkt(id=2,off=70,r=70,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=70,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 475: (07) r1 += 8
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (tmp_seqnum + 1 > data_end)
Aug 03 08:27:06 andy-vm hostintd[4936]: 476: (3d) if r7 >= r1 goto pc+158
Aug 03 08:27:06 andy-vm hostintd[4936]: from 476 to 635: R0=inv2 R1_w=pkt(id=2,off=74,r=74,umax_value=60,var_off=(0x0; 0x3c)) R2=pkt(id=2,off=70,r=74,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=50,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((int_data + 32) > data_end) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 635: (bf) r1 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 636: (07) r1 += 74
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if ((int_data + 32) > data_end) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 637: (2d) if r1 > r7 goto pc-252
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1_w=pkt(id=0,off=74,r=74,imm=0) R2=pkt(id=2,off=70,r=74,umax_value=60,var_off=(0x0; 0x3c)) R4=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 638: (7b) *(u64 *)(r10 -424) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 639: (79) r3 = *(u64 *)(r10 -392)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __builtin_memcpy(&udph_cpy, udph, sizeof(udph_cpy));
Aug 03 08:27:06 andy-vm hostintd[4936]: 640: (69) r1 = *(u16 *)(r3 +2)
Aug 03 08:27:06 andy-vm hostintd[4936]: 641: (67) r1 <<= 16
Aug 03 08:27:06 andy-vm hostintd[4936]: 642: (69) r2 = *(u16 *)(r3 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: 643: (4f) r1 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 644: (69) r2 = *(u16 *)(r3 +4)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; csum = (__u32)(~udph->check);
Aug 03 08:27:06 andy-vm hostintd[4936]: 645: (69) r5 = *(u16 *)(r3 +6)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __builtin_memcpy(&udph_cpy, udph, sizeof(udph_cpy));
Aug 03 08:27:06 andy-vm hostintd[4936]: 646: (bf) r4 = r5
Aug 03 08:27:06 andy-vm hostintd[4936]: 647: (67) r4 <<= 16
Aug 03 08:27:06 andy-vm hostintd[4936]: 648: (4f) r4 |= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 649: (67) r4 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 650: (4f) r4 |= r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 651: (7b) *(u64 *)(r10 -456) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 652: (b7) r1 = 8
Aug 03 08:27:06 andy-vm hostintd[4936]: 653: (7b) *(u64 *)(r10 -432) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 654: (b7) r2 = 42
Aug 03 08:27:06 andy-vm hostintd[4936]: ; pkt_seq_num = bpf_ntohl(*seq_num);
Aug 03 08:27:06 andy-vm hostintd[4936]: 655: (61) r1 = *(u32 *)(r3 +36)
Aug 03 08:27:06 andy-vm hostintd[4936]: 656: (05) goto pc+10
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 667: (dc) r1 = be32 r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 668: (7b) *(u64 *)(r10 -416) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 669: (bf) r1 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: 670: (0f) r1 += r2
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 670 first_idx 667
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 669: (bf) r1 = r6
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 668: (7b) *(u64 *)(r10 -416) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 667: (dc) r1 = be32 r1
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv2 R1_rw=inv(id=0,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R2_rw=invP42 R3_w=pkt(id=2,off=30,r=74,umax_value=60,var_off=(0x0; 0x3c)) R4_w=inv(id=0) R5_w=inv(id=0,umax_value=65535,var_off=(0x0; 0xffff)) R6_r=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-424_w=mmmmmmmm fp-432_w=inv8 fp-456_w=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: parent didn't have regs=4 stack=0 marks
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 656 first_idx 474
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 656: (05) goto pc+10
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 655: (61) r1 = *(u32 *)(r3 +36)
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 654: (b7) r2 = 42
Aug 03 08:27:06 andy-vm hostintd[4936]: 671: (a7) r5 ^= -1
Aug 03 08:27:06 andy-vm hostintd[4936]: 672: (47) r5 |= -65536
Aug 03 08:27:06 andy-vm hostintd[4936]: 673: (b7) r2 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 674: (7b) *(u64 *)(r10 -408) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 674 first_idx 667
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 673: (b7) r2 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 675: (b7) r2 = 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 676: (b7) r3 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 677: (b7) r4 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 678: (85) call bpf_csum_diff#28
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 678 first_idx 667
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 677: (b7) r4 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 676: (b7) r3 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=4 stack=0 before 675: (b7) r2 = 32
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 678 first_idx 667
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=10 stack=0 before 677: (b7) r4 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 679: (7b) *(u64 *)(r10 -448) = r0
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u32 e2e_latency_ns = ((__u32)curr_ts - bpf_ntohl(intmdsrc->ingress_ts));
Aug 03 08:27:06 andy-vm hostintd[4936]: 680: (61) r2 = *(u32 *)(r9 +8)
Aug 03 08:27:06 andy-vm hostintd[4936]: 681: (dc) r2 = be32 r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; __u32 e2e_latency_ns = ((__u32)curr_ts - bpf_ntohl(intmdsrc->ingress_ts));
Aug 03 08:27:06 andy-vm hostintd[4936]: 682: (79) r1 = *(u64 *)(r10 -368)
Aug 03 08:27:06 andy-vm hostintd[4936]: 683: (1f) r1 -= r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 684: (bf) r2 = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 685: (67) r2 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 686: (77) r2 >>= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 687: (b7) r3 = 50000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (e2e_latency_ns < 50 * MILLI_TO_NANO) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 688: (2d) if r3 > r2 goto pc+21
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0_w=inv(id=0) R1_w=inv(id=0) R2_w=inv(id=0,umin_value=50000000,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R3_w=inv50000000 R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-408_w=00000000 fp-416_w=mmmmmmmm fp-424=mmmmmmmm fp-432=inv8 fp-448_w=mmmmmmmm fp-456=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 689: (b7) r3 = 100000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 690: (b7) r4 = 50000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ; } else if (e2e_latency_ns < 100 * MILLI_TO_NANO) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 691: (7b) *(u64 *)(r10 -408) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 692: (2d) if r3 > r2 goto pc+17
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv(id=0) R1=inv(id=0) R2=inv(id=0,umin_value=100000000,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R3=inv100000000 R4=inv50000000 R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-408=inv50000000 fp-416=mmmmmmmm fp-424=mmmmmmmm fp-432=inv8 fp-448=mmmmmmmm fp-456=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; } else if (e2e_latency_ns < 225 * MILLI_TO_NANO) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 693: (bf) r3 = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 694: (67) r3 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 695: (77) r3 >>= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 696: (b7) r2 = 225000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 697: (b7) r4 = 100000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ; } else if (e2e_latency_ns < 225 * MILLI_TO_NANO) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 698: (7b) *(u64 *)(r10 -408) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 699: (2d) if r2 > r3 goto pc+10
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv(id=0) R1=inv(id=0) R2_w=inv225000000 R3_w=inv(id=0,umin_value=225000000,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R4_w=inv100000000 R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-408_w=inv100000000 fp-416=mmmmmmmm fp-424=mmmmmmmm fp-432=inv8 fp-448=mmmmmmmm fp-456=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 700: (b7) r2 = 500000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 701: (b7) r4 = 225000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ; } else if (e2e_latency_ns < 500 * MILLI_TO_NANO) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 702: (7b) *(u64 *)(r10 -408) = r4
Aug 03 08:27:06 andy-vm hostintd[4936]: 703: (2d) if r2 > r3 goto pc+6
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv(id=0) R1=inv(id=0) R2=inv500000000 R3=inv(id=0,umin_value=500000000,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R4=inv225000000 R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-408=inv225000000 fp-416=mmmmmmmm fp-424=mmmmmmmm fp-432=inv8 fp-448=mmmmmmmm fp-456=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: ; } else if (e2e_latency_ns < 750 * MILLI_TO_NANO) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 704: (67) r1 <<= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 705: (77) r1 >>= 32
Aug 03 08:27:06 andy-vm hostintd[4936]: 706: (b7) r3 = 750000000
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 707: (2d) if r3 > r1 goto pc+1
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv(id=0) R1_w=inv(id=0,umin_value=750000000,umax_value=4294967295,var_off=(0x0; 0xffffffff)) R2=inv500000000 R3_w=inv750000000 R4=inv225000000 R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-408=inv225000000 fp-416=mmmmmmmm fp-424=mmmmmmmm fp-432=inv8 fp-448=mmmmmmmm fp-456=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 708: (b7) r2 = 750000000
Aug 03 08:27:06 andy-vm hostintd[4936]: 709: (7b) *(u64 *)(r10 -408) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 710: (bf) r2 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: ;
Aug 03 08:27:06 andy-vm hostintd[4936]: 711: (07) r2 += -80
Aug 03 08:27:06 andy-vm hostintd[4936]: ; bpf_map_lookup_elem(&sink_flow_stats_map, &key);
Aug 03 08:27:06 andy-vm hostintd[4936]: 712: (18) r1 = 0xffff9af5f4c16800
Aug 03 08:27:06 andy-vm hostintd[4936]: 714: (85) call bpf_map_lookup_elem#1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; if (flow_stats_rec == NULL) {
Aug 03 08:27:06 andy-vm hostintd[4936]: 715: (55) if r0 != 0x0 goto pc+35
Aug 03 08:27:06 andy-vm hostintd[4936]:  R0=inv0 R6=pkt(id=0,off=0,r=74,imm=0) R7=pkt_end(id=0,off=0,imm=0) R8=ctx(id=0,off=0,imm=0) R9=pkt(id=2,off=50,r=74,umax_value=60,var_off=(0x0; 0x3c)) R10=fp0 fp-64=mmmmmm?? fp-72=???mmmmm fp-80=mmmmmmmm fp-88=??mmmmmm fp-96=mmmmmmmm fp-104=????mmmm fp-112=mmmmmmmm fp-120=mmmmmmmm fp-128=????0000 fp-136=00000000 fp-144=00000000 fp-152=mm?????? fp-368=mmmmmmmm fp-376=inv4 fp-384=mmmmmmmm fp-392=pkt fp-400=mmmmmmmm fp-408=inv750000000 fp-416=mmmmmmmm fp-424=mmmmmmmm fp-432=inv8 fp-448=mmmmmmmm fp-456=mmmmmmmm
Aug 03 08:27:06 andy-vm hostintd[4936]: 716: (b7) r1 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: ; struct sink_flow_stats_datarec new_flow_stats_rec = {
Aug 03 08:27:06 andy-vm hostintd[4936]: 717: (63) *(u32 *)(r10 -328) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 717 first_idx 715
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 716: (b7) r1 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: ; .src_node_id = bpf_ntohl(intmdsrc->node_id),
Aug 03 08:27:06 andy-vm hostintd[4936]: 718: (61) r1 = *(u32 *)(r9 +0)
Aug 03 08:27:06 andy-vm hostintd[4936]: 719: (dc) r1 = be32 r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; struct sink_flow_stats_datarec new_flow_stats_rec = {
Aug 03 08:27:06 andy-vm hostintd[4936]: 720: (63) *(u32 *)(r10 -324) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; .src_port = bpf_ntohs(intmdsrc->ingress_port),
Aug 03 08:27:06 andy-vm hostintd[4936]: 721: (69) r1 = *(u16 *)(r9 +4)
Aug 03 08:27:06 andy-vm hostintd[4936]: ; struct sink_flow_stats_datarec new_flow_stats_rec = {
Aug 03 08:27:06 andy-vm hostintd[4936]: 722: (79) r2 = *(u64 *)(r10 -408)
Aug 03 08:27:06 andy-vm hostintd[4936]: 723: (7b) *(u64 *)(r10 -272) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 724: (79) r2 = *(u64 *)(r10 -416)
Aug 03 08:27:06 andy-vm hostintd[4936]: 725: (63) *(u32 *)(r10 -296) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 726: (79) r2 = *(u64 *)(r10 -368)
Aug 03 08:27:06 andy-vm hostintd[4936]: 727: (7b) *(u64 *)(r10 -304) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 728: (79) r2 = *(u64 *)(r10 -400)
Aug 03 08:27:06 andy-vm hostintd[4936]: 729: (6b) *(u16 *)(r10 -312) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: 730: (79) r2 = *(u64 *)(r10 -424)
Aug 03 08:27:06 andy-vm hostintd[4936]: 731: (63) *(u32 *)(r10 -316) = r2
Aug 03 08:27:06 andy-vm hostintd[4936]: ; .src_port = bpf_ntohs(intmdsrc->ingress_port),
Aug 03 08:27:06 andy-vm hostintd[4936]: 732: (dc) r1 = be16 r1
Aug 03 08:27:06 andy-vm hostintd[4936]: ; struct sink_flow_stats_datarec new_flow_stats_rec = {
Aug 03 08:27:06 andy-vm hostintd[4936]: 733: (6b) *(u16 *)(r10 -320) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 734: (b7) r1 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 735: (6b) *(u16 *)(r10 -276) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: last_idx 735 first_idx 715
Aug 03 08:27:06 andy-vm hostintd[4936]: regs=2 stack=0 before 734: (b7) r1 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 736: (63) *(u32 *)(r10 -280) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 737: (7b) *(u64 *)(r10 -288) = r1
Aug 03 08:27:06 andy-vm hostintd[4936]: 738: (b7) r9 = 0
Aug 03 08:27:06 andy-vm hostintd[4936]: 739: (bf) r2 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: 740: (07) r2 += -80
Aug 03 08:27:06 andy-vm hostintd[4936]: 741: (bf) r3 = r10
Aug 03 08:27:06 andy-vm hostintd[4936]: 742: (07) r3 += -328
Aug 03 08:27:06 andy-vm hostintd[4936]: ; int ret = bpf_map_update_elem(&sink_flow_stats_map, &key,
Aug 03 08:27:06 andy-vm hostintd[4936]: 743: (18) r1 = 0xffff9af5f4c16800
Aug 03 08:27:06 andy-vm hostintd[4936]: 745: (b7) r4 = 1
Aug 03 08:27:06 andy-vm hostintd[4936]: 746: (85) call bpf_map_update_elem#2
Aug 03 08:27:06 andy-vm hostintd[4936]: invalid indirect read from stack off -328+10 size 64
Aug 03 08:27:06 andy-vm hostintd[4936]: processed 426 insns (limit 1000000) max_states_per_insn 1 total_states 21 peak_states 21 mark_read 17
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf: -- END LOG --
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf: failed to load program 'sink_func'
Aug 03 08:27:06 andy-vm hostintd[4936]: libbpf: failed to load object '/usr/lib/hostint/intmd_xdp_ksink.o'
Aug 03 08:27:06 andy-vm hostintd[4936]: Error - loading BPF-OBJ file(/usr/lib/hostint/intmd_xdp_ksink.o) (-4007): Unknown error 4007
Aug 03 08:27:06 andy-vm hostintd[4936]: Error - loading file: /usr/lib/hostint/intmd_xdp_ksink.o
```

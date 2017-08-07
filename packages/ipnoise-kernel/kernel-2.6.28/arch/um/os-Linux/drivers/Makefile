# 
# Copyright (C) 2000, 2002 Jeff Dike (jdike@karaya.com)
# Licensed under the GPL
#

ethertap-objs := ethertap_kern.o ethertap_user.o
tuntap-objs := tuntap_kern.o tuntap_user.o

ipnoise-icq-objs := ipnoise_icq.o ipnoise_base64.o
ipnoise-udp-objs := ipnoise_udp.o ipnoise_base64.o
ipnoise-tcp-objs := ipnoise_tcp.o ipnoise_base64.o
ipnoise-api-objs := ipnoise_api.o ipnoise_base64.o ipnoise_proc.o

obj-y = 
obj-$(CONFIG_UML_NET_ETHERTAP) += ethertap.o
obj-$(CONFIG_UML_NET_TUNTAP) += tuntap.o

## IPNoise
obj-m += ipnoise-api.o
## Moved to user-space via host-os sockets (20101229 morik)
#obj-m += ipnoise-udp.o
#obj-m += ipnoise-tcp.o
#obj-m += ipnoise-icq.o

include arch/um/scripts/Makefile.rules


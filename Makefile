OBJS = \
	arp_frame.o\
	arp.o\
	nic.o\
	bio.o\
	console.o\
	e1000.o\
	exec.o\
	file.o\
	fs.o\
	ide.o\
	ioapic.o\
	kalloc.o\
	kbd.o\
	lapic.o\
	log.o\
	main.o\
	mp.o\
	pci.o\
	picirq.o\
	pipe.o\
	proc.o\
	spinlock.o\
	string.o\
	swtch.o\
	syscall.o\
	sysarp.o\
	sysfile.o\
	sysproc.o\
	timer.o\
	trapasm.o\
	trap.o\
	uart.o\
	vectors.o\
	util.o\
	vm.o\
	select.o\
	monitor.o\
	readline.o\
	printfmt.o\
	kdebug.o\
	assert.o\
	sem.o\
	thread.o\
	lwip_ping.o\
	${LWIP_OBJS}\

LWIP_OBJS = \
			lwip/xv6/arch/sys_arch.o\
            lwip/netif/etharp.o\
            lwip/netif/ethernetif.o\
            lwip/netif/loopif.o\
            lwip/core/ipv4/ip_addr.o\
            lwip/core/ipv4/icmp.o\
            lwip/core/ipv4/ip.o\
            lwip/core/ipv4/ip_frag.o\
            lwip/core/inet.o\
            lwip/core/mem.o\
            lwip/core/memp.o\
            lwip/core/netif.o\
            lwip/core/pbuf.o\
            lwip/core/raw.o\
            lwip/core/stats.o\
            lwip/core/sys.o\
            lwip/core/tcp.o\
            lwip/core/tcp_out.o\
            lwip/core/tcp_in.o\
            lwip/core/udp.o\
            lwip/core/dhcp.o\
            lwip/api/api_lib.o\
            lwip/api/api_msg.o\
            lwip/api/err.o\
            lwip/api/sockets.o\
            lwip/api/tcpip.o\
#            lwip/core/ipv6/ip6_addr.o\
#            lwip/core/ipv6/icmp6.o\
#            lwip/core/ipv6/ip6.o\
#            lwip/core/inet6.o\
#			lwip/netif/ppp/auth.o\
#			lwip/netif/ppp/chap.o\
#            lwip/netif/ppp/chpms.o\
#            lwip/netif/ppp/fsm.o\
#            lwip/netif/ppp/ipcp.o\
#            lwip/netif/ppp/lcp.o\
#            lwip/netif/ppp/magic.o\
#            lwip/netif/ppp/md5.o\
#            lwip/netif/ppp/pap.o\
#            lwip/netif/ppp/ppp.o\
#            lwip/netif/ppp/randm.o\
#            lwip/netif/ppp/vj.o\
#            lwip/netif/slipif.o\

LWIP_INC = -Ilwip/xv6 -Ilwip/include -Ilwip/include/ipv4

# Cross-compiling (e.g., on Mac OS X)
# TOOLPREFIX = i386-jos-elf

# Using native tools (e.g., on X86 Linux)
#TOOLPREFIX = 

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if i386-jos-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-jos-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-jos-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-*-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-jos-elf-', set your TOOLPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# If the makefile can't find QEMU, specify its path here
# QEMU = qemu-system-i386

# Try to infer the correct QEMU
ifndef QEMU
QEMU = $(shell if which qemu > /dev/null; \
	then echo qemu; exit; \
	elif which qemu-system-i386 > /dev/null; \
	then echo qemu-system-i386; exit; \
	else \
	qemu=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu; \
	if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	echo "*** or have you tried setting the QEMU variable in Makefile?" 1>&2; \
	echo "***" 1>&2; exit 1)
endif

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
#CFLAGS = -fno-pic -static -nostdinc -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Wno-format -Wno-unused -Werror -fno-omit-frame-pointer
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O0 -Wall -MD -m32 -Werror -Wno-address -fno-omit-frame-pointer -Wno-unused -Wno-switch -gstabs ${LWIP_INC}
# CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O0 -Wall -MD -m32 -fno-omit-frame-pointer -Wno-unused -gstabs ${LWIP_INC}
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide
# FreeBSD ld wants ``elf_i386_fbsd''
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null)

GCC_LIB := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

xv6.img: bootblock kernel fs.img
	dd if=/dev/zero of=xv6.img count=10000
	dd if=bootblock of=xv6.img conv=notrunc
	dd if=kernel of=xv6.img seek=1 conv=notrunc
	cp xv6.img xv62.img
	cp xv6.img xv63.img
	cp xv6.img xv64.img

xv6memfs.img: bootblock kernelmemfs
	dd if=/dev/zero of=xv6memfs.img count=10000
	dd if=bootblock of=xv6memfs.img conv=notrunc
	dd if=kernelmemfs of=xv6memfs.img seek=1 conv=notrunc

bootblock: bootasm.S bootmain.c
	$(CC) $(CFLAGS) -fno-pic -O -nostdinc -I. -c bootmain.c
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c bootasm.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o bootblock.o bootasm.o bootmain.o
	$(OBJDUMP) -S bootblock.o > bootblock.asm
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	./sign.pl bootblock

entryother: entryother.S
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o bootblockother.o entryother.o
	$(OBJCOPY) -S -O binary -j .text bootblockother.o entryother
	$(OBJDUMP) -S bootblockother.o > entryother.asm

initcode: initcode.S
	$(CC) $(CFLAGS) -nostdinc -I. -c initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode
	$(OBJDUMP) -S initcode.o > initcode.asm

kernel: $(OBJS) entry.o entryother initcode kernel.ld
	$(LD) $(LDFLAGS) -nostdlib -T kernel.ld -o kernel entry.o $(OBJS) $(GCC_LIB) -b binary initcode entryother
	$(OBJDUMP) -S kernel > kernel.asm
	$(OBJDUMP) -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym

# kernelmemfs is a copy of kernel that maintains the
# disk image in memory instead of writing to a disk.
# This is not so useful for testing persistent storage or
# exploring disk buffering implementations, but it is
# great for testing the kernel on real hardware without
# needing a scratch disk.
MEMFSOBJS = $(filter-out ide.o,$(OBJS)) memide.o
kernelmemfs: $(MEMFSOBJS) entry.o entryother initcode kernel.ld fs.img
	$(LD) $(LDFLAGS) -T kernel.ld -o kernelmemfs entry.o  $(MEMFSOBJS) -b binary initcode entryother fs.img
	$(OBJDUMP) -S kernelmemfs > kernelmemfs.asm
	$(OBJDUMP) -t kernelmemfs | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernelmemfs.sym

tags: $(OBJS) entryother.S _init
	etags *.S *.c

vectors.S: vectors.pl
	perl vectors.pl > vectors.S

ULIB = ulib.o usys.o printf.o umalloc.o printfmt.o lwip/core/inet.o lwip/core/ipv4/ip_addr.o
# 	lwip/api/sockets.o
# 	$(OBJS)
# 	lwip/core/sys.o lwip/api/api_lib.o console.o

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^ $(GCC_LIB)
	$(OBJDUMP) -S $@ > $*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $*.sym

# _forktest: forktest.o $(ULIB)
# 	# forktest has less library code linked in - needs to be small
# 	# in order to be able to max out the proc table.
# 	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o _forktest forktest.o ulib.o usys.o
# 	$(OBJDUMP) -S _forktest > forktest.asm

mkfs: mkfs.c fs.h
	gcc -Werror -Wall -o mkfs mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	_arptest\
	_arpserv\
	_cat\
	_echo\
	_grep\
	_init\
	_kill\
	_ln\
	_ls\
	_mkdir\
	_rm\
	_sh\
	_wc\
	_zombie\
 	_ping\
	_xv6Chat\
	_pkill\

fs.img: mkfs README $(UPROGS)
	./mkfs fs.img README $(UPROGS)
	cp fs.img fs2.img
	cp fs.img fs3.img
	cp fs.img fs4.img

-include *.d

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*.o *.d *.asm *.sym vectors.S bootblock entryother \
	initcode initcode.out kernel xv6.img xv62.img xv63.img xv64.img fs.img fs2.img fs3.img fs4.img kernelmemfs mkfs \
	.gdbinit \
	$(UPROGS) \
	${LWIP_OBJS} \
	xv6ChatServer \
# 	find lwip -iname "*.d" -exec rm {} \;
# 	find lwip -iname "*.o" -exec rm {} \;

# make a printout
FILES = $(shell grep -v '^\#' runoff.list)
PRINT = runoff.list runoff.spec README toc.hdr toc.ftr $(FILES)

xv6.pdf: $(PRINT)
	./runoff
	ls -l xv6.pdf

print: xv6.pdf

# run in emulators

bochs : fs.img xv6.img
	if [ ! -e .bochsrc ]; then ln -s dot-bochsrc .bochsrc; fi
	bochs -q
#   sudo bochs -q

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)
ifndef CPUS
CPUS := 1 
endif

PORT7 := $(shell expr $(GDBPORT) + 1)
PORT80 := $(shell expr $(GDBPORT) + 2)

QEMUOPTS =  -smp $(CPUS) -m 512 $(QEMUEXTRA)

## These are the original (from Lab 5):
QEMUSERVOPTS = $(QEMUOPTS) -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw
QEMUSERVOPTS += -netdev tap,id=net1,ifname=tap1,script=/etc/qemu-ifup \
		-device e1000,netdev=net1,mac=12:13:14:15:16:27 -object filter-dump,id=f1,netdev=net1,file=qemuserv.pcap
# QEMUSERVOPTS += -netdev user,id=net1,hostfwd=tcp::5555-:23,ifname=tap1,script=/etc/qemu-ifup \
# 		-device e1000,netdev=net1,mac=12:13:14:15:16:27 -object filter-dump,id=f1,netdev=net1,file=qemuserv.pcap
# -device e1000,netdev=net1,mac=12:13:14:15:16:27 -netdev user,id=net1,hostfwd=tcp::5555-:22 -object filter-dump,id=f1,netdev=net1,file=qemuserv.pcap

# -netdev tap,id=net1,ifname=tap1,script=/etc/qemu-ifup,hostfwd=tcp::$(PORT80)-10.0.3.15:80,hostfwd=udp::$(PORT7)-10.0.3.15:7 \
## ^ hostfwd like this is invalid, must use with -nic instead?

QEMUCLIENTOPTS = $(QEMUOPTS) -drive file=fs2.img,index=1,media=disk,format=raw -drive file=xv62.img,index=0,media=disk,format=raw
QEMUCLIENTOPTS +=  -netdev tap,id=net0,ifname=tap0,script=/etc/qemu-ifup \
			-device e1000,netdev=net0,mac=12:13:14:15:16:17 -object filter-dump,id=f0,netdev=net0,file=qemu.pcap

			

QEMUCLIENT1OPTS = $(QEMUOPTS) -drive file=fs3.img,index=1,media=disk,format=raw -drive file=xv63.img,index=0,media=disk,format=raw
QEMUCLIENT1OPTS += -netdev tap,id=net2,ifname=tap2,script=/etc/qemu-ifup \
			-device e1000,netdev=net2,mac=12:13:14:15:16:37 -object filter-dump,id=f2,netdev=net2,file=qemu2.pcap
			
			
QEMUCLIENT2OPTS = $(QEMUOPTS) -drive file=fs4.img,index=1,media=disk,format=raw -drive file=xv64.img,index=0,media=disk,format=raw
QEMUCLIENT2OPTS += -netdev tap,id=net3,ifname=tap3,script=/etc/qemu-ifup \
			-device e1000,netdev=net3,mac=12:13:14:15:16:47 -object filter-dump,id=f3,netdev=net3,file=qemu3.pcap


#### Refer to: https://qemu.weilnetz.de/doc/qemu-doc.html#Network-options

## Here are my experimental augmentations:
# QEMUSERVOPTS += -nic user,model=e1000,mac=12:13:14:15:16:27,ipv6=off
# QEMUSERVOPTS += -netdev user,id=n0,net=10.0.2.15,dhcpstart=10.0.2.17 \
# 				-device e1000,netdev=n0,mac=12:13:14:15:16:27 -object filter-dump,id=f1,netdev=n0,file=qemuserv.pcap

# QEMUCLIENTOPTS += -nic user,model=e1000,mac=12:13:14:15:16:17,ipv6=off,hostfwd=tcp:
# QEMUCLIENTOPTS += -netdev user,id=n1,net=10.0.2.16,dhcpstart=10.0.2.17 \
# 			-device e1000,netdev=n1,mac=12:13:14:15:16:17 -object filter-dump,id=f0,netdev=n1,file=qemu.pcap

# ^ configures QEMU’s network stack to record all incoming and outgoing packets to qemu.pcap 
#		To get a hex/ASCII dump of captured packets use tcpdump like this:
#		$ tcpdump -XXnr qemu.pcap
## NOTE: '-redir' is reported to be an invalid option

#------------------------------------------------------------------------------
# From Lab 5:
#	1) Run: make qemu-nox-serv in a terminal (now called the server terminal)

#		(Note: if you see the error:
#			invalid object type: filter-dump
# 			Makefile:266: recipe for target 'qemu-nox-serv' failed
# 		you may need to upgrade your version of QEMU to 2.10 or later or alter the makefile to use the older -net dump arguments.)

# 	2) Run: make qemu-nox in a separate terminal (now called the client terminal)
# 	3) In the server terminal, run the arpserv user level program
# 	4) In the client terminal, run the arptest user level program
#------------------------------------------------------------------------------

qemu: fs.img xv6.img
	$(QEMU) -serial mon:stdio $(QEMUCLIENTOPTS)

qemu-memfs: xv6memfs.img
	$(QEMU) -drive file=xv6memfs.img,index=0,media=disk,format=raw -smp $(CPUS) -m 256

qemu-nox: fs.img xv6.img
	$(QEMU) -nographic $(QEMUCLIENTOPTS)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUCLIENTOPTS) -S $(QEMUGDB)

qemu-nox-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUCLIENTOPTS) -S $(QEMUGDB)


qemu-serv: fs.img xv6.img
	$(QEMU) -serial mon:stdio $(QEMUSERVOPTS)

qemu-nox-serv: fs.img xv6.img
	$(QEMU) -nographic $(QEMUSERVOPTS)

qemu-gdb-serv: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUSERVOPTS) -S $(QEMUGDB)

qemu-nox-gdb-serv: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUSERVOPTS) -S $(QEMUGDB)


qemu-1: fs.img xv6.img
	$(QEMU) -serial mon:stdio $(QEMUCLIENT1OPTS)
	
qemu-nox-1: fs.img xv6.img
	$(QEMU) -nographic $(QEMUCLIENT1OPTS)
	
	
qemu-2: fs.img xv6.img
	$(QEMU) -serial mon:stdio $(QEMUCLIENT2OPTS)
	
qemu-nox-2: fs.img xv6.img
	$(QEMU) -nographic $(QEMUCLIENT2OPTS)


server: server.c 
	gcc -g -Wall $< -o xv6ChatServer



# CUT HERE
# prepare dist for students
# after running make dist, probably want to
# rename it to rev0 or rev1 or so on and then
# check in that version.

EXTRA=\
	arptest.c arptestreceive.c mkfs.c ulib.c user.h cat.c echo.c grep.c kill.c\
	ln.c ls.c mkdir.c rm.c wc.c zombie.c pkill.c ping.c\
	printf.c umalloc.c printfmt.c\
	README dot-bochsrc *.pl toc.* runoff runoff1 runoff.list\
	.gdbinit.tmpl gdbutil\

handin-check:
	@if ! test -d .git; then \
		echo No .git directory, is this a git repository?; \
		false; \
	fi
	@if test "$$(git symbolic-ref HEAD)" != refs/heads/lab$(LAB); then \
		git branch; \
		read -p "You are not on the lab$(LAB) branch.  Hand-in the current branch? [y/N] " r; \
		test "$$r" = y; \
	fi
	@if ! git diff-files --quiet || ! git diff-index --quiet --cached HEAD; then \
		git status; \
		echo; \
		echo "You have uncomitted changes.  Please commit or stash them."; \
		false; \
	fi
	@if test -n "`git ls-files -o --exclude-standard`"; then \
		git status; \
		read -p "Untracked files will not be handed in.  Continue? [y/N] " r; \
		test "$$r" = y; \
	fi



tarball: handin-check
	git archive --format=tar HEAD | gzip > lab$(LAB)-handin.tar.gz



dist:
	rm -rf dist
	mkdir dist
	for i in $(FILES); \
	do \
		grep -v PAGEBREAK $$i >dist/$$i; \
	done
	sed '/CUT HERE/,$$d' Makefile >dist/Makefile
	echo >dist/runoff.spec
	cp $(EXTRA) dist

dist-test:
	rm -rf dist
	make dist
	rm -rf dist-test
	mkdir dist-test
	cp dist/* dist-test
	cd dist-test; $(MAKE) print
	cd dist-test; $(MAKE) bochs || true
	cd dist-test; $(MAKE) qemu

# update this rule (change rev#) when it is time to
# make a new revision.
tar:
	rm -rf /tmp/xv6
	mkdir -p /tmp/xv6
	cp dist/* dist/.gdbinit.tmpl /tmp/xv6
	(cd /tmp; tar cf - xv6) | gzip >xv6-rev9.tar.gz  # the next one will be 9 (6/27/15)

.PHONY: dist-test dist

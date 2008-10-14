EXECS = m68k/gpiotrace.m68k arm/gpiotrace.arm

all: gpiotrace.prc

install: gpiotrace.prc
	pilot-xfer -p /dev/tts/USB0 -i gpiotrace.prc

gpiotrace.prc: arm-objs m68k-objs gui 
	build-prc -n WhineHack -c WhHk $(EXECS) *.bin

arm-objs:
	make -C arm

m68k-objs:
	make -C m68k

gui: include/gpiotrace_r.h
	pilrc -q -I include include/gpiotrace.rcp 
clean:
	rm -f *.prc *.map *~ *.bin 
	make -C arm clean
	make -C m68k clean

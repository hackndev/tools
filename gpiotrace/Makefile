EXECS = m68k/gpiotrace.m68k arm/gpiotrace.arm

all: gpiotrace.prc uhctrace.prc cs2trace.prc pmtrace.prc udctrace.prc

install: gpiotrace.prc
	pilot-xfer -p /dev/tts/USB0 -i gpiotrace.prc

gpiotrace.prc: arm-objs m68k-objs gui 
	build-prc -n gpiotrace -c WhHk -o gpiotrace.prc $(EXECS) *.bin

uhctrace.prc: arm-objs-uhc m68k-objs-uhc gui 
	build-prc -n uhctrace -c WhHk -o uhctrace.prc $(EXECS) *.bin

cs2trace.prc: arm-objs-cs2 m68k-objs-cs2 gui
	build-prc -n cs2trace -c WhHk -o cs2trace.prc $(EXECS) *.bin

pmtrace.prc: arm-objs-pm m68k-objs-pm gui
	build-prc -n pmtrace -c WhHk -o pmtrace.prc $(EXECS) *.bin

udctrace.prc: arm-objs-udc m68k-objs-udc gui
	build-prc -n udctrace -c WhHk -o udctrace.prc $(EXECS) *.bin

arm-objs:
	make -BC arm

m68k-objs:
	make -BC m68k

arm-objs-uhc:
	make -BC arm DEFINES=-DUHC

m68k-objs-uhc:
	make -BC m68k DEFINES=-DUHC

arm-objs-cs2:
	make -BC arm DEFINES=-DCHIP_SELECT2

m68k-objs-cs2:
	make -BC m68k DEFINES=-DCHIP_SELECT2

arm-objs-pm:
	make -BC arm DEFINES=-DPM

m68k-objs-pm:
	make -BC m68k DEFINES=-DPM

arm-objs-udc:
	make -BC arm DEFINES=-DUDC

m68k-objs-udc:
	make -BC m68k DEFINES=-DUDC


gui: include/gpiotrace_r.h
	pilrc -q -I include include/gpiotrace.rcp 
clean:
	rm -f *.prc *.map *~ *.bin 
	make -C arm clean
	make -C m68k clean

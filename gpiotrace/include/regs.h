
#ifndef _REGS_H_
#define _REGS_H_

#define GPLR0		0x90e00000
#define GPLR1		0x90e00004
#define GPLR2		0x90e00008
#define GPLR3		0x90e00100

#define GPDR0		0x90e0000c
#define GPDR1		0x90e00010
#define GPDR2		0x90e00014
#define GPDR3		0x90e0010c

#define GAFR_L0         0x90e00054
#define GAFR_L1         0x90e0005c
#define GAFR_L2         0x90e00064
#define GAFR_L3         0x90e00154

#define GAFR_U0         0x90e00058
#define GAFR_U1         0x90e00060
#define GAFR_U2         0x90e00068
#define GAFR_U3         0x90e00158

#define RCNR		0x90900000

#ifdef CHIP_SELECT2
#define REGISTER	0x08000000
#define SIZE		( 0xac / 4 )
#endif

#ifdef UHC
#define REGISTER	0x9c000000
#define SIZE		( 0x6c / 4 )	
#endif

#ifdef UDC
#define REGISTER	0x90600000
#define SIZE		( 0x045c / 4 )	
#endif

#ifdef PM
#define REGISTER	0x90f00000
#define SIZE		( 0xfc / 4 )	
#endif



#endif

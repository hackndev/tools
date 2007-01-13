# GPIO deocding and formatting tool for HH.org wiki
#
# bobofdooom@gmail.com
#

# Number of gpios, 120 on PXA27x
gpio_count = 120

# GPIO direction registers
#GPDR = [0xC3EF001CL, 0xFF22AB53L, 0x9B1CFFFFL, 0x01F7E785L]
GPDR = [0xcf9905fc, 0xff623ba1, 0x93ccffff, 0x007dff81]

# GPIO alternate function registers (go 0_L, 0_U, 1_L, 1_U ...)
GAFR = [0x0e0000000L, 0xA500001aL, 0x00088052L, 0xaaa04959L, 0x22AAAAAAL, 0x45440f08L,
		0x56aa95ff, 0x00001401L]

# GPIO level registers
GPLR = [0x051BA5FFL, 0x65FD5FBCL, 0x42FB2D93L, 0x0066138DL]

# GPIO rising edge detect
GRER = [0x0020c800L, 0L, 0x04000000L, 0x00000000L]

# GPIO falling edge detect
GFER = [0x00208000L, 0x00000000L, 0L, 0x00000000L]

# End of data

DIR = ['In', 'Out']


def get_bit(bit, reg):
	i = bit / 32
	o = bit % 32
	if reg[i] & (1L<<o): return 1
	return 0

def print_gpio(n):
	# decode
	direc = DIR[ get_bit(n, GPDR) ]
	level = str( get_bit(n, GPLR) )
	af = str( get_bit(n*2, GAFR) + (get_bit(n*2+1, GAFR)<<1) )
	ed = ''
	if get_bit(n, GRER): ed += 'RE '
	if get_bit(n, GFER): ed += 'FE '
	if not ed: ed = ' '
	
	comment = AF_NAMES.get((n, direc, int(af)), ' ')
	
	# print
	print "||'''" +  str(n) + "'''||" + direc + "||" + level + "||" + af + "||" +  ed + "||" + comment + "||"

def main():
	print "== GPIO configuration =="
	print
	print "||'''GPIO#'''||'''Direction'''||'''State'''||'''AFR'''||'''Interrupt'''||'''Function'''||"
	map(print_gpio, range(gpio_count))
	print """
'''Legend:'''

 * Direction: In - input, Out - output, In/Out - bidirectional
 * State: pin level, GPLR
 * AFR (Alternate Function Number): 0-3, GAFR
 * Interrupt (status): Rising Edge (RE), Falling Edge (FE)
 * Function - description"""
	

# note this list doesn't contain all pxa270 alternate functions
AF_NAMES = {
(  1, "In", 1): "RTS",
(  6, "Out", 1): "MMCCLK",
(  7, "Out", 1): "48MHz",
(  8, "Out", 1): "MMCCS0",
(  9, "Out", 1): "MMCCS1",
( 10, "Out", 1): "RTCCLK",
( 11, "Out", 1): "3_6MHz",
( 12, "Out", 1): "32KHz",
( 13, "Out", 2): "MBGNT",
( 14, "In", 1): "MBREQ",
( 15, "Out", 2): "nCS_1",
( 16, "Out", 2): "PWM0",
( 17, "Out", 2): "PWM1",
( 17, "In", 2): "CIF_DD<6>",
( 18, "In", 1): "RDY",
( 19, "In", 1): "DREQ1",
( 20, "In", 1): "DREQ0",
( 23, "Out", 2): "SCLK",
( 24, "Out", 2): "SFRM",
( 25, "Out", 2): "STXD",
( 26, "In", 1): "SRXD",
( 27, "In", 1): "SEXTCLK",
( 28, "In", 1): "BITCLK_AC97",
( 28, "In", 2): "BITCLK_In_I2S",
( 28, "Out", 1): "BITCLK_Out_I2S",
( 29, "In", 1): "SDATA_In_AC97",
( 29, "In", 2): "SDATA_In_I2S",
( 30, "Out", 2): "SDATA_Out_AC97",
( 30, "Out", 1): "SDATA_Out_I2S",
( 31, "Out", 1): "SYNC_I2S",
( 31, "Out", 2): "SYNC_AC97",
( 32, "In", 1): "SDATA_In1_AC97",
( 32, "Out", 1): "SYSCLK_I2S",
( 32, "Out", 2): "MMCCLK",
( 33, "Out", 2): "nCS_5",
( 34, "In", 1): "FFRXD",
( 34, "Out", 2): "MMCCS0",
( 35, "In", 1): "FFCTS",
( 36, "In", 1): "FFDCD",
( 37, "In", 1): "FFDSR",
( 38, "In", 1): "FFRI",
( 39, "Out", 1): "MMCCS1",
( 39, "Out", 2): "FFTXD",
( 40, "Out", 2): "FFDTR",
( 41, "Out", 2): "FFRTS",
( 42, "In", 1): "BTRXD",
( 42, "In", 3): "HWRXD",
( 43, "Out", 2): "BTTXD",
( 43, "Out", 3): "HWTXD",
( 44, "In", 1): "BTCTS",
( 44, "In", 3): "HWCTS",
( 45, "Out", 2): "BTRTS",
( 45, "Out", 3): "HWRTS",
( 45, "Out", 1): "SYSCLK_AC97",
( 46, "In", 1): "ICPRXD",
( 46, "In", 2): "STRXD",
( 47, "Out", 2): "ICPTXD",
( 47, "Out", 1): "STTXD",
( 48, "In", 1): "CIF_DD<5>",
( 48, "Out", 2): "nPOE",
( 48, "Out", 1): "HWTXD",
( 49, "Out", 2): "nPWE",
( 49, "In", 1): "HWRXD",
( 50, "Out", 2): "nPIOR",
( 50, "In", 1): "DD_CIF<3>", #PXA250: "HWCTS",
( 51, "In", 1): "DD_CIF<2>",
( 51, "Out", 2): "nPIOW",
( 51, "Out", 1): "HWRTS",
( 52, "In", 1): "DD_CIF<4>",
( 52, "Out", 2): "nPCE_1",
( 53, "Out", 2): "nPCE_2",
( 53, "Out", 1): "MMCCLK",
( 54, "Out", 1): "MMCCLK",
( 54, "Out", 2): "nPCE_2",
( 54, "Out", 2): "pSKTSEL",
( 55, "Out", 2): "nPREG",
( 55, "In", 1): "DD_CIF<1>",
( 56, "In", 1): "nPWAIT",
( 57, "In", 1): "nIOIS16",
( 58, "Out", 2): "LDD_0",
( 59, "Out", 2): "LDD_1",
( 60, "Out", 2): "LDD_2",
( 61, "Out", 2): "LDD_3",
( 62, "Out", 2): "LDD_4",
( 63, "Out", 2): "LDD_5",
( 64, "Out", 2): "LDD_6",
( 65, "Out", 2): "LDD_7",
( 66, "Out", 2): "LDD_8",
( 66, "In", 1): "MBREQ",
( 67, "Out", 2): "LDD_9",
( 67, "Out", 1): "MMCCS0",
( 68, "Out", 2): "LDD_10",
( 68, "Out", 1): "MMCCS1",
( 69, "Out", 2): "LDD_11",
( 69, "Out", 1): "MMCCLK",
( 70, "Out", 2): "LDD_12",
( 70, "Out", 1): "RTCCLK",
( 71, "Out", 2): "LDD_13",
( 71, "Out", 1): "3_6MHz",
( 72, "Out", 2): "LDD_14",
( 72, "Out", 1): "32kHz",
( 73, "Out", 2): "LDD_15",
( 73, "Out", 1): "MBGNT",
( 74, "Out", 2): "LCD_FCLK",
( 75, "Out", 2): "LCD_LCLK",
( 76, "Out", 2): "LCD_PCLK",
( 77, "Out", 2): "LCD_ACBIAS",
( 78, "Out", 2): "nCS_2",
( 79, "Out", 2): "nCS_3",
( 79, "Out", 1): "pSKTSEL",
( 80, "Out", 2): "nCS_4",
( 81, "Out", 1): "NSSP_CLK_Out",
( 81, "In", 1): "NSSP_CLK_In",
( 81, "In", 2): "DD_CIF<0>",
( 82, "Out", 1): "NSSP_FRM_Out",
( 82, "In", 1): "NSSP_FRM_In",
( 83, "Out", 1): "NSSP_TX",
( 83, "In", 2): "NSSP_RX",
( 84, "Out", 1): "NSSP_TX",
( 84, "In", 3): "CIF_FV",
( 84, "In", 2): "NSSP_RX",
( 85, "In", 3): "CIF_LV",
( 85, "Out", 1): "nPCE_1",
( 89, "Out", 1): "AC97_SYSCLK",
( 91, "In", 1): "KP_MKIN<6>",
( 92, "Out", 1): "MMCDAT0",
( 93, "In", 1): "KP_MKIN<0>",
(104, "Out", 1): "pSKTSEL",
(109, "Out", 1): "MMCDAT1",
(110, "Out", 1): "MMCDAT2",
(110, "Out", 1): "MMCCS0",
(111, "Out", 1): "MMCDAT3",
(110, "Out", 1): "MMCCS1",
(112, "Out", 1): "MMCCMD",
(113, "Out", 1): "I2S_SYSCLK",
(113, "Out", 2): "AC97_RESET_N",
(117, "Out", 1): "I2CSCL",
(118, "In", 1): "I2CSDA",
(118, "Out", 1): "I2CSDA",
}

if __name__ == '__main__': main()

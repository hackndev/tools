#!/usr/bin/env python
import sys

if len(sys.argv) < 5:
	print 'Usage:', sys.argv[0], 'lccr0 lccr1 lccr2 lccr3'
	print '   eg:', sys.argv[0], '0x062000E1 0x1207113F 0x0705053F 0x04700007'
	sys.exit(1)

LCCR0, LCCR1, LCCR2, LCCR3 = [long(x, 0) for x in sys.argv[1:5]]


#LCCR0 = 0x062000E1
#LCCR1 = 0x1207113F
#LCCR2 = 0x0705053F
#LCCR3 = 0x04700007

fields0 = [
  ('ENB', 'LCD Controller Enabled', 1),
  ('CMS', 'Color/Monochrome Select (0=color)', 1),
  ('SDS', 'Single-Scan/dual scan display seelect (0=single)', 1),
  ('LDM', 'LCD Disable Done Mask (0=generate interrupt)', 1),
  ('SOFM0', 'Start of Frame Mask (0=generate interrupt)', 1),
  ('IUM', 'Interrupt FIFO underrun mask', 1),
  ('EOFM0', 'End of Frame Mask', 1),
  ('PAS', 'Passive/active display select (0=passive)', 1),
  ('reserved', '', 1),
  ('DPD', 'Double-Pixel Data pin mode (0=4 pixel/pixclock, 1=8 pixel)', 1),
  ('DIS', 'LCD disable', 1),
  ('QDM', 'Quick Disable Mask', 1),
  ('PDD', 'Paletter DMA delay', 7),
  ('BSM0', 'Branch status mask', 1),
  ('OUM', 'Output FIFO underrun mask', 1),
  ('LCDT', 'LCD type (1=internal frame buffer)', 1),
  ('RDSTM', 'LCD Read Status interrupt mask', 1),
  ('CMDIM', 'LCD Command interrupt mask', 1),
  ('OUC', 'Overlay Underlay Control bit', 1),
  ('LDDALT', 'LCDD Alternate map', 1),
  ]

fields1 = [
  ('PPL', 'pixels per line (minus one)', 10),
  ('HSW', 'Horizontal sync pulse width (minus one)', 6),
  ('ELW', 'End of Line pixel clock wait count (minus one)', 8),
  ('BLW', 'Beginning of Line pixel clock wait count (minus one)', 8),
]

fields2 = [
  ('LPP', 'lines per panel (minus one)', 10),
  ('VSW', 'Vertical sync pulse width (minus one)', 6),
  ('EFW', 'End of frame wait count (minus one)', 8),
  ('BFW', 'Beginning of frame wait count (minus one)', 8),
]

fields3 = [
  ('PCD', 'pixel clock divisor', 8),
  ('ACB', 'AC bias pin frequency', 8),
  ('API', 'AC bias pin transitions per interrupt', 4),
  ('VSP', 'vertical sync polarity', 1),
  ('HSP', 'horizontal sync polarity', 1),
  ('PCP', 'pixel clock polarity', 1),
  ('OEP', 'output enable polarity', 1),
  ('DPC', 'double pixel clock', 1),
  ('reserved','', 1),
  ('BPP', 'bits per pixel', 3),
  ('PDFOR', 'pixel data format', 2),
]

m = {}

def decode(fields, reg):
	for name, desc, size in fields:
		val = reg & ((1<<size)-1)
		if name != 'reserved':
			print name + ':\t', val, '\t' + desc + ''
			m[name] = val

		reg >>= size
	print

decode(fields0, LCCR0)
decode(fields1, LCCR1)
decode(fields2, LCCR2)
decode(fields3, LCCR3)

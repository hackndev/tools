/*
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <PalmOS.h>
#include <PceNativeCall.h>
#include <Standalone.h>
#include "regs.h"
#include "gpiotrace.h"

STANDALONE_CODE_RESOURCE_ID(0);

/* Disable IRQ and FIQ */
#define irq_off() asm volatile ("mrs r0, cpsr \n" \
                    "orr r0, r0, #0xc0 \n" \
		    "msr cpsr, r0" : : : "r0" )

/* Enable IRQ and FIQ */
#define irq_on() asm volatile ("mrs r0, cpsr \n" \
                    "and r0, r0, #0xffffff3f \n" \
		    "msr cpsr, r0" : : : "r0" )


#define CPU_VENDOR_MASK 0xff000000
#define CPU_MODEL_MASK 0xff000fff
#define CPUV_INTEL 0x69000000
#define CPU_SA1100 0x69000B11 /* StrongArm */

/* Note: This function needs to be kept at the top of the file */
unsigned long arm_entry(const void *emulStateP, UInt32 reg/*pref*/,
			Call68KFuncType * call68KFuncP)
{
/*	UInt32 cpu_id = 0;
	UInt32 divisor = pref & DIVISOR_MASK;
*///	UInt32 *reg;

//	reg = (UInt32*)reg;
	return *(UInt32*)reg;

//	return ~pref;
}
/*
asm("HALDisplaySleep:\n"
        "ldr     ip, [r9, #-4]\n"
	"ldr     pc, [ip, #856]\n");
		
*/
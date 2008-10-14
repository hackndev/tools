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
#include <MemGlue.h>
#include <MemoryMgr.h>
#include <stdio.h>

#include "gpiotrace.h"
#include "gpiotrace_r.h"
#include "mainform.h"
#include "regs.h"
#include "VFSMgr.h"


FormPtr mainform;

/**
 * Call an ARM side function
 */
UInt32 call_arm(UInt32 addr)
{
	MemHandle arm_code_handle;
	void *arm_code = NULL;
	UInt32 ret = 3;

	arm_code_handle = DmGetResource('armc', 0);
	if (!arm_code_handle) {
		return 1;
	}

	arm_code = MemHandleLock(arm_code_handle);
	if (!arm_code_handle) {
		return 2;
	}

	ret = PceNativeCall(arm_code, (void*)addr);
	MemHandleUnlock(arm_code_handle);

	return ret;
}

void event_loop()
{
	EventType event;
	UInt16 err;
	FormPtr form;
	Int16 form_id;

	do {
		EvtGetEvent(&event, 200);

		if (SysHandleEvent(&event))
			continue;
		if (MenuHandleEvent(NULL, &event, &err))
			continue;

		if (event.eType == frmLoadEvent) {
			form_id = event.data.frmLoad.formID;
			mainform = form = FrmInitForm(form_id);
			FrmSetActiveForm(form);

			if (form_id == MainForm) {
				FrmSetEventHandler(form, mainform_event);
			}
		}
		if (event.eType == frmOpenEvent) {
			FrmDrawForm(FrmGetActiveForm());
		}

		FrmDispatchEvent(&event);
	} while (event.eType != appStopEvent);
}

UInt16 start_app()
{
	FrmGotoForm(MainForm);

	return 0;
}

void stop_app()
{
}

void disable_hack()
{
	UInt16  card;
	LocalID db_id;
	SysCurAppDatabase(&card, &db_id);

	SysNotifyUnregister(card, db_id, sysNotifyIdleTimeEvent, sysNotifyNormalPriority);
}

void enable_hack()
{
	UInt16 err=0, card;
	LocalID db_id;

	SysCurAppDatabase(&card, &db_id);


	err |= SysNotifyRegister(card, db_id, sysNotifyLateWakeupEvent, NULL,
				sysNotifyNormalPriority, NULL);
	err |= SysNotifyRegister(card, db_id, sysNotifyDisplayChangeEvent, NULL,
				sysNotifyNormalPriority, NULL);
	err |= SysNotifyRegister(card, db_id, sysNotifyAppLaunchingEvent, NULL,
				sysNotifyNormalPriority, NULL);
	err |= SysNotifyRegister(card, db_id, sysNotifyAppQuittingEvent, NULL,
				sysNotifyNormalPriority, NULL);
	err |= SysNotifyRegister(card, db_id, sysNotifyIdleTimeEvent, NULL,
				sysNotifyNormalPriority, NULL);
	FrmCustomAlert(InfoAlert, "GPIOhack",
	err ? "SysNotifyRegister failed." : "Hack registered",
	err ? ":-(" : ":-)");

	if (err) {
		err = 0;
		err |= SysNotifyUnregister(card, db_id, sysNotifyLateWakeupEvent, 
					sysNotifyNormalPriority);
		err |= SysNotifyUnregister(card, db_id, sysNotifyDisplayChangeEvent, 
					sysNotifyNormalPriority);
		err |= SysNotifyUnregister(card, db_id, sysNotifyAppLaunchingEvent, 
					sysNotifyNormalPriority);
		err |= SysNotifyUnregister(card, db_id, sysNotifyAppQuittingEvent, 
					sysNotifyNormalPriority);
		err |= SysNotifyUnregister(card, db_id, sysNotifyIdleTimeEvent, 
					sysNotifyNormalPriority);
		FrmCustomAlert(InfoAlert, "GPIOhack",
		"Hack disabled",
		err ? ":-(" : ":-)");

	}


}

void append_data(
		UInt32 gplr_ts, UInt32 gplr0, UInt32 gplr1, UInt32 gplr2, UInt32 gplr3, UInt32 gplr_te,
		UInt32 gpdr_ts, UInt32 gpdr0, UInt32 gpdr1, UInt32 gpdr2, UInt32 gpdr3,UInt32 gpdr_te,
		UInt32 gafr_l_ts, UInt32 gafr_l_0, UInt32 gafr_l_1, UInt32 gafr_l_2, UInt32 gafr_l_3, UInt32 gafr_l_te,
		UInt32 gafr_u_ts, UInt32 gafr_u_0, UInt32 gafr_u_1, UInt32 gafr_u_2, UInt32 gafr_u_3, UInt32 gafr_u_te) {
	UInt16 volRefNum;
	UInt32 volIterator=vfsIteratorStart;
	FileRef fRef;
	UInt32 rBytes;
	char buf[1023];
	UInt16  err;

	while ((volIterator != vfsIteratorStop)) {
		if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone) {

	if (VFSFileOpen(volRefNum,"/GPIO.txt", vfsModeWrite,&fRef) == errNone) {
	VFSFileSeek(fRef,vfsOriginBeginning,0);

/*	StrPrintF(buf,"GPLR%i --> 0x%x%x [0x%x%x]\n",gplr_nr,
		(changed&0xffff0000)>>16,changed&0xffff,(val&0xffff0000)>>16,val&0xffff);
*/

	sprintf(buf,	"GPLR [0x%08lx] 0x%08lx 0x%08lx 0x%08lx 0x%08lx [0x%08lx] || "
			"GPDR [0x%08lx] 0x%08lx 0x%08lx 0x%08lx 0x%08lx [0x%08lx] || "
			"GAFR_L [0x%08lx] 0x%08lx 0x%08lx 0x%08lx 0x%08lx [0x%08lx] || "
			"GAFR_U [0x%08lx] 0x%08lx 0x%08lx 0x%08lx 0x%08lx [0x%08lx]\n",
		gplr_ts, gplr0, gplr1, gplr2, gplr3, gplr_te,
		gpdr_ts, gpdr0, gpdr1, gpdr2, gpdr3, gpdr_te,
		gafr_l_ts, gafr_l_0, gafr_l_1, gafr_l_2, gafr_l_3, gafr_l_te,
		gafr_u_ts, gafr_u_0, gafr_u_1, gafr_u_2, gafr_u_3, gafr_u_te);

	err = VFSFileSeek(fRef, vfsOriginEnd, 0);
	err = VFSFileWrite (fRef, StrLen(buf), (const void *)buf, &rBytes);
	VFSFileClose(fRef);

			}
		}
	}
}

UInt32 PilotMain(UInt16 launch_code, MemPtr cmd_PBP, UInt16 launch_flags)
{
//	UInt32 GPLR0_p = 0, GPLR1_p = 0, GPLR2_p = 0;
	UInt32 GPLR0_a[10] = {0}, GPLR1_a[10] = {0}, GPLR2_a[10] = {0}, GPLR3_a[10] = {0}, GPLR_ts[10] = {0}, GPLR_te[10] = {0};
	UInt32 GPDR0_a[10] = {0}, GPDR1_a[10] = {0}, GPDR2_a[10] = {0}, GPDR3_a[10] = {0}, GPDR_ts[10] = {0}, GPDR_te[10] = {0};
	UInt32 GAFR_L0_a[10] = {0}, GAFR_L1_a[10] = {0}, GAFR_L2_a[10] = {0}, GAFR_L3_a[10] = {0}, GAFR_L_ts[10] = {0}, GAFR_L_te[10] = {0};
	UInt32 GAFR_U0_a[10] = {0}, GAFR_U1_a[10] = {0}, GAFR_U2_a[10] = {0}, GAFR_U3_a[10] = {0}, GAFR_U_ts[10] = {0}, GAFR_U_te[10] = {0};

//	UInt16 size;
	UInt16 err;
	int i ;
//	char buf[255];

	UInt16 volRefNum;
	UInt32 volIterator=vfsIteratorStart;

	if (launch_code == sysAppLaunchCmdNormalLaunch) {
		err = start_app();
		if (err) {
			return err;
		}

/*		if (PrefGetAppPreferences ('WhHk', 1, &GPLR0_a, &size, true) == noPreferenceFound)
			PrefSetAppPreferences ('WhHk', 1, 0, &GPLR0_a, sizeof(UInt32), true);
*/
		while ((volIterator != vfsIteratorStop)) {
			if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone) {
				VFSFileCreate(volRefNum,"/GPIO.txt");
			}
		}

		enable_hack();
//		event_loop();

		stop_app();
//	} else if (launch_code == sysAppLaunchCmdSystemReset || 
//			launch_code == sysAppLaunchCmdSyncNotify) {
//		enable_hack();
	} else if (launch_code == sysAppLaunchCmdNotify) {
//		PrefGetAppPreferences ('WhHk', 1, &GPLR0_p, &size, true);
for (i=0;i<10;i++) {
		GPLR_ts[i] = call_arm(RCNR);
		GPLR0_a[i] = call_arm(GPLR0);
		GPLR1_a[i] = call_arm(GPLR1);
		GPLR2_a[i] = call_arm(GPLR2);
		GPLR3_a[i] = call_arm(GPLR3);
		GPLR_te[i] = call_arm(RCNR);

		GPDR_ts[i] = call_arm(RCNR);
		GPDR0_a[i] = call_arm(GPDR0);
		GPDR1_a[i] = call_arm(GPDR1);
		GPDR2_a[i] = call_arm(GPDR2);
		GPDR3_a[i] = call_arm(GPDR3);
		GPDR_te[i] = call_arm(RCNR);

		GAFR_L_ts[i] = call_arm(RCNR);
		GAFR_L0_a[i] = call_arm(GAFR_L0);
		GAFR_L1_a[i] = call_arm(GAFR_L1);
		GAFR_L2_a[i] = call_arm(GAFR_L2);
		GAFR_L3_a[i] = call_arm(GAFR_L3);
		GAFR_L_te[i] = call_arm(RCNR);

		GAFR_U_ts[i] = call_arm(RCNR);
		GAFR_U0_a[i] = call_arm(GAFR_U0);
		GAFR_U1_a[i] = call_arm(GAFR_U1);
		GAFR_U2_a[i] = call_arm(GAFR_U2);
		GAFR_U3_a[i] = call_arm(GAFR_U3);
		GAFR_U_te[i] = call_arm(RCNR);
}
for (i=0;i<10;i++) {
		append_data(
			GPLR_ts[i], GPLR0_a[i], GPLR1_a[i], GPLR2_a[i], GPLR3_a[i], GPLR_te[i],
			GPDR_ts[i], GPDR0_a[i], GPDR1_a[i], GPDR2_a[i], GPDR3_a[i], GPDR_te[i],
			GAFR_L_ts[i], GAFR_L0_a[i], GAFR_L1_a[i], GAFR_L2_a[i], GAFR_L3_a[i], GAFR_L_te[i],
			GAFR_U_ts[i], GAFR_U0_a[i], GAFR_U1_a[i], GAFR_U2_a[i], GAFR_U3_a[i], GAFR_U_te[i]
		);
}
//		if (GPLR0_p ^ GPLR0_a) {

//			sprintf(buf, "  TFC: 0x%08lx\n", GPLR0_a ^ GPLR0_p);
//			FrmCustomAlert(InfoAlert, "GPIOhack", buf, " ");
//		}


//		PrefSetAppPreferences ('WhHk', 1, 0, &GPLR0_a, sizeof(UInt32), true);

	}
	return 0;
}

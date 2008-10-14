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
#include "gpiotrace.h"
#include "gpiotrace_r.h"
#include "mainform.h"
#include "regs.h"
#include <stdio.h>
#include <DataMgr.h>

UInt32 reg(UInt32 addr);

Boolean mainform_menu_event(Int16 id)
{
	return true;
}



Boolean mainform_event(EventPtr event)
{
	Boolean handled = false;
	FormPtr form = NULL;

	return handled;
}

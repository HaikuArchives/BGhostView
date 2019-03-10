/****************************************************************************
**
** Name       : BPSConsole
** Author     : Andreas Raquet
** Copyright  : (C) 12/10/1998 by Andreas Raquet
** Description: Console-window for BPSWidget 
**				Allows to directly communicate with GS
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/
#include "BPSConsole.h"


BPSConsole::BPSConsole(BRect frame, const char *name)
	:BWindow(frame, name, B_DOCUMENT_WINDOW, 0) {
	output = new BTextView(BRect(0,0,frame.Width()-B_V_SCROLL_BAR_WIDTH,frame.Height()-B_H_SCROLL_BAR_HEIGHT),"GS Output", BRect(0,0,frame.Width()-B_V_SCROLL_BAR_WIDTH,frame.Height()-B_H_SCROLL_BAR_HEIGHT),B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM,B_WILL_DRAW);
    AddChild(new BScrollView("ScrollOuput",output, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM, B_FULL_UPDATE_ON_RESIZE, true, true));
	};

void BPSConsole::toggle() {
	Lock();
	if (IsHidden()) Show();
	else Hide();
	Unlock();
}

bool BPSConsole::QuitRequested() {
	Hide();
	return false;
};

void BPSConsole::addText(const char* text) {
	Lock();
	output->Insert(output->TextLength(), text, strlen(text));
	Unlock();
	}


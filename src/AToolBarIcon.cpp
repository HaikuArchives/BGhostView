/****************************************************************************
**
** Name       : AToolBarIcon
** Author     : Andreas Raquet
** Copyright  : (C) 8/23/1998 by Andreas Raquet
** Description: A generic ToolBar Icon
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/
#include "AToolBarIcon.h"
#include <stdlib.h>

AToolBarIcon::AToolBarIcon( BRect frame, BBitmap *bitmap, const char *name, BMessage *message ) 
  : BButton(frame,name,NULL,message,B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW) {
	SetViewColor(B_TRANSPARENT_COLOR);
	selectable=false;
	control=false;
	icon=bitmap;
  if (icon==NULL) {
    BAlert *d = new BAlert("Error","Icon not found\n","Ok");
    d->Go();
    exit(1);
  }
};

void AToolBarIcon::GetPreferredSize(float *width, float *height) {
	*width=icon->Bounds().Width()+2;
	*height=icon->Bounds().Height()+2;};

void AToolBarIcon::MouseMoved(BPoint point, uint32 transit, const BMessage *message) {
	switch (transit) {
	case B_ENTERED_VIEW: 
		selectable=true; 
		Invalidate();
		break;
	case B_EXITED_VIEW: 
		selectable=false;
		Invalidate();
		break;
	};
};

void AToolBarIcon::SetEnabled(bool enabled) {
	BButton::SetEnabled(enabled);
	Invalidate(); };

void AToolBarIcon::SetValue(int32 value) {
	switch (value) {
	case 1: control=true; selectable=true; break;
	case 0: control=false; 
		BPoint pt; uint32 btns;
		GetMouse(&pt, &btns);
		selectable=Bounds().Contains(pt);  break;};
	Invalidate();
	BButton::SetValue(value);
};

void AToolBarIcon::Draw(BRect frame) {
		rgb_color BeShadow=tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DISABLED_LABEL_TINT);
	if (selectable && IsEnabled()) {
		if (!control) SetHighColor(White);
		else SetHighColor(BeShadow);
		StrokeLine(BPoint(0,Bounds().bottom),BPoint(0,0));
		StrokeLine(BPoint(0,0),BPoint(Bounds().right,0));
		if (!control) SetHighColor(BeShadow);
		else SetHighColor(White);
		StrokeLine(BPoint(1,Bounds().bottom),BPoint(Bounds().right,Bounds().bottom));
		StrokeLine(BPoint(Bounds().right,Bounds().bottom-1),BPoint(Bounds().right,0));}
	if (IsEnabled()) SetDrawingMode(B_OP_OVER);
	else SetDrawingMode(B_OP_BLEND);
	DrawBitmap(icon, BPoint(1,1));
	if (!IsEnabled()) {
		SetHighColor(BeBackgroundGrey);
		FillRect(BRect(0,0,Bounds().Width(),Bounds().Height()));};
};

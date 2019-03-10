/****************************************************************************
**
** Name       : BGVPageItem
** Author     : Andreas Raquet
** Copyright  : (C) 8/23/1998 by Andreas Raquet
** Description: A ListviewItem that displays Pagenumbers and Pagemarks.
**							Maintains a link to the page-structure as delivered
**							by ps.c that can be fed to BPSWidget when the user
**							clicks on the pagemark.
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include "BGVPageItem.h"
#include <Colors.h>

BGVPageItem::BGVPageItem(struct page *ref) {
	referedPage=ref;
	showing=false;
	};
	
void BGVPageItem::DrawItem(BView *owner, BRect frame, bool complete) {
	owner->SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),showing ? B_DARKEN_1_TINT : B_LIGHTEN_2_TINT));
	owner->SetLowColor(owner->HighColor());
	owner->FillRect(frame);
	owner->SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),showing? B_DARKEN_2_TINT: B_LIGHTEN_MAX_TINT));
	owner->StrokeLine(frame.LeftTop(),frame.RightTop());
	owner->SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),showing ? B_LIGHTEN_1_TINT: B_DARKEN_3_TINT));
	owner->StrokeLine(frame.LeftBottom(),frame.RightBottom());
	int height = (int) (frame.bottom-frame.top);
	owner->SetHighColor(Black);
	BPoint points[5];
	int shift = ( showing ? 1 : 0);
	points[0]=BPoint(frame.left+3+shift,frame.top+2+shift);
	points[1]=BPoint(frame.left+height-5+shift,frame.top+2+shift);
	points[2]=BPoint(frame.left+height-5+shift,frame.top+4+shift);
	points[3]=BPoint(frame.left+height-3+shift,frame.top+4+shift);
	points[4]=BPoint(frame.left+height-3+shift,frame.bottom-2+shift);
	points[5]=BPoint(frame.left+3+shift,frame.bottom-2+shift);
	BPolygon *poly = new BPolygon(points,6);
	if (IsSelected())
		owner->FillPolygon(poly);
	else
		owner->StrokePolygon(poly);
	owner->StrokeLine(points[1],points[3]);
	owner->MovePenTo(frame.left+2*height+shift,frame.bottom-2+shift);
	owner->DrawString(referedPage->label);
	delete poly;
	};

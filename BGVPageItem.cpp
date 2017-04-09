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
	int height = frame.bottom-frame.top;
	if (showing) owner->SetHighColor(BeBackgroundGrey);
	else owner->SetHighColor(White);
	owner->FillRect(frame);
	owner->SetHighColor(Black);
	BPoint points[5];
	points[0]=BPoint(frame.left+3,frame.top+2);
	points[1]=BPoint(frame.left+height-5,frame.top+2);
	points[2]=BPoint(frame.left+height-5,frame.top+4);
	points[3]=BPoint(frame.left+height-3,frame.top+4);
	points[4]=BPoint(frame.left+height-3,frame.bottom-2);
	points[5]=BPoint(frame.left+3,frame.bottom-2);
	BPolygon *poly = new BPolygon(points,6);
	if (IsSelected())
		owner->FillPolygon(poly);
	else
		owner->StrokePolygon(poly);
	owner->StrokeLine(points[1],points[3]);
	owner->MovePenTo(frame.left+1.3*height,frame.bottom-2);
	owner->DrawString(referedPage->label);
	delete poly;
	};

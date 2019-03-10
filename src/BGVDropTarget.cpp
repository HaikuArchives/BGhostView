/****************************************************************************
**
** Name       : BGVDropTarget
** Author     : Andreas Raquet
** Copyright  : (C) 12/09/1998 by Andreas Raquet
** Description: A simple BView-extension to allow BGhostview
**				to accept drop-events on its main-window
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/
#include "BGVDropTarget.h"


BGVDropTarget::BGVDropTarget(BGhostview *parent, BRect frame, const char *name, uint32 resizingMode, uint32 flags) 
	:BView(frame, name, resizingMode, flags) {
	bgv=parent;
	};

void BGVDropTarget::MessageReceived(BMessage *message) {
  type_code type;
  entry_ref ref;
  int32 count;
  message->GetInfo("refs",&type, &count);
  if (message->WasDropped() && type == B_REF_TYPE && message->FindRef("refs",count-1,&ref)==B_OK) {
    	BEntry entry(&ref);
    	BPath path;
    	entry.GetPath(&path);
	    bgv->openFile(path.Path());
  	}
  else
    BView::MessageReceived(message);
};


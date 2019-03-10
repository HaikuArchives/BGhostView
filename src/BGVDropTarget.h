#ifndef BGV_DROP_TARGET_H
#define BGV_DROP_TARGET_H

#include <Be.h>
#include "BGhostview.h"

class BGVDropTarget : public BView { 
public:
	BGVDropTarget(BGhostview *parent, BRect frame, const char *name, uint32 resizingMode, uint32 flags);
	void MessageReceived(BMessage *message);
private:
	BGhostview *bgv;
};

#endif
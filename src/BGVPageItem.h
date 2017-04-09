#ifndef BGV_PAGEITEM_H
#define BGV_PAGEITEM_H

#include <Be.h>
extern "C" {
	#include "ps.h"
}

class BGVPageItem : public BListItem {
public:
	BGVPageItem(struct page *ref);
	struct page *referedPage;
	void DrawItem(BView *owner, BRect frame, bool complete);
	bool showing;
	};
	
#endif

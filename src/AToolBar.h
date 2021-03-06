#ifndef ATOOLBAR_H
#define ATOOLBAR_H

#include <Be.h>
#include "AToolBarIcon.h"

class AToolBar : public BView {
public:
	AToolBar( BRect frame, const char *name );
	void Draw(BRect frame);
  void AddIcon(BBitmap *bm, const char *name, BMessage *msg, bool enabled =true);
  void AddView(BView *view);
	void AllAttached();
	BControl *ItemAt(int index) { return (BControl *) ChildAt(index); };
private:
	float rightEdge;
};

#endif

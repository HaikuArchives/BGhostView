#ifndef ATOOLBAR_ICON_H
#define ATOOLBAR_ICON_H

#include <Be.h>

class AToolBarIcon : public BButton {
public:
	AToolBarIcon( BRect frame, BBitmap *bitmap, const char *name, BMessage *message );
	void Draw(BRect frame);
	void SetEnabled(bool enabled);
	void MouseMoved( BPoint point, uint32 transit, const BMessage *message);
	void GetPreferredSize(float *width, float *height);
	virtual void SetValue(int32 value);
private:
	BBitmap *icon;
	bool selectable;
	bool control;
};

#endif

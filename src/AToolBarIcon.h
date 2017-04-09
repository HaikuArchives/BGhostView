// A Generic text dialog

#include <Be.h>
#include <Colors.h>

class AToolBarIcon : public BButton {
public:
	AToolBarIcon( BRect frame, const char *filename, const char *name, BMessage *message );
	void Draw(BRect frame);
	void SetEnabled(bool enabled);
	void MouseMoved( BPoint point, uint32 transit, const BMessage *message);
	void GetPreferredSize(float *width, float *height);
	virtual void SetValue(int32 value);
private:
	const char* filename;
	BBitmap *icon;
	bool selectable;
	bool control;
};



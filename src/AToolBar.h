// A Generic text dialog

#include <Be.h>
#include <Colors.h>
#include "AToolBarIcon.h"

class AToolBar : public BView {
public:
	AToolBar( BRect frame, const char *name );
	void Draw(BRect frame);
  void AddIcon(char *filename, const char *name, BMessage *msg, bool enabled =true);
  void AddView(BView *view);
	void AllAttached();
	BControl *ItemAt(int index) { return (BControl *) ChildAt(index); };
private:
	float rightEdge;
};



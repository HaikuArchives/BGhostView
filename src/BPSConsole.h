#ifndef BPS_CONSOLE_H
#define BPS_CONSOLE_H

#include <Be.h>

class BPSConsole : public BWindow { 
public:
	BPSConsole(BRect frame, const char *name);
	virtual bool QuitRequested();
	void toggle();
	void addText(const char* text, int len);
private:
	BTextView *output;
};

#endif

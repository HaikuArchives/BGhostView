#ifndef BPSWIDGET_H
#define BPSWIDGET_H

#include <stdlib.h>
#include <math.h>
#include <Be.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>

#include "BPSConsole.h"

#if !defined(BUFSIZ)
#define BUFSIZ 512;
#endif

int gsdll_call(int message, char *str, unsigned long count);
long gsloop(void* psview);
bool writePS(FILE *in, FILE *out, long begin, unsigned int len);

class BPSWidget : public BView
{
public:
	BPSWidget(BRect frame, const char* name, const char *tempDir);
	~BPSWidget();
	void Draw(BRect frame);
	void TargetedByScrollView(BScrollView *v);
	void SetPaperSize(float width, float height);
	char *GetPaperSwitch();
	char *GetResolutionSwitch();
	BScrollView *scrollView;
	
	bool sendPS(FILE *fp, long begin,unsigned int len,bool close);
	void disableInterpreter() { stopInterpreter(); };
	void enableInterpreter() { startInterpreter(); };
	void showConsole(); 
	void hideConsole();
	void toggleConsole();
	void printOnConsole(const char *text);
	void quitInterpreter();
	bool isInterpreterRunning();
	FILE* out;
	sem_id shutdown_sem, startup_sem, painter_sem, keepup_sem;
	
  char* filename;
	float xdpi, ydpi;
	char *pagemedia;
		
protected:	
	void startInterpreter();
	void stopInterpreter();
	void interpreterFailed();
	thread_id gs_thread;
	char *gs_call[100];
	int gs_arg;
	char *buf;
	bool disable_start;
	bool running;
	char rswitch[128];
	char pswitch[128];
	BPSConsole* console;
}; 

#endif // BPSWIDGET_H



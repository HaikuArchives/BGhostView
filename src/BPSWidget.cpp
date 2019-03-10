/****************************************************************************
**
** Name       : BPSWidget
** Author     : Andreas Raquet
** Copyright  : GPL
** Description: A BView-extension that uses gs to display postscript-data.
**              Spawns gs_thread to talk to the gs_dll (gs-library)
**							and handles the data-transfer from source-file to
**              temporary-files, which are interpretd by gs.
**
** Based on code in KGhostview Copyright (C) 1997 by Mark Donohoe.
** Based on original work by Tim Theisen.
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include "BPSWidget.h"
#include <string.h>
#include <ctype.h>
#include <unistd.h>

extern "C" { 
void gsdll_draw(unsigned char *device, BView *view, BRect dest, BRect src); 
int gsdll_exit();
int gsdll_execute_cont(char *buf, int len);
int gsdll_execute_begin();
int gsdll_execute_end();
int gsdll_init( int fct(int i,char* c,unsigned long ul), int what, int argc, char** argv);
int gsdll_lock_device(unsigned char *dev, int whatever);
int gsdll_unlock_device(unsigned char *dev, int whatever);
} 


char PAGE_FILE[B_FILE_NAME_LENGTH];
bool displayComplete;
float width,height;

BPSWidget *painter;
unsigned char *bdev = NULL; 
int gsdll_call(int message, char *str, unsigned long count) { 
    switch (message) { 
    case 1:
    		strcpy(str,"\n");
  			painter->SetPaperSize(width,height);
    		release_sem(painter->painter_sem); // signal that interpreter is ready to paint
				acquire_sem(painter->keepup_sem); // wait for signal to quit
				acquire_sem(painter->painter_sem); // interpreter should not paint during shutdown
				displayComplete=true;
        return strlen(str);
				break;
    case 2: 	
	    if (!displayComplete) painter->printOnConsole(str);
				return count;
				break;
    case 3: 
        bdev = (unsigned char *) str; 
				break;
    case 4:
				break;
    case 5: 
				break;
    case 6: 
        width=(float)(count & 0xffff);
        height=(float)((count>>16) & 0xffff);
    	  break;
    case 7: 
				return 0; break;
    default: 
        break; 
    } 
    return 0; 
} 

long gsloop(void* psview) {
  BPSWidget *ps = (BPSWidget *) psview;
	int code; 
	int gs_arg;
	char *gs_call[128];
	gs_arg=0;
	gs_call[gs_arg++] = "gs";
	gs_call[gs_arg++] = "-dQUIET";
	if (ps->pagemedia) {
		printf("using fixed media\n");
		gs_call[gs_arg++] = "-dFIXEDMEDIA";
		gs_call[gs_arg++] = ps->GetPaperSwitch();
	}		
	gs_call[gs_arg++] = ps->GetResolutionSwitch();
	gs_call[gs_arg++] = "-dBATCH";
	if (ps->filename) 
		gs_call[gs_arg++] = ps->filename;
	else
		gs_call[gs_arg++] = PAGE_FILE;
	displayComplete=false;
  code = gsdll_init(gsdll_call,NULL, gs_arg, gs_call); 
  if (code==0) {
  	if (displayComplete) gsdll_exit();
  	else {
   		painter->SetPaperSize(width,height);
    	release_sem(painter->painter_sem);
			acquire_sem(painter->keepup_sem);
			acquire_sem(painter->painter_sem);
    	gsdll_exit();
  	}}
	release_sem(painter->shutdown_sem); // signal that interpreter has shut down
	return 0; // no errors
};     

bool writePS( FILE *in, FILE *out, long begin, unsigned int len){
	int rr;
	char *buffer = (char *) malloc(BUFSIZ);
	fseek(in, begin, SEEK_SET);
	while (len>BUFSIZ) {
		rr = fread(buffer, sizeof(char), BUFSIZ, in);
		fwrite(buffer, sizeof(char), rr, out); 
		len -=rr;};
	rr = fread(buffer, sizeof(char), len, in);
	fwrite(buffer, sizeof(char), rr, out); 
	fflush(out);
	return true;
}    

BPSWidget::BPSWidget( BRect frame, const char *name, const char *tempDir) 
  :BView(frame, name , B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	gs_thread=0;
	buf = (char *)malloc(BUFSIZ);
	xdpi=75.0;
	ydpi=75.0; 
  painter=this;
  bdev=0;
	startup_sem = create_sem(1,"gs_startup_protector");
	shutdown_sem = create_sem(1,"gs_shutdown_protector");
	painter_sem = create_sem(1,"gs_paintlock");
	keepup_sem = create_sem(1,"gs_keeprunning");
	if (tempDir==NULL)
		strcpy(PAGE_FILE,"/boot/home/tmp/bgv.tmp");	
	else 
		sprintf(PAGE_FILE,"%s/%s",tempDir,"bgv.tmp");
  out = fopen(PAGE_FILE,"w");
	acquire_sem(keepup_sem);
	acquire_sem(shutdown_sem);
	acquire_sem(painter_sem);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	console=new BPSConsole(BRect(20,40,400,300),"GS Console");
	console->Show();
	console->Hide();
}
 	
void BPSWidget::showConsole() {	
	console->Show();
} 

void BPSWidget::toggleConsole() {
	console->toggle();
}

void BPSWidget::hideConsole() {
 	console->Hide();
 } 
 
BPSWidget::~BPSWidget()
{
	quitInterpreter();
	delete_sem(startup_sem);
	delete_sem(shutdown_sem);
	delete_sem(keepup_sem);
	delete_sem(painter_sem);
	console->Lock();
	console->Quit();
}


void BPSWidget::printOnConsole(const char* text) {
	console->addText(text);
	}
	
void BPSWidget::TargetedByScrollView(BScrollView *viewer) {
	scrollView=viewer;};
	
void BPSWidget::SetPaperSize(float width, float height) {
	printf("dim %f, %f\n",width,height);
  BScrollBar *hbar = scrollView->ScrollBar(B_HORIZONTAL);
  BScrollBar *vbar = scrollView->ScrollBar(B_VERTICAL);
  int scrollWidth, scrollHeight;
  if (Window()->LockWithTimeout(100000)==B_OK) {
  	scrollWidth = (int) (width- Bounds().Width()-1);
  	scrollHeight= (int) (height- Bounds().Height()-1);
  	if (scrollWidth<0) scrollWidth=0;
  	if (scrollHeight<0) scrollHeight=0;
  	hbar->SetRange(0,scrollWidth);
  	vbar->SetRange(0,scrollHeight);
  	hbar->SetSteps(20,100);
  	vbar->SetSteps(20,100);
  	hbar->Invalidate();
  	vbar->Invalidate();
  	Invalidate();
  	Window()->Unlock();};
  };

void BPSWidget::Draw(BRect area) { 
  if(bdev) {
  	if (acquire_sem_etc(painter_sem,1,B_TIMEOUT,0)==B_NO_ERROR) {
  		// draw only if interpreter is up and running, block shutdown while drawing
    	gsdll_lock_device(bdev, 1); 
    	gsdll_draw(bdev, this, area, area); 
    	gsdll_lock_device(bdev, 0);
	  	release_sem(painter_sem);}; 
  };
} 

char *BPSWidget::GetPaperSwitch() {
	if (pagemedia) {
		int len=strlen(pagemedia);
		char *lcmedia = (char *)malloc(len+1);
		for (int i=0; i<=len; i++) lcmedia[i] = tolower(pagemedia[i]);
		sprintf(pswitch,"-sPAPERSIZE=%s",lcmedia);};
  return pswitch; };

char *BPSWidget::GetResolutionSwitch() {
	sprintf(rswitch,"-r%fx%f",xdpi,ydpi);
  return rswitch; };
  
bool BPSWidget::isInterpreterRunning() { 
  if (acquire_sem_etc(startup_sem,1,B_TIMEOUT,0)==B_NO_ERROR) {
		release_sem(startup_sem);
		return false;}
	else return true; };

bool BPSWidget::sendPS( FILE *fp, long begin, unsigned int len, bool close ){
  bool writeOk=writePS(fp,out,begin,len);
	if (close) fclose(out);
  return writeOk;
}    

void BPSWidget::startInterpreter(){
	acquire_sem(startup_sem); // allow only one interpreter to run at a time
	gs_thread = spawn_thread(gsloop, "GS Loop", B_NORMAL_PRIORITY, (void *)this);
	resume_thread(gs_thread); 
};

void BPSWidget::stopInterpreter(){
	release_sem(keepup_sem); // allow interpreter to quit
	acquire_sem(shutdown_sem); // wait until interpreter is shut down
	fclose(out);
  out = fopen(PAGE_FILE,"w");
  release_sem(startup_sem); // signal interpreter can be restarted
}

void BPSWidget::quitInterpreter(){
	if (isInterpreterRunning()) stopInterpreter();
	fclose(out);
	remove(PAGE_FILE);
	if (gs_thread) kill_thread(gs_thread);
}

void BPSWidget::interpreterFailed()
{ stopInterpreter();	} 
  



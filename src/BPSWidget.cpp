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

struct display_callback {
    /* Size of this structure */
    /* Used for checking if we have been handed a valid structure */
    int size;

    /* Major version of this structure  */
    /* The major version number will change if this structure changes. */
    int version_major;

    /* Minor version of this structure */
    /* The minor version number will change if new features are added
     * without changes to this structure.  For example, a new color
     * format.
     */
    int version_minor;

    /* New device has been opened */
    /* This is the first event from this device. */
    int (*display_open)(void *handle, void *device);

    /* Device is about to be closed. */
    /* Device will not be closed until this function returns. */
    int (*display_preclose)(void *handle, void *device);

    /* Device has been closed. */
    /* This is the last event from this device. */
    int (*display_close)(void *handle, void *device);

    /* Device is about to be resized. */
    /* Resize will only occur if this function returns 0. */
    /* raster is byte count of a row. */
    int (*display_presize)(void *handle, void *device,
        int width, int height, int raster, unsigned int format);

    /* Device has been resized. */
    /* New pointer to raster returned in pimage */
    int (*display_size)(void *handle, void *device, int width, int height,
        int raster, unsigned int format, unsigned char *pimage);

    /* flushpage */
    int (*display_sync)(void *handle, void *device);

    /* showpage */
    /* If you want to pause on showpage, then don't return immediately */
    int (*display_page)(void *handle, void *device, int copies, int flush);

    /* Notify the caller whenever a portion of the raster is updated. */
    /* This can be used for cooperative multitasking or for
     * progressive update of the display.
     * This function pointer may be set to NULL if not required.
     */
    int (*display_update)(void *handle, void *device, int x, int y,
        int w, int h);

    /* Allocate memory for bitmap */
    /* This is provided in case you need to create memory in a special
     * way, e.g. shared.  If this is NULL, the Ghostscript memory device
     * allocates the bitmap. This will only called to allocate the
     * image buffer. The first row will be placed at the address
     * returned by display_memalloc.
     */
    void *(*display_memalloc)(void *handle, void *device, unsigned long size);

    /* Free memory for bitmap */
    /* If this is NULL, the Ghostscript memory device will free the bitmap */
    int (*display_memfree)(void *handle, void *device, void *mem);

    /* Added in V2 */
    /* When using separation color space (DISPLAY_COLORS_SEPARATION),
     * give a mapping for one separation component.
     * This is called for each new component found.
     * It may be called multiple times for each component.
     * It may be called at any time between display_size
     * and display_close.
     * The client uses this to map from the separations to CMYK
     * and hence to RGB for display.
     * GS must only use this callback if version_major >= 2.
     * The unsigned short c,m,y,k values are 65535 = 1.0.
     * This function pointer may be set to NULL if not required.
     */
#if 0
    int (*display_separation)(void *handle, void *device,
        int component, const char *component_name,
        unsigned short c, unsigned short m,
        unsigned short y, unsigned short k);
#endif
};

int gsapi_new_instance(void **pinstance, void *caller_handle);
void gsapi_delete_instance(void *instance);
int gsapi_set_stdio (void *instance, int(*stdin_fn)(void *caller_handle, char *buf, int len), int(*stdout_fn)(void *caller_handle, const char *str, int len), int(*stderr_fn)(void *caller_handle, const char *str, int len));
int gsapi_exit(void* instance);
int gsapi_execute_cont(char *buf, int len);
int gsapi_execute_begin();
int gsapi_execute_end();
int gsapi_init_with_args(void* instance, int argc, const char** argv);
int gsapi_set_display_callback (void *instance, display_callback *callback);
} 


char PAGE_FILE[B_FILE_NAME_LENGTH];
bool displayComplete;
float width,height;
void* gDisplayData;
BBitmap* gBitmap;

BPSWidget *painter;
static void *bdev = NULL; 

int gsapi_display_open(void *handle, void *device)
{
	return 0;
}

int gsapi_display_preclose(void *handle, void *device)
{
	bdev = 0;
	return 0;
}

int gsapi_display_close(void *handle, void *device)
{
	return 0;
}

int gsapi_display_presize(void *handle, void *device,
        int w, int h, int r, unsigned int f)
{
	width = w;
	height = h;
	bdev = device;
	return 0;
}

int gsapi_display_size(void* handle, void* d, int w, int h, int r, unsigned int f,
	unsigned char* i)
{
	return 0;
}

int gsapi_display_sync(void* h, void* d)
{
	return 0;
}

int gsapi_display_page(void *handle, void *device, int copies, int flush)
{
	painter->SetPaperSize(width,height);
		// Also invalidates the window to trigger redraw
	displayComplete=true;
	painter->painter_lock.Unlock(); // now it's ok for painter to draw

	// Wait and keep the bitmap in memory
	acquire_sem(painter->keepup_sem);
	return 0;
}


void* gsapi_memalloc(void *handle, void *device, unsigned long size)
{
	area_id area = create_area("gs bitmap", &gDisplayData, B_ANY_ADDRESS, size, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA | B_CLONEABLE_AREA);
	gBitmap = new BBitmap(area, 0, BRect(0, 0, width - 1, height - 1), 0, B_RGB32);
	return gDisplayData;
}

int gsapi_memfree(void *handle, void *device, void *mem)
{
	delete gBitmap;
	area_id area = area_for(mem);
	delete_area(area);
	return 0;
}


int gsapi_stdin(void *caller_handle, char *buf, int len)
{
	strcpy(buf,"\n");
	return 1;
}


int gsapi_stdout(void *caller_handle, const char *str, int len)
{
	fprintf(stderr, "\x1b[33m%*.*s\x1b[0m", len, len, str);
	painter->printOnConsole(str, len);
	return len;
}


int gsapi_stderr(void *caller_handle, const char *str, int len)
{
	fprintf(stderr, "\x1b[32m%*.*s\x1b[0m", len, len, str);
	painter->printOnConsole(str, len);
	return len;
}


display_callback cb;
void* instance;

int32 gsloop(void* psview) {
	BPSWidget *ps = (BPSWidget *) psview;
	int code;
	int gs_arg;
	const char *gs_call[32];
	gs_arg=0;
	gs_call[gs_arg++] = "gs";
	gs_call[gs_arg++] = "-dQUIET";
	gs_call[gs_arg++] = "-sDEVICE=display";
	gs_call[gs_arg++] = "-dDisplayFormat=2180";
	gs_call[gs_arg++] = "-sDisplayHandle=0";
	gs_call[gs_arg++] = "-dTextAlphaBits=4";
	gs_call[gs_arg++] = "-dGraphicsAlphaBits=4";
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

	cb.size = sizeof(display_callback);
	cb.version_major = 1;
	cb.version_minor = 0;
	cb.display_open = gsapi_display_open;
	cb.display_preclose = gsapi_display_preclose;
	cb.display_close = gsapi_display_close;
	cb.display_presize = gsapi_display_presize;
	cb.display_size = gsapi_display_size;
	cb.display_sync = gsapi_display_sync;
	cb.display_page = gsapi_display_page;
	cb.display_memalloc = gsapi_memalloc;
	cb.display_memfree = gsapi_memfree;

	code = gsapi_new_instance(&instance, NULL);
	if (code != 0)
		fprintf(stderr, "Fail to create instance: %d\n", code);
	gsapi_set_stdio(instance, gsapi_stdin, gsapi_stdout, gsapi_stderr);

	code = gsapi_set_display_callback(instance, &cb);
	if (code != 0)
		fprintf(stderr, "Fail to set display callback: %d\n", code);

	code = gsapi_init_with_args(instance, gs_arg, gs_call); 
	if (code==0) {
		if (displayComplete) {
			gsapi_exit(instance);
		} else {
			painter->SetPaperSize(width,height);
			acquire_sem(painter->keepup_sem);
			gsapi_exit(instance);
		}
	} else {
		fprintf(stderr, "Fail to initialize ghostscript: %d\n", code);
	}
	release_sem(painter->shutdown_sem); // signal that interpreter has shut down
	gsapi_delete_instance(instance);
	return 0; // no errors
}


bool writePS( FILE *in, FILE *out, long begin, unsigned int len){
	int rr;
	char *buffer = (char *) malloc(BUFSIZ);
	fseek(in, begin, SEEK_SET);
	while (len>BUFSIZ) {
		rr = fread(buffer, sizeof(char), BUFSIZ, in);
		fwrite(buffer, sizeof(char), rr, out); 
		len -=rr;
	}
	rr = fread(buffer, sizeof(char), len, in);
	fwrite(buffer, sizeof(char), rr, out); 
	fflush(out);
	return true;
}    

BPSWidget::BPSWidget( BRect frame, const char *name, const char *tempDir) 
  : BView(frame, name , B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
  , painter_lock("painter lock")
{
	gs_thread=0;
	buf = (char *)malloc(BUFSIZ);
	xdpi=75.0;
	ydpi=75.0; 
	painter=this;
	bdev=0;
	startup_sem = create_sem(1,"gs_startup_protector");
	shutdown_sem = create_sem(1,"gs_shutdown_protector");
	painter_lock.Lock();
	keepup_sem = create_sem(1,"gs_keeprunning");
	if (tempDir==NULL)
		strcpy(PAGE_FILE,"/boot/home/tmp/bgv.tmp");	
	else 
		sprintf(PAGE_FILE,"%s/%s",tempDir,"bgv.tmp");
	out = fopen(PAGE_FILE,"w");
	acquire_sem(keepup_sem);
	acquire_sem(shutdown_sem);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	console=new BPSConsole(BRect(20,40,400,300),"GS Console");
	console->Hide();
	console->Show();
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
	console->Lock();
	console->Quit();
}


void BPSWidget::printOnConsole(const char* text, int len)
{
	console->addText(text, len);
}

void BPSWidget::TargetedByScrollView(BScrollView *viewer)
{
	scrollView=viewer;
}

void BPSWidget::FrameResized(float viewWidth, float viewHeight)
{
	int scrollWidth, scrollHeight;
	BScrollBar *hbar = scrollView->ScrollBar(B_HORIZONTAL);
	BScrollBar *vbar = scrollView->ScrollBar(B_VERTICAL);
  	scrollWidth = (int) (width - viewWidth);
  	scrollHeight= (int) (height - viewHeight);
  	if (scrollWidth<0) scrollWidth=0;
  	if (scrollHeight<0) scrollHeight=0;
  	hbar->SetRange(0,scrollWidth);
  	vbar->SetRange(0,scrollHeight);
  	hbar->SetSteps(20,100);
  	vbar->SetSteps(20,100);
	hbar->SetProportion(viewWidth / width);
	vbar->SetProportion(viewHeight / height);
}
	
void BPSWidget::SetPaperSize(float width, float height)
{
	if (Window()->LockWithTimeout(100000)==B_OK) {
		FrameResized(Bounds().Width() - 1, Bounds().Height() - 1);
		Invalidate();
		Window()->Unlock();
	}
}


void BPSWidget::Draw(BRect area) {
  if(bdev) {
  	if (painter_lock.LockWithTimeout(0) == B_OK) {
  		// draw only if interpreter is up and running, block shutdown while drawing
		DrawBitmap(gBitmap);
    }
  }
}


char *BPSWidget::GetPaperSwitch()
{
	if (pagemedia) {
		int len=strlen(pagemedia);
		char *lcmedia = (char *)malloc(len+1);
		for (int i=0; i<=len; i++) lcmedia[i] = tolower(pagemedia[i]);
		sprintf(pswitch,"-sPAPERSIZE=%s",lcmedia);
	}
	return pswitch;
}


char *BPSWidget::GetResolutionSwitch()
{
	sprintf(rswitch,"-r%fx%f",xdpi,ydpi);
	return rswitch;
}


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
  



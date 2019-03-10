#ifndef BGHOSTVIEW_H
#define BGHOSTVIEW_H

#include <Be.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include "BPSWidget.h"
#include "BGVPageItem.h"
#include "AToolBar.h"
#include "Preferences.h"
 
class BGhostview : public BWindow
{

public:
	BGhostview(BRect frame, const char* title);
	void Quit();
	bool QuitRequested();
	void setup();
	void show_page(unsigned int number);
	void set_magstep(int i);
	void new_file(int number);
	void openFile(const char* name );
	void set_pagemedia( int number );

	void readSettings();
	void writeSettings();
	void updateRecentList();
	BView *createToolbar(BRect frame);
	BView *createMenubar(BRect frame);
	void MessageReceived(BMessage *message);
	
	BListView *marklist;
	BPSWidget *page;
	BScrollView *scrollView;
	AToolBar *toolBar;
	BMenuField *magBar;
	BMenuField *orientationBar;
	BMenuField *paperBar;
			
	void printStart( int mode, bool reverseOrder, bool toFile, 
					char* printerName, char* spoolerCommmand,
					char* printerVariable,
					int pgStart, int pgEnd );
				
	void printToFile( char* fname, bool all);
				
	char* printToPrinter( char* printerName, 
				char* spoolerCommand, char* printerVariable,
				bool allMode, char** ml  );
	
	FILE		*psfile;
	char  	filename[B_FILE_NAME_LENGTH];	
	char		psTmp[B_FILE_NAME_LENGTH];
	char 		*tempDir;
	int			current_page;
	int 		pages_in_part[10];
	int 		num_parts;
	struct 		document *doc;	
	
	BList* lastOpened;
	char* tempFile;
	
public:
	void scrollTop();
	void scrollDown();
	void scrollUp();
	void scrollLeft();
	void scrollRight();
	void about();
	void help();
	void nextPage();
	void prevPage();
	void goToPage();
	void goToStart();
	void goToEnd();
	void readDown();
	void print();
	void zoomIn();
	void zoomOut();
	void copyright();
	void openLoadPanel();
	void openSaveAllPanel();
	void openSaveMarkedPanel();
	void openRecent( int id );

private:
	BFilePanel *filePanel;
	BFilePanel *savePanel;

	BMenuBar *menubar;
	BMenu *m_file;
	BMenu *m_recent;
	BMenu *m_view;
	BMenu *m_go;
	BMenu *m_pagemarks;
	BMenu *m_help;
	
	char* printerName;
	int printSelection;
	int magstep;
	int pagemedia;
	bool	toc_text;
	int	toc_length;
	int	toc_entry_length;
	char page_total_label[20];
	char part_total_label[20];
	char* part_string;
	char* page_string;
	char* page_label_text;
	char* part_label_text;
	char* position_label;	 
	BBitmap *icons[10];   
};

#define BGV_NEW_FILE	'GVNF'
#define BGV_OLD_FILE	'GVOF'
#define BGV_OPEN_RECENT	'GVOR'
#define BGV_SAVE_ALL	'GVSA'
#define BGV_SAVE_MARKED	'GVSM'
#define BGV_SAVE_ALL_REFS	'GVAR'
#define BGV_SAVE_MARKED_REFS	'GVMR'
#define BGV_REDISPLAY	'GVRD'
#define BGV_PAGE_SETUP 'GVPS'
#define BGV_PRINT_ALL	'GVPA'
#define BGV_PRINT_MARKED	'GVPM'
#define BGV_EXIT	'GVEX'
#define BGV_ZOOMIN	'GVZI'
#define BGV_ZOOMOUT 'GVZO'
#define BGV_ZOOM		'GVZM'
#define BGV_INFO		'GVIN'
#define BGV_CLOSE		'GVCL'
#define BGV_PREV_PAGE 'GVPP'
#define BGV_NEXT_PAGE	'GVNP'
#define BGV_GOTO_PAGE 'GVGP'
#define BGV_GOTO_START 'GVGS'
#define BGV_GOTO_END 'GVGE'
#define BGV_READ_DOWN 'GVDN'
#define BGV_MARK_CURRENT 'GVMC'
#define BGV_MARK_ALL	'GVMA'
#define BGV_MARK_EVEN	'GVME'
#define BGV_MARK_ODD	'GVMO'
#define BGV_TOGGLE_MARKS 'GVTM'
#define BGV_REMOVE_MARKS 'GVRM'
#define BGV_PAGE_LIST	'GVPL'
#define BGV_ABOUT 'GVAB'
#define BGV_HELP 'GVHL'
#define BGV_COPYRIGHT 'GVCR'
#define BGV_SELECT_PAGE 'GVSP'
#define BGV_INVOKE_PAGE 'GVIP'
#define BGV_SET_MAGSTEP 'GVMG'
#define BGV_SET_MEDIA 'GVMD'
#define BGV_CONSOLE 'GVCN'


#endif // BGHOSTVIEW_H



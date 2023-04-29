/****************************************************************************
**
** Name       : BGhostview
** Author     : Andreas Raquet
** Copyright  : GPL
** Description: Main Ghostview-Window, sets up controls, manages
**              settings like recent-list and handles user-input.
**              Uses ps.c to parse ps-files and feed the
**							BPSWidget with single pages upon user-request.
**
** Based on code in KGhostview Copyright (C) 1997 by Mark Donohoe.
** Based on original work by Tim Theisen.
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/
#include "BGhostview.h"
#include "BGVDropTarget.h"

#ifndef BUFSIZE
#define BUFSIZE 1024
#endif

BGhostview::BGhostview( BRect frame, const char *name )
	: BWindow( frame,name,B_DOCUMENT_WINDOW,0)
{
	tempFile=NULL;
	filePanel=NULL;
	psfile=0;
	doc=0;
	current_page=-1;
	num_parts=0;
	for(int part_count=0;part_count<10;part_count++) {
		pages_in_part[part_count]=0;
	}

	magstep = 9;
	pagemedia=0;
	lastOpened=new BList();
	readSettings();

	BPath path;
	find_directory(B_SYSTEM_TEMP_DIRECTORY, &path);
	strcpy(tempDir, path.Path());
	strcpy(psTmp, tempDir);
	strcat(psTmp, "/");

	BView *menuBar=createMenubar(BRect(0,0,0,0));
	AddChild(menuBar);
	toolBar = (AToolBar *)createToolbar(BRect(0,menuBar->Bounds().Height()+1,frame.Width(),25));
	AddChild(toolBar);
	BView *mainView = new BGVDropTarget(this, BRect(0,menuBar->Bounds().Height()+toolBar->Bounds().Height()+1,frame.Width(),frame.Height()),"GVMainView",B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	int width=(int) mainView->StringWidth("0000 0000");
	marklist = new BListView(BRect(3,4,width,mainView->Bounds().Height()-B_H_SCROLL_BAR_HEIGHT),"Marks",B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	marklist->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	mainView->AddChild(new BScrollView("ScrollMarks",marklist, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, B_FULL_UPDATE_ON_RESIZE, false, true));
	page = new BPSWidget(BRect(width+5+B_V_SCROLL_BAR_WIDTH,4,mainView->Bounds().Width()-B_H_SCROLL_BAR_HEIGHT,mainView->Bounds().Height()-B_V_SCROLL_BAR_WIDTH),"PSView", tempDir);
	scrollView =new BScrollView("ScrollPage",page, B_FOLLOW_ALL, B_WILL_DRAW, true, true);
	mainView->AddChild(scrollView);
	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));  
	marklist->SetSelectionMessage(new BMessage(BGV_SELECT_PAGE));
	marklist->SetInvocationMessage(new BMessage(BGV_INVOKE_PAGE));
	AddChild(mainView);

	printSelection = 1;
	printerName = "lp0";

	Show();
}


void BGhostview::MessageReceived(BMessage *message) {
	switch (message->what) {
	case BGV_OLD_FILE: openLoadPanel(); 
		break;
	case BGV_OPEN_RECENT:
		BMenuItem* item;
		message->FindPointer("source", (void**)&item);
		openFile(item->Label());
		break;
  case BGV_CONSOLE: page->toggleConsole();  break;
  case BGV_REDISPLAY: show_page(current_page); 
  	break;
  case BGV_PRINT_ALL: print(); 
  	break;
  case BGV_SAVE_ALL: openSaveAllPanel(); 
  	break;
  case BGV_SAVE_MARKED: openSaveMarkedPanel(); 
  	break;
  case BGV_ZOOMIN: zoomIn(); 
  	break;
  case BGV_ZOOMOUT: zoomOut(); 
  	break;
  case BGV_PREV_PAGE: prevPage(); 
  	break;
  case BGV_NEXT_PAGE: nextPage(); 
  	break;
  case BGV_GOTO_START: goToStart(); 
  	break;
  case BGV_GOTO_END: goToEnd(); 
  	break;
  case BGV_READ_DOWN: readDown(); 
  	break;
  case BGV_MARK_CURRENT: marklist->Select(current_page,true);
  	break;
  case BGV_MARK_ALL: marklist->Select(0,marklist->CountItems()-1);
  				break;
  case BGV_MARK_EVEN: 
  				marklist->DeselectAll();
  				for (int i=0; i<marklist->CountItems(); i+=2) 
  					marklist->Select(i,true);
  				break;
  case BGV_MARK_ODD: 
  				marklist->DeselectAll();
  				for (int i=1; i<marklist->CountItems(); i+=2) 
  					marklist->Select(i,true);
  				break;
  case BGV_TOGGLE_MARKS: 
  				for (int i=0; i<marklist->CountItems(); i++)
  					if (marklist->IsItemSelected(i)) marklist->Deselect(i);
  					else marklist->Select(i,true);
  				break;
  case BGV_REMOVE_MARKS: 
  				marklist->DeselectAll();
  				break;
  case BGV_ABOUT: about(); break;
  case BGV_COPYRIGHT: copyright(); break;
  case BGV_INVOKE_PAGE: show_page(message->FindInt32("index")); break;
  case BGV_SELECT_PAGE:
  	if (marklist->CurrentSelection(1)==-1)
  		show_page(message->FindInt32("index"));
  	break;
  case B_REFS_RECEIVED: {
    uint32 type;
    int32 count;
    entry_ref ref;
		message->GetInfo("refs",&type,&count);
    if (type == B_REF_TYPE && message->FindRef("refs",count-1,&ref)==B_OK) {
    		BEntry entry(&ref);
    		BPath path;
    		entry.GetPath(&path);
				openFile(path.Path());
  	}}; 
  	break;
  case BGV_SAVE_MARKED_REFS:
  case BGV_SAVE_ALL_REFS: {
    uint32 type;
    int32 count;
    entry_ref ref;
    const char *fname=new char[B_PATH_NAME_LENGTH];
		message->GetInfo("directory",&type,&count);
    if (type == B_REF_TYPE && message->FindRef("directory",count-1,&ref)==B_OK) {
    	message->GetInfo("name",&type,&count);
    	if (type==B_STRING_TYPE && message->FindString("name",&fname)==B_OK) {
    		BEntry entry(&ref);
    		BPath path;
    		entry.GetPath(&path);
    		path.Append(fname);
    		printToFile(path.Path(), message->what==BGV_SAVE_ALL_REFS);
  	}}
  }; 
  	break;
  case BGV_SET_MAGSTEP: 
 		set_magstep(message->FindInt32("index"));
    break;
  case BGV_SET_MEDIA: 
 		set_pagemedia(message->FindInt32("index"));
    break;
  default: 
    BWindow::MessageReceived(message);
  };
}; 

void BGhostview::Quit() {
	writeSettings();
	if (page->isInterpreterRunning()) page->quitInterpreter(); 
	if (tempFile!=NULL) remove(tempFile);
	if (psfile!=NULL) fclose(psfile);
	for (int i=0; i<10; i++) delete icons[i];
	BWindow::Quit();
}	

bool BGhostview::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
};
 
void BGhostview::copyright()
{
	BAlert *d = new BAlert("Copyright",
	"This program is free software; you can redistribute it and/or modify "\
	"it under the terms of the GNU General Public License as published by "\
	"the Free Software Foundation; either version 2 of the License, or "\
	"(at your option) any later version.\n\n"\
	"This program is distributed in the hope that it will be useful, "\
	"but WITHOUT ANY WARRANTY; without even the implied warranty of "\
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "\
	"GNU General Public License for more details.\n\n"\
	"You should have received a copy of the GNU General Public License "\
	"along with this program; if not, write to the Free Software "\
	"Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.",
	"Ok",NULL,NULL,B_WIDTH_AS_USUAL,B_INFO_ALERT);
	d->Go();
}


void BGhostview::about()
{
	BAlert *d = new BAlert("About","BGhostview\n\nAuthor: Andreas Raquet\n\n"\
	"based on\n\nKGhostview by Mark Donohoe and\n"\
	"Ghostview by Tim Teisen\n\nRequires Aladdin Ghostscript port by Jake Hamby",
	"Ok",NULL,NULL,B_WIDTH_AS_USUAL,B_INFO_ALERT);
	d->Go();
}

BView *BGhostview::createMenubar(BRect frame)
{
	BMenuItem *item;
	m_recent = new BMenu("Open recent");
	
	int32 items = lastOpened->CountItems();
	for( int i=0; i<items; i++ ) {
		m_recent->AddItem(new BMenuItem((char*) lastOpened->ItemAt(i), new BMessage(BGV_OPEN_RECENT)));
	}
	
 	m_file = new BMenu("File");
	m_file->AddItem(new BMenuItem("Open...", new BMessage(BGV_OLD_FILE)));
	m_file->AddItem(m_recent);
	item=new BMenuItem("Save as...", new BMessage(BGV_SAVE_ALL));
	item->SetEnabled(false);
	m_file->AddItem(item);
	item=new BMenuItem("Save marked...", new BMessage(BGV_SAVE_MARKED));
	item->SetEnabled(false);
	m_file->AddItem(item);
	m_file->AddSeparatorItem();
	item=new BMenuItem("Reload", new BMessage(BGV_REDISPLAY));
	item->SetEnabled(false);
	m_file->AddItem(item);
  m_file->AddSeparatorItem();
	item=new BMenuItem("Print...", new BMessage(BGV_PRINT_ALL));
	item->SetEnabled(false);
	m_file->AddItem(item);
	item=new BMenuItem("Print marked...", new BMessage(BGV_PRINT_MARKED));
	item->SetEnabled(false);
	m_file->AddItem(item);
  m_file->AddSeparatorItem();
	m_file->AddItem(new BMenuItem("Exit", new BMessage(B_QUIT_REQUESTED)));
    
	m_view = new BMenu("Zoom");
	item=new BMenuItem("Zoom in", new BMessage(BGV_ZOOMIN));
	item->SetEnabled(false);
	m_view->AddItem(item);
	item=new BMenuItem("Zoom out", new BMessage(BGV_ZOOMOUT));
	item->SetEnabled(false);
	m_view->AddItem(item);
	item=new BMenuItem("Zoom...", new BMessage(BGV_ZOOM));
	item->SetEnabled(false);
	m_view->AddItem(item);

  m_go = new BMenu("Go");
	item=new BMenuItem("Previous page", new BMessage(BGV_PREV_PAGE));
	item->SetEnabled(false);
	m_go->AddItem(item);
	item=new BMenuItem("Next Page", new BMessage(BGV_NEXT_PAGE));
	item->SetEnabled(false);
	m_go->AddItem(item);
  m_go->AddSeparatorItem();
	item=new BMenuItem("Go to start", new BMessage(BGV_GOTO_START));
	item->SetEnabled(false);
	m_go->AddItem(item);
	item=new BMenuItem("Go to end", new BMessage(BGV_GOTO_END));
	item->SetEnabled(false);
	m_go->AddItem(item);
	item=new BMenuItem("Read down", new BMessage(BGV_READ_DOWN));
	item->SetEnabled(false);
	m_go->AddItem(item);
	
	m_pagemarks = new BMenu("Pagemarks");
	item=new BMenuItem("Mark current page", new BMessage(BGV_MARK_CURRENT));
	item->SetEnabled(false);
	m_pagemarks->AddItem(item);	
	item=new BMenuItem("Mark all pages", new BMessage(BGV_MARK_ALL));
	item->SetEnabled(false);
	m_pagemarks->AddItem(item);	
	item=new BMenuItem("Mark even pages", new BMessage(BGV_MARK_EVEN));
	item->SetEnabled(false);
	m_pagemarks->AddItem(item);	
	item=new BMenuItem("Mark odd pages", new BMessage(BGV_MARK_ODD));
	item->SetEnabled(false);
	m_pagemarks->AddItem(item);	
  m_pagemarks->AddSeparatorItem();
	item=new BMenuItem("Toggle page marks", new BMessage(BGV_TOGGLE_MARKS));
	item->SetEnabled(false);
	m_pagemarks->AddItem(item);	
	item=new BMenuItem("Remove page marks", new BMessage(BGV_REMOVE_MARKS));
	item->SetEnabled(false);
	m_pagemarks->AddItem(item);	
      
  m_help = new BMenu("Help");

	m_help->AddItem(new BMenuItem("About...", new BMessage(BGV_ABOUT)));
	m_help->AddItem(new BMenuItem("Copyright", new BMessage(BGV_COPYRIGHT)));
	m_help->AddItem(new BMenuItem("GS Console", new BMessage(BGV_CONSOLE)));
	
  menubar = new BMenuBar(frame,"BGV_Menubar");
    
  menubar->AddItem( m_file );
  menubar->AddItem( m_view );
  menubar->AddItem( m_go );
	menubar->AddItem( m_pagemarks );
	menubar->AddItem( m_help ); 
	return menubar;	
}

BView* BGhostview::createToolbar(BRect frame)
{
	AToolBar *toolBar=new AToolBar(frame,"BGV_Toolbar");
	icons[0] = BTranslationUtils::GetBitmap("fileopen");
	icons[1] = BTranslationUtils::GetBitmap("reload");
	icons[2] = BTranslationUtils::GetBitmap("printpage");
	icons[3] = BTranslationUtils::GetBitmap("zoomin");
	icons[4] = BTranslationUtils::GetBitmap("zoomout");
	icons[5] = BTranslationUtils::GetBitmap("firstpage");
	icons[6] = BTranslationUtils::GetBitmap("prevpage");
	icons[7] = BTranslationUtils::GetBitmap("next");
	icons[8] = BTranslationUtils::GetBitmap("nextpage");
	icons[9] = BTranslationUtils::GetBitmap("lastpage");
	if (icons[0] && icons[1] && icons[2] && icons[3] && icons[4] && icons[5] && icons[6] && icons[7] && icons[8] && icons[9]) {
		toolBar->AddIcon(icons[0] ,"fileopen", new BMessage(BGV_OLD_FILE));
		toolBar->AddIcon(icons[1] ,"reload",new BMessage(BGV_REDISPLAY),false);
		toolBar->AddIcon(icons[2] ,"printpage",new BMessage(BGV_PRINT_ALL),false);
		toolBar->AddIcon(icons[3] ,"zoomin",new BMessage(BGV_ZOOMIN),false);
		toolBar->AddIcon(icons[4] ,"zoomout",new BMessage(BGV_ZOOMOUT),false);
		toolBar->AddIcon(icons[5] ,"firstpage",new BMessage(BGV_GOTO_START),false);
		toolBar->AddIcon(icons[6] ,"prevpage",new BMessage(BGV_PREV_PAGE),false);
		toolBar->AddIcon(icons[7] ,"page",new BMessage(BGV_READ_DOWN),false);
		toolBar->AddIcon(icons[8] ,"nextpage",new BMessage(BGV_NEXT_PAGE),false);
		toolBar->AddIcon(icons[9] ,"lastpage",new BMessage(BGV_GOTO_END),false);
	} else {
		BAlert *d = new BAlert("Error","Could not locate icons.\nMake sure XPMTranslator is installed.","Ok");
		d->Go();
	}

	BPopUpMenu *magMenu=new BPopUpMenu("100%");
	BMenuItem *item;
	for (int i = 1; i<=10; i++) {
		char *magStr = new char[6];
		sprintf(magStr,"%d0%%",i);
		item=new BMenuItem(magStr, new BMessage(BGV_SET_MAGSTEP));
		if (i==magstep+1) item->SetMarked(true);
		magMenu->AddItem(item);};
	item=new BMenuItem("125%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	item=new BMenuItem("150%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	item=new BMenuItem("175%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	item=new BMenuItem("200%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	item=new BMenuItem("250%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	item=new BMenuItem("300%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	item=new BMenuItem("400%", new BMessage(BGV_SET_MAGSTEP));
	magMenu->AddItem(item);
	magBar = new BMenuField(BRect(0,0,60,20),"BGV_MagBar","",magMenu);
	magBar->SetDivider(0);
	toolBar->AddView(magBar);
	BPopUpMenu *orientationMenu=new BPopUpMenu("Portrait");
	orientationMenu->AddItem(new BMenuItem("Portrait", new BMessage(BGV_SET_MAGSTEP)));
	orientationMenu->AddItem(new BMenuItem("Landscape", new BMessage(BGV_SET_MAGSTEP)));
	orientationMenu->AddItem(new BMenuItem("Seascape", new BMessage(BGV_SET_MAGSTEP)));
	orientationMenu->AddItem(new BMenuItem("Upside down", new BMessage(BGV_SET_MAGSTEP)));
	orientationBar = new BMenuField(BRect(0,0,80,25),"BGV_OrientBar",NULL,orientationMenu);
	orientationBar->SetDivider(0);
	//toolBar->AddView(orientationBar);
	BPopUpMenu *paperMenu=new BPopUpMenu("Document");
	item=new BMenuItem("Document", new BMessage(BGV_SET_MEDIA));
	if (0==pagemedia) item->SetMarked(true);
	paperMenu->AddItem(item);
	for (int i = 0; papersizes[i].name; i++) {
		item=new BMenuItem(papersizes[i].name, new BMessage(BGV_SET_MEDIA));
		if (i+1==pagemedia) item->SetMarked(true);
		paperMenu->AddItem(item);
	}; 
	paperBar = new BMenuField(BRect(0,0,100,25),"BGV_PaperBar",NULL,paperMenu);
	paperBar->SetDivider(0);
	toolBar->AddView(paperBar);
	return toolBar;
}


void BGhostview::readSettings()
{
	Preferences prefs("x-vnd.raquet-bghostview");
	if (prefs.InitCheck()) {
		for (int i=0; i<5 ;i++) {
		char *s = new char[B_PATH_NAME_LENGTH];
		s[0]=0;
		lastOpened->AddItem(s);};
		return;};
	PreferenceSet set(prefs,"BGhostview",true);
	if (set.InitCheck()) {
		for (int i=0; i<5 ;i++) {
		char *s = new char[B_PATH_NAME_LENGTH];
		s[0]=0;
		lastOpened->AddItem(s);};
		return;};
	const void* data;
	ssize_t size;
	uint32 type='    ';
	char *s = new char[B_PATH_NAME_LENGTH];
	if(!set.GetData("Recent0",data, size, type)) {
		memcpy(s,data,size);	}
	else
		s[0]=0;
	lastOpened->AddItem(s);
	s = (char *) malloc(B_PATH_NAME_LENGTH);
	if(!set.GetData("Recent1",data, size, type)) {
		memcpy(s,data,size);	}
	else
		s[0]=0;
	lastOpened->AddItem(s);
	s = new char[B_PATH_NAME_LENGTH];
	if(!set.GetData("Recent2",data, size, type))
		memcpy(s,data,size);	
	else
		s[0]=0;
	lastOpened->AddItem(s);
	s = new char[B_PATH_NAME_LENGTH];
	if(!set.GetData("Recent3",data, size, type))
		memcpy(s,data,size);	
	else
		s[0]=0;
	lastOpened->AddItem(s);
	s = new char[B_PATH_NAME_LENGTH];
	if(!set.GetData("Recent4",data, size, type))
		memcpy(s,data,size);	
	else
		s[0]=0;
	lastOpened->AddItem(s);
	if (!set.GetData("Zoom",data,size,type))
		magstep = *((int*) data);
	else magstep=9;	
	if (!set.GetData("Pagemedia",data,size,type))
		pagemedia = *((int*) data);
	else pagemedia=0;	
}

void BGhostview::writeSettings() {
	Preferences prefs("x-vnd.raquet-bghostview");
	PreferenceSet set(prefs,"BGhostview",true);
	set.SetData("Recent0",lastOpened->ItemAt(0), B_PATH_NAME_LENGTH);
	set.SetData("Recent1",lastOpened->ItemAt(1), B_PATH_NAME_LENGTH);
	set.SetData("Recent2",lastOpened->ItemAt(2), B_PATH_NAME_LENGTH);
	set.SetData("Recent3",lastOpened->ItemAt(3), B_PATH_NAME_LENGTH);
	set.SetData("Recent4",lastOpened->ItemAt(4), B_PATH_NAME_LENGTH);
	set.SetData("Zoom",&magstep, sizeof(int));
	set.SetData("Pagemedia",&pagemedia, sizeof(int));
	BRect frame(Frame());
	set.SetData("Bounds",&frame,sizeof(BRect));
	set.Save();
};

void BGhostview::updateRecentList() {
	if (!strcmp(filename,(char *) lastOpened->ItemAt(0))) return;
	if (!strcmp(filename,(char *) lastOpened->ItemAt(1))) return;
	if (!strcmp(filename,(char *) lastOpened->ItemAt(2))) return;
	if (!strcmp(filename,(char *) lastOpened->ItemAt(3))) return;
	if (!strcmp(filename,(char *) lastOpened->ItemAt(4))) return;
	int count = m_recent->CountItems();
	for (int i=0; i<count; i++)
		delete m_recent->RemoveItem((int32)0);
	void *ofname=lastOpened->LastItem();
	lastOpened->RemoveItem(ofname);
	delete[] (char*)ofname;
	char *fname = new char[B_PATH_NAME_LENGTH];
	strcpy(fname,filename);
	lastOpened->AddItem(fname,0);
	int32 items = lastOpened->CountItems();
	for( int i=0; i<items; i++ ) {
		m_recent->AddItem(new BMenuItem((const char*) lastOpened->ItemAt(i), new BMessage(i)));
	}
};

void BGhostview::readDown()
{
	float min, max;
	BScrollBar *vbar = scrollView->ScrollBar(B_VERTICAL);
	vbar->GetRange(&min,&max);
	if (vbar->Value()>max-10)
		nextPage();
	else
	  scrollDown();
}

void BGhostview::scrollTop()
{
	BScrollBar *vbar = scrollView->ScrollBar(B_VERTICAL);
	vbar->SetValue(0);
}

void BGhostview::scrollUp()
{
	float min, max;
	BScrollBar *vbar = scrollView->ScrollBar(B_VERTICAL);
	vbar->GetRange(&min,&max);
	if (vbar->Value()<150) vbar->SetValue(0);
	else vbar->SetValue(vbar->Value()-150);
}

void BGhostview::scrollDown()
{
	float min, max;
	BScrollBar *vbar = scrollView->ScrollBar(B_VERTICAL);
	vbar->GetRange(&min,&max);
	if (vbar->Value()>max-150) vbar->SetValue(max);
	else vbar->SetValue(vbar->Value()+150);
}

void BGhostview::scrollLeft()
{
	float min, max;
	BScrollBar *hbar = scrollView->ScrollBar(B_HORIZONTAL);
	hbar->GetRange(&min,&max);
	if (hbar->Value()<150) hbar->SetValue(0);
	else hbar->SetValue(hbar->Value()-150);
}

void BGhostview::scrollRight()
{
	float min, max;
	BScrollBar *hbar = scrollView->ScrollBar(B_HORIZONTAL);
	hbar->GetRange(&min,&max);
	if (hbar->Value()>max-150) hbar->SetValue(max);
	else hbar->SetValue(hbar->Value()+150);
}

void BGhostview::print()
{
	/*
	pd = new PrintDialog( this, "print dialog", doc->numpages, 
								( marklist->markList()->count() > 0 ) );
	pd->setCaption(i18n("Print"));
	
	if( pd->exec() ) {
		printStart( pd->pgMode, pd->cbOrder->isChecked(),
					pd->cbFile->isChecked(),
					pd->printerName, pd->spoolerCommand,
					pd->printerVariable,
					QString( pd->leStart->text() ).toInt(),
					QString( pd->leEnd->text() ).toInt() );
	}*/
}


void BGhostview::zoomIn()
{
	if (magstep<19) set_magstep(magstep+1);
}

void BGhostview::zoomOut()
{
	if (magstep>0) set_magstep(magstep-1);
}

void BGhostview::nextPage()
{
    int new_page = 0;

    if (toc_text) {
	    new_page = current_page + 1;
			if ((unsigned int) new_page >= doc->numpages)
			return;
    }
	  show_page(new_page);
    scrollTop();
}

void BGhostview::goToStart()
{
    int new_page = 0;
    
	  show_page(new_page);
    scrollTop();
}

void BGhostview::goToEnd()
{
    int new_page = 0;

    if (toc_text) new_page = doc->numpages-1;
    scrollTop();    
	  show_page(new_page);
}


void BGhostview::prevPage()
{
    int new_page = 0;
	
    if (toc_text) {
	    new_page = current_page - 1;
		  if (new_page < 0) return;
    }
	
	  show_page(new_page);
    scrollTop();
}

void BGhostview::openLoadPanel()
{
	if (filePanel==NULL) {
		filePanel=new BFilePanel(B_OPEN_PANEL,NULL,NULL,0,false);
		}
	filePanel->Show();
}

void BGhostview::openSaveAllPanel()
{
	if (savePanel==NULL) {
		savePanel=new BFilePanel(B_SAVE_PANEL,NULL,NULL,0,false);
		}
	BMessage message(BGV_SAVE_ALL_REFS);
	savePanel->SetMessage(&message);
	savePanel->Show();
}

void BGhostview::openSaveMarkedPanel()
{
	if (savePanel==NULL) {
		savePanel=new BFilePanel(B_SAVE_PANEL,NULL,NULL,0,false);
		}
	BMessage message(BGV_SAVE_MARKED_REFS);
	savePanel->SetMessage(&message);
	savePanel->Show();
}

void BGhostview::printStart( int mode, bool reverseOrder, 
					bool toFile,
					char* printerName, char* spoolerCommand,
					char* printerVariable,
					int pgStart, int pgEnd )
{ 


/*
	QStrList *ml = new QStrList;
	
	switch( mode ) {
		case PrintDialog::All:
			break;
				
		case PrintDialog::Current:
			ml->append( marklist->text( current_page ) );
			break;
				
		case PrintDialog::Marked:
			ml = marklist->markList();
			break;
				
		case PrintDialog::Range:
			if ( pgStart <= pgEnd )
				for( int j = pgStart-1; j< pgEnd; j++ )
					ml->append( marklist->text( j ) );
			else 
				for( int j = pgEnd-1; j< pgStart; j++ )
					ml->append( marklist->text( j ) );
			break;
	}
	
	if ( reverseOrder ) {
		QStrList *sl = new QStrList;
		for( ml->last(); ml->current(); ml->prev() )
			sl->append( ml->current() );
		*ml = *sl;
		delete sl;
	}
	
	QString error;
	
	if ( toFile ) {
		error = printToFile( ( mode == PrintDialog::All ), ml );
	} else { 
		error = printToPrinter( printerName, spoolerCommand, printerVariable,
						( mode == PrintDialog::All ), ml );
	}
	
	if ( error ) QMessageBox::warning( 0, i18n( "Error printing" ), error );
	
	delete ml;*/
}


char* BGhostview::printToPrinter( const char* printerName, char* spoolerCommand,
						char* printerVariable, bool allMode, char **ml )
{
    /*
    FILE *printer;
    SIGVAL (*oldsig) (int);
    int bytes = 0;
    char buf[ BUFSIZ ];
    Bool failed;
    QString ret_val;

	page->disableInterpreter();

	// For SYSV, SVR4, USG printer variable="LPDEST", print command=lp
	// Other systems printer variable="PRINTER", print command=lpr
	
	printerVariable.append("\"");
	printerVariable.prepend("\"");

    if ( printerName.data() != '\0') {
		setenv( printerVariable.data(), printerName.data(), True );
    }
    oldsig = signal( SIGPIPE, SIG_IGN );
    printer = popen( spoolerCommand.data(), "w" );
	
    if ( toc_text && !allMode ) {
		psCopyDoc( printer, ml );
    } else {
		FILE *tmpfile = ::fopen( filename, "r" );

		while ( ( bytes = ::read( fileno(tmpfile), buf, BUFSIZ ) ) ) {
	   		bytes = ::write( fileno(printer), buf, bytes);
	   	}
		::fclose( tmpfile );
    }
	
    failed = ( pclose( printer ) != 0 );
	
    if ( failed ) {
		sprintf( buf, 
			i18n( "Print failure : %s" ), spoolerCommand.data() );
		ret_val = QString( buf );
    } else {
		ret_val = 0;
    }
	
	signal( SIGPIPE, oldsig );
    return( ret_val );*/
}


void BGhostview::printToFile(const char *destFile, bool all) {
	FILE *out;
	if (toc_text) {
		int32 max=marklist->CountItems();
		int numpages=0;
		char pages[max+1];
		for (int i=0; i<max; i++) {
			BGVPageItem *item = (BGVPageItem *) marklist->ItemAt(i);
			if (item->IsSelected() || all) {
				pages[i]='*';
				numpages++;
			}
			else pages[i]='-';
			}
		pages[max]=0;
		if (numpages>0) {
			out = fopen(destFile,"w");
			fseek(psfile,0,SEEK_SET);
			pscopydoc(out,psfile,doc,pages);
			fclose(out);
		}
		else {
			BAlert *d = new BAlert("Error","No pages are marked.\nSaving would create an empty file.","Abort");
			d->Go();
		}
	}
	else { // copy if only one page in source or source unknown
		char buf[BUFSIZE];
		int bytes = fread(buf,sizeof(char),BUFSIZE, psfile);
		out = fopen(destFile,"w");
		fseek(psfile,0,SEEK_SET);
		while (bytes==BUFSIZE) {
			fwrite(buf,sizeof(char),bytes,out); 
			bytes = fread(buf,sizeof(char),BUFSIZE, psfile);
		}
		fwrite(buf,sizeof(char),bytes,out);
		fclose(out); 
	}
};

void BGhostview::openFile(const char* name ) {
  FILE *fp;
	if ( strlen(name)==0 ) {
		BAlert *alert=new BAlert("BGhostview Error","Error opening file\nNo file name was specified.\nNo document was loaded.","Ok");
		alert->Go();
		return;}
  if (strcmp(name, "-")) {
    if ( ( fp = fopen(name, "r") ) == 0 ) {
			BAlert *alert=new BAlert("BGhostview Error","The document could not be opened.\nNo document has been loaded.","Ok");
			alert->Go();
 			return ;} 
	  else {
			strcpy(filename,name);
			if (psfile) fclose(psfile);
	    psfile = fp;
	    	
	    new_file(0);
			show_page(0);
	    return;}
    } else {
    
		strcpy(filename,name);
		if ( psfile ) fclose( psfile );
		psfile = 0;
		
		new_file(0);		
		show_page(0);
      	
		return;
  }
}



bool useful_page_labels;
void	BGhostview::setup()
{
 	int k;
  int this_page, last_page=0;
    
	if (!marklist->IsEmpty()) {
		Lock();
		int count = marklist->CountItems();
		for (int i=0; i<count; i++) 
			delete marklist->RemoveItem((int32)0);
		Unlock();
	};
  if (doc!=NULL) psfree( doc );
  doc = NULL;
  current_page = -1;
  toc_text = 0;
  for(k=0;k<10;k++) pages_in_part[k]=0;
  num_parts=0;
  // Scan document and start setting things up
  if (psfile) {
		// 18/3/98 Jake Hamby patch - slightly changed for BeOS-port
		char *filename_dscP = 0;
		char *filename_uncP = 0;
		char *error_name=0;
		char *error_details=0;
		char cmd_scan_pdf[512];
		strcpy(cmd_scan_pdf, tempDir);
		strcat(cmd_scan_pdf,"/Ghostscript/bin/gs -dNODISPLAY -dQUIET -sPDFname=\"%s\" -sDSCname=\"%s\" pdf2dsc.ps -c quit");
		const char *cmd_uncompress = "gzip -d -c %s > %s";
		doc = psscan(&psfile, filename,psTmp, &filename_dscP,cmd_scan_pdf, &filename_uncP, cmd_uncompress, &error_name, &error_details);
		if (error_name[0]!=0) {
			BAlert *d = new BAlert(error_name,error_details,"Ok");
			d->Go();
		}	
	if (tempFile!=NULL) {
		remove(tempFile);
		delete tempFile;
		tempFile=NULL; };	

	if(filename_dscP!=NULL) {
		strcpy(filename,filename_dscP);
		free(filename_dscP);
		tempFile= new char[B_PATH_NAME_LENGTH];
		strcpy(tempFile,filename);}

	if (filename_uncP!=NULL) {
		strcpy(filename,filename_uncP);
		free(filename_uncP);
		tempFile= new char[B_PATH_NAME_LENGTH];
		strcpy(tempFile,filename);}
	// end of patch
  }
  if (doc && ((!doc->epsf && doc->numpages > 0) ||
		 (doc->epsf && doc->numpages > 1))) {
		int maxlen = 0;
		unsigned int i, j;
		useful_page_labels = false;
		if (doc->numpages == 1) useful_page_labels = true;
		for (i = 1; i < doc->numpages; i++)
			if ( (useful_page_labels = (useful_page_labels ||
				strcmp(doc->pages[i-1].label, doc->pages[i].label)))) break;
		if (useful_page_labels) {
			for (i = 0; i < doc->numpages; i++) 
				if((unsigned int)maxlen<strlen(doc->pages[i].label))
					maxlen=strlen(doc->pages[i].label);} 
		else {
			double x;
			x = doc->numpages;
			maxlen = (int)( log10(x) + 1 );}
		toc_entry_length = maxlen + 3;
		toc_length = doc->numpages * toc_entry_length - 1;
		toc_text = 1;

		for (i = 0; i < doc->numpages;i++) {
			if (useful_page_labels) {
				if (doc->pageorder == DESCEND) {
					j = (doc->numpages - 1) - i;} 
				else {
					j = i;}
				this_page=atoi(doc->pages[j].label);
				if(last_page>this_page) {
					num_parts++;}
				if (num_parts<10) pages_in_part[num_parts]++;
				last_page=this_page;} 
		}
		page->filename = 0;
		
		Lock();
		for ( i = 1; i <= doc->numpages; i++) {
			 j = doc->numpages-i;
			 BGVPageItem *item= new BGVPageItem(&doc->pages[j]);
			 marklist->AddItem(item,0);}
		Unlock();
    }
  else {
		toc_length = 0;
		toc_entry_length = 3;
		page->filename=filename;
		Lock();
		BGVPageItem *item;
		struct page *ref = (struct page *) malloc(sizeof(struct page));
		page->filename=filename;
		ref->label=new char[strlen("EPS")+1];
		if (doc && doc->epsf) strcpy(ref->label,"EPS");
		else strcpy (ref->label,"1");
		item= new BGVPageItem(ref);
		marklist->AddItem(item,0);
		Unlock();
    }
	if (doc) {
		if (num_parts>10 || (unsigned int)num_parts==doc->numpages) {
			num_parts=0;
			pages_in_part[0]=doc->numpages;}		
	}
	
	set_pagemedia(pagemedia);
		
	current_page=0;

	if (doc && doc->pdf) m_file->ItemAt(2)->SetEnabled(false);
	else m_file->ItemAt(2)->SetEnabled(true);
	if (doc && !doc->pdf) m_file->ItemAt(3)->SetEnabled(true);
	else m_file->ItemAt(3)->SetEnabled(false);
	m_file->ItemAt(5)->SetEnabled(true);
	m_view->ItemAt(2)->SetEnabled(true);
	Lock();
	m_go->ItemAt(5)->SetEnabled(true);
	m_pagemarks->ItemAt(0)->SetEnabled(true);
	m_pagemarks->ItemAt(1)->SetEnabled(true);
	m_pagemarks->ItemAt(2)->SetEnabled(true);
	m_pagemarks->ItemAt(3)->SetEnabled(true);
	m_pagemarks->ItemAt(5)->SetEnabled(true);
	m_pagemarks->ItemAt(6)->SetEnabled(true);
	
	toolBar->ItemAt(1)->SetEnabled(true);
	toolBar->ItemAt(3)->SetEnabled(true);
	toolBar->ItemAt(4)->SetEnabled(true);
	toolBar->ItemAt(7)->SetEnabled(true);
	Unlock();
}


void BGhostview::new_file( int number )
{
 	updateRecentList();
	if (page->isInterpreterRunning()) page->disableInterpreter();	
  setup();

  if (toc_text) {
		if ((unsigned int)number >= doc->numpages) number = doc->numpages - 1;
		if (number < 0) number = 0;
	};
}

void BGhostview::set_pagemedia(int number) {
		if (number==0) page->pagemedia=NULL;
		else page->pagemedia=papersizes[number-1].name;
 		if (current_page>=0) show_page(current_page);
};

void BGhostview::set_magstep(int i)
{
	int newMagstep=0;
	switch (i) {
	case 0:newMagstep=32; break;
	case 1:newMagstep=45; break;
	case 2:newMagstep=55; break;
	case 3:newMagstep=63; break;
	case 4:newMagstep=71; break;
	case 5:newMagstep=77; break;
	case 6:newMagstep=84; break;
	case 7:newMagstep=89; break;
	case 8:newMagstep=95; break;
	case 9:newMagstep=100; break;
	case 10:newMagstep=112; break;
	case 11:newMagstep=122; break;
	case 12:newMagstep=132; break;
	case 13:newMagstep=141; break;
	case 14:newMagstep=158; break;
	case 15:newMagstep=173; break;
	case 16:newMagstep=200; break;
	};
 	magstep=i;
	magBar->Menu()->ItemAt(magstep)->SetMarked(true);
	m_view->ItemAt(0)->SetEnabled(magstep<16);
	m_view->ItemAt(1)->SetEnabled(magstep>0);
	page->xdpi=newMagstep*75.0/100;
	page->ydpi=newMagstep*75.0/100;
	if (current_page>=0) show_page(current_page); 
}

void BGhostview::show_page(int number)
{
	if (number==-1) return;
  Lock();
  BGVPageItem *item = (BGVPageItem *) marklist->ItemAt(current_page);
  if (item) {
  	item->showing=false;
   	marklist->InvalidateItem(current_page);};
  if (toc_text) {
		if (number >= doc->numpages) number = doc->numpages - 1;
		if (number < 0) number = 0;	
		m_go->ItemAt(1)->SetEnabled(number+1<doc->numpages);
		m_go->ItemAt(4)->SetEnabled(number+1<doc->numpages);
		toolBar->ItemAt(8)->SetEnabled(number+1<doc->numpages);
		toolBar->ItemAt(9)->SetEnabled(number+1<doc->numpages);
		m_go->ItemAt(0)->SetEnabled(number>0);
		m_go->ItemAt(3)->SetEnabled(number>0);
		toolBar->ItemAt(5)->SetEnabled(number>0);
		toolBar->ItemAt(6)->SetEnabled(number>0);
		current_page = number;}
	else { 
		m_go->ItemAt(1)->SetEnabled(false);
		m_go->ItemAt(4)->SetEnabled(false);
		toolBar->ItemAt(8)->SetEnabled(false);
		toolBar->ItemAt(9)->SetEnabled(false);
		m_go->ItemAt(0)->SetEnabled(false);
		m_go->ItemAt(3)->SetEnabled(false);
		toolBar->ItemAt(5)->SetEnabled(false);
		toolBar->ItemAt(6)->SetEnabled(false);} 
  item = (BGVPageItem *) marklist->ItemAt(current_page);
  if (item) {
  	item->showing=true;
  	marklist->InvalidateItem(current_page);};
  Unlock();
        
	if (page->isInterpreterRunning()) page->disableInterpreter();
  if (toc_text) {
		page->sendPS(psfile, doc->beginheader, doc->lenheader, false);
		page->sendPS(psfile, doc->beginprolog, doc->lenprolog, false);
		page->sendPS(psfile, doc->beginsetup, doc->lensetup, false);
		page->sendPS(psfile, doc->pages[current_page].begin, doc->pages[current_page].len, false);
		page->enableInterpreter();
  } 
  else 
	  page->enableInterpreter();  
}







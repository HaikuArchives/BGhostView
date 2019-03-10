/****************************************************************************
**
** Name       : BGVMain
** Author     : Andreas Raquet
** Copyright  : (C) 8/23/1998 by Andreas Raquet
** Description: BApplication class of BGhostview
**
** This code is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include "BGVMain.h"


int main(int argc, char **argv) {
	new BGVApplication();
	return(0);
};

void BGVApplication::ArgvReceived(int32 argc, char **argv) {
	if (argc > 1) {
		char *filename = argv[1];
		BEntry entry(filename);
		BPath path;
		entry.GetPath(&path);
		startup->openFile(path.Path());
		}
};

void BGVApplication::RefsReceived(BMessage *message) {
  startup->MessageReceived(message);	
  }; 


void BGVApplication::MessageReceived(BMessage *message) {
	if (message->what==BGV_SAVE_ALL_REFS 
			|| message->what==BGV_SAVE_MARKED_REFS)
		startup->MessageReceived(message);
	else 
		BApplication::MessageReceived(message);
};
	
BGVApplication::BGVApplication() 
	:BApplication(signature){
	BRect frame(100,50,580,550);
	Preferences prefs("x-vnd.raquet-bghostview");
	if (prefs.InitCheck()==0) { 
		PreferenceSet set(prefs,"BGhostview",true);
		if (set.InitCheck()==0) {
			const void* data;
			ssize_t size;
			uint32 type='    ';
			if (!set.GetData("Bounds",data,size,type)) 
				memcpy(&frame,data,sizeof(frame));
		}
	}
	startup = new BGhostview(frame,"BGhostview");
	Run();
	};	


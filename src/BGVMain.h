/***********************************************
 **  BGhostView - v0.8.1 - 5/7/98, Andreas Raquet **
 ***********************************************/

#ifndef BGV_MAIN_H
#define BGV_MAIN_H 

#include <Be.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "BGhostview.h"

#define signature "application/x-vnd.raquet-bghostview"

class BGVApplication : public BApplication {
private:
	BGhostview *startup;
public:
	BGVApplication();
	virtual void ArgvReceived(int32 argc, char **argv);
	virtual void MessageReceived(BMessage *message);
	virtual void RefsReceived(BMessage *message);
};


#endif

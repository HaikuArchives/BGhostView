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
	if (argc>0)	{
		char *filename = new char[B_FILE_NAME_LENGTH];
		new BGVApplication(argv[1]);}
	else new BGVApplication(NULL);
	return(0);
};

BGVApplication::BGVApplication(char* filename) 
	:BApplication(signature){
	BGhostview *startup = new BGhostview(BRect(100,50,580,550),"BGhostview");
	if (filename!=NULL)
		startup->openFile(filename);
	Run();
	};	


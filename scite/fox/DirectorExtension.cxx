/** FXSciTE - FXScintilla based Text Editor
 *
 *  DirectorExtension.cxx - Extension for communicating with a director program
 *
 *  Copyright 2001-2002 by Gilles Filippini <gilles.filippini@free.fr>
 *
 *  Adapted from the SciTE GTK source DirectorExtension.cxx 
 *  Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
 *
 *  ====================================================================
 *
 *  This file is part of FXSciTE.
 * 
 *  FXSciTE is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  FXSciTE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FXSciTE; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <fox/fx.h>

#include "Platform.h"

#include "PropSet.h"

#include "Scintilla.h"
#include "Accessor.h"
#include "Extender.h"
#include "DirectorExtension.h"
#include "SciTE.h"
#include "SciTEBase.h"


static ExtensionAPI *host = 0;
static DirectorExtension *pde = 0;
static int fdDirector = 0;
static int fdCorrespondent = 0;
static bool startedByDirector = false;
//static FILE *fdDebug = 0;

static bool SendPipeCommand(const char *pipeCommand, int fdOutPipe) {
	//check that there is actually a pipe
	int size = 0;

	if (fdOutPipe != -1) {
		size = write(fdOutPipe, pipeCommand, strlen(pipeCommand) + 1);
		//fprintf(fdDebug, "dd: Send pipecommand: %s %d bytes\n", pipeCommand, size);
		if (size != -1)
			return true;
	}
	return false;
}

long DirectorExtension::onPipeSignal(FXObject * sender, FXSelector sel, void * ptr){
	int readLength;
	char pipeData[8192];
	PropSetFile pipeProps;
	
	SString pipeString;
	while ((readLength = read(fdReceiver, pipeData, sizeof(pipeData))) > 0) {
		pipeString.append(pipeData);
	}
	HandleStringMessage(pipeString.c_str());
	return 1;
}

static void SendDirector(const char *verb, const char *arg = 0) {
	//fprintf(fdDebug, "SendDirector:(%s, %s):  fdDirector = %d fdCorrespondent = %d\n", verb, arg, fdDirector, fdCorrespondent);
	if ((fdDirector != 0) || (fdCorrespondent != 0)) {
		int fdDestination = 0;
		if( fdDirector != 0 )
			fdDestination = fdDirector;
		else
			fdDestination = fdCorrespondent;
		
		SString addressedMessage;
		addressedMessage += verb;
		addressedMessage += ":";
		if (arg)
			addressedMessage += arg;
		//fprintf(fdDebug, "SendDirector: SendPipeCommand: >%s< to %d\n", addressedMessage.c_str(), fdDestination);
		//send the message through the existing pipe
		::SendPipeCommand(addressedMessage.c_str(), fdDestination);
	}
	else{
		//fprintf(fdDebug, "SendDirector: fdDirector & fdCorrespondent == 0\n");
	}
}

static void CheckEnvironment(ExtensionAPI *host) {
	if (!host)
		return ;
	if (!fdDirector) {
		char *director = host->Property("ipc.director.name");
		if (director && *director) {
			startedByDirector = true;
			fdDirector = open(director, O_RDWR | O_NONBLOCK);
		}
		delete []director;
	}
}

FXDEFMAP(DirectorExtension) DirectorExtensionMap[]={
  FXMAPFUNC(SEL_IO_READ, DirectorExtension::ID_PIPESIGNAL, DirectorExtension::onPipeSignal),
};

FXIMPLEMENT(DirectorExtension,FXObject,DirectorExtensionMap,ARRAYNUMBER(DirectorExtensionMap))

DirectorExtension::DirectorExtension() : fdReceiver(0) {
	pde = this;
}

DirectorExtension::~DirectorExtension() {
	pde = 0;
}

bool DirectorExtension::Initialise(ExtensionAPI *host_) {
	host = host_;
	CheckEnvironment(host);
	//fdDebug = fopen("/tmp/SciTE.log", "w");
	if( startedByDirector ){
		CreatePipe();
		//fprintf(fdDebug, "Initialise: fdReceiver: %d\n", fdReceiver);
		if (!fdReceiver)
			::exit(FALSE);
	}
	return true;
}

bool DirectorExtension::Finalise() {
	::SendDirector("closing");
	fdReceiver = 0;
	return true;
}

bool DirectorExtension::Clear() {
	return true;
}

bool DirectorExtension::Load(const char *) {
	return true;
}

bool DirectorExtension::OnOpen(const char *path) {
	CheckEnvironment(host);
	if (*path) {
		::SendDirector("opened", path);
	}
	return true;
}

bool DirectorExtension::OnSwitchFile(const char *path) {
	CheckEnvironment(host);
	if (*path) {
		::SendDirector("switched", path);
	}
	return true;
};

bool DirectorExtension::OnSave(const char *path) {
	CheckEnvironment(host);
	if (*path) {
		::SendDirector("saved", path);
	}
	return true;
}

bool DirectorExtension::OnChar(char) {
	return false;
}

bool DirectorExtension::OnExecute(const char *) {
	return false;
}

bool DirectorExtension::OnSavePointReached() {
	return false;
}

bool DirectorExtension::OnSavePointLeft() {
	return false;
}

bool DirectorExtension::OnStyle(unsigned int, int, int, Accessor *) {
	return false;
}

// These should probably have arguments

bool DirectorExtension::OnDoubleClick() {
	return false;
}

bool DirectorExtension::OnUpdateUI() {
	return false;
}

bool DirectorExtension::OnMarginClick() {
	return false;
}

bool DirectorExtension::OnMacro(const char *command, const char *params) {
	SendDirector(command, params);
	return true;
}

bool DirectorExtension::SendProperty(const char *prop) {
	CheckEnvironment(host);
	if (*prop) {
		::SendDirector("property", prop);
	}
	return true;
}

void DirectorExtension::HandleStringMessage(const char *message) {
	// Message may contain multiple commands separated by '\n'
	// Reentrance trouble - if this function is reentered, the fdCorrespondent may
	// be set to zero before time.
	WordList wlMessage(true);
	wlMessage.Set(message);
	//fprintf(fdDebug, "HandleStringMessage: got %s\n", message);
	for (int i = 0; i < wlMessage.len; i++) {
		// Message format is [:return address:]command:argument
		char *cmd = wlMessage[i];
		if (isprefix(cmd, "closing:")) {
			fdDirector = 0;
			if (startedByDirector)
				host->ShutDown();
		} else if (host) {
			host->Perform(cmd);
		}
		fdCorrespondent = 0;
	}
}

bool DirectorExtension::CreatePipe(bool forceNew) {

	bool anotherPipe = false;
	bool tryStandardPipeCreation = false;
	SString pipeFilename = host->Property("ipc.scite.name");
	char* pipeName = NULL;
	fdReceiver = -1;

	//check we have been given a specific pipe name
	if (pipeFilename.size() > 0)
	{
		printf("CreatePipe: if (pipeFilename.size() > 0): %s\n", pipeFilename.c_str());
		//snprintf(pipeName, CHAR_MAX - 1, "%s", pipeFilename.c_str());
		pipeName = strdup(pipeFilename.c_str());
		fdReceiver = open(pipeName, O_RDWR | O_NONBLOCK);
		if (fdReceiver == -1 && errno == EACCES) {
			//fprintf(fdDebug, "CreatePipe: No access\n");
			tryStandardPipeCreation = true;
		}
		//there isn't one - so create one
		else if (fdReceiver == -1) {
			SString fdReceiverString;
			//fprintf(fdDebug, "CreatePipe: Non found - making\n");
//WB++
#ifndef __vms
			mkfifo(pipeName, 0777);
#else           // no mkfifo on OpenVMS!
			creat(pipeName, 0777);
#endif
//WB--
			fdReceiver = open(pipeName, O_RDWR | O_NONBLOCK);
			
			fdReceiverString = fdReceiver;
			host->SetProperty("ipc.scite.fdpipe",fdReceiverString.c_str());
			tryStandardPipeCreation = false;
		}
		else
		{
			//fprintf(fdDebug, "CreatePipe: Another one there - opening\n");

			fdReceiver = open(pipeName, O_RDWR | O_NONBLOCK);

			//there is already another pipe so set it to true for the return value
			anotherPipe = true;
			//I don;t think it is a good idea to be able to listen to our own pipes (yet) so just return
			//break;
			return anotherPipe;
		}
	}
	else
	{
		tryStandardPipeCreation = true;
	}
	
	if( tryStandardPipeCreation )
	{
		//possible bug here (eventually), can't have more than a 1000 SciTE's open - ajkc 20001112
		for (int i = 0; i < 1000; i++) {
	
			//create the pipe name - we use a number as well just incase multiple people have pipes open
			//or we are forceing a new instance of scite (even if there is already one)
			sprintf(pipeName, "/tmp/.SciTE.%d.ipc", i);
	
			//fprintf(fdDebug, "Trying pipe %s\n", pipeName);
			//check to see if there is already one
			fdReceiver = open(pipeName, O_RDWR | O_NONBLOCK);
	
			//there is one but it isn't ours
			if (fdReceiver == -1 && errno == EACCES) {
				//fprintf(fdDebug, "No access\n");
				continue;
			}
			//there isn't one - so create one
			else if (fdReceiver == -1) {
				SString fdReceiverString;
				//fprintf(fdDebug, "Non found - making\n");
//WB++
#ifndef __vms
				mkfifo(pipeName, 0777);
#else           // no mkfifo on OpenVMS!
				creat(pipeName, 0777);
#endif
//WB--
				fdReceiver = open(pipeName, O_RDWR | O_NONBLOCK);
				//store the file descriptor of the pipe so we can write to it again. (mainly for the director interface)
				fdReceiverString = fdReceiver;
				host->SetProperty("ipc.scite.fdpipe",fdReceiverString.c_str());
				break;
			}
			//there is so just open it (and we don't want out own)
			else if (forceNew == false) {
				//fprintf(fdDebug, "Another one there - opening\n");
	
				fdReceiver = open(pipeName, O_RDWR | O_NONBLOCK);
	
				//there is already another pipe so set it to true for the return value
				anotherPipe = true;
				//I don;t think it is a good idea to be able to listen to our own pipes (yet) so just return
				//break;
				return anotherPipe;
			}
			//we must want another one
		}
	}

	if (fdReceiver != -1) {
		
		//store the inputwatcher so we can remove it.
		FXApp::instance()->addInput(fdReceiver, INPUT_READ, this, ID_PIPESIGNAL);
		return anotherPipe;
	}

	//we must have failed or something so there definately isn't "another pipe"
	return false;
}

#ifdef _MSC_VER
// Unreferenced inline functions are OK
#pragma warning(disable: 4514)
#endif
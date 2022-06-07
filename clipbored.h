#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>


//Postfix when creating a shared memory file
//for reducing any chance of conflicts
const char * shmPostfix = "_clipbored"; 

void ___millisecSleep(long millisec) {
	struct timespec ts;
	ts.tv_sec = millisec / 1000;
	ts.tv_nsec = (millisec % 1000) * 1000000;
	nanosleep(&ts, &ts);
}

char * ___LongToStr(long long input) {
	static char output[32] = "";
	sprintf(output, "%lld", input);
	return output;
}

//An over-glorified fork. But instead of cloning 
//it into another process, it clones into a daemon.
pid_t ___daemonFork() {
	pid_t sid, pid = fork();
	if (pid > 0) {
		signal(SIGCHLD, SIG_IGN);
		return pid;
	}

	sid = setsid();
	if (sid < 0) {
		return -1;
	}

	pid = fork();
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	return pid;
}

typedef struct {
	Display * display;
	int DefaultScreenIDnum;
	Atom TextEncoding;
	Atom clipSelection;
	Atom text_atom;
	Atom targets_atom;
	Atom xseldata;
} ___prereqUnix;
___prereqUnix * ___prut;
 //Acronym, Prerequisite Unix t...Forgot what 't' meant

typedef struct {
	bool stayAlive;
	bool clipOpen;
	pid_t ParentID; //technically not parent. Process ID of original process
	size_t size;
} ___CopyTextStruct;
___CopyTextStruct * ___CTS;

char ___SharedStringPID[23] = "/";



//Functions for initialising stuff
void ___X11prereqInit() {
	Display * display = XOpenDisplay(0);
	___prut = (___prereqUnix *) malloc(sizeof(___prereqUnix));
	___prut->display = display;
	___prut->DefaultScreenIDnum = DefaultScreen(display);
	___prut->TextEncoding = XInternAtom(display, "UTF8_STRING", 1);
	if (___prut->TextEncoding == None) {
		___prut->TextEncoding = XA_STRING;
	}
	___prut->clipSelection = XInternAtom(display, "CLIPBOARD", 0);
	___prut->text_atom = XInternAtom(display, "TEXT", 0);
	___prut->targets_atom = XInternAtom(display, "TARGETS", 0);
	___prut->xseldata = XInternAtom(display, "XSEL_DATA", 0);
}

//Initialising struct needed for clipboard stuff. 
//Separate from the function above as this ain't X11.
void ___clipInit(___CopyTextStruct ** CTS) {
	pid_t pid = getpid();
	strcat(___SharedStringPID, ___LongToStr(pid));
	strcat(___SharedStringPID, shmPostfix);
	strcat(___SharedStringPID, "_a");
	
	//Opening shared memory
	int shmFd = shm_open(___SharedStringPID, O_CREAT | O_RDWR, 0666);
	ftruncate(shmFd, sizeof(___CopyTextStruct));
	(*CTS) = mmap(0, sizeof(___CopyTextStruct), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

	(*CTS)->stayAlive = true;
	(*CTS)->ParentID = pid;

	___SharedStringPID[strlen(___SharedStringPID) - 1] = 'b';
}




//Paste from Clipboard
//Won't document. Not my code, I don't understand much of it myself.
char * XPaste() {
	Window window = XCreateSimpleWindow(___prut->display, RootWindow(___prut->display, ___prut->DefaultScreenIDnum), 0, 0, 1, 1, 0, BlackPixel(___prut->display, ___prut->DefaultScreenIDnum), WhitePixel(___prut->display, ___prut->DefaultScreenIDnum));

	XEvent event;
	size_t unreadBytes, size;
	int format;
	char * AddrHolder, *output = 0;

	Atom ClipDataType;
	XConvertSelection(___prut->display, ___prut->clipSelection, ___prut->TextEncoding, ___prut->xseldata, window, CurrentTime);
	XSync(___prut->display, 0);
	XNextEvent(___prut->display, &event);
	
	if ((event.type == SelectionNotify) && (event.xselection.selection == ___prut->clipSelection)) {
		if(event.xselection.property) {
			XGetWindowProperty(event.xselection.display, event.xselection.requestor, event.xselection.property, 0L, (~0L), 0, AnyPropertyType, &ClipDataType, &format, &size, &unreadBytes, (unsigned char **) &AddrHolder);

			if(ClipDataType == ___prut->TextEncoding) {
				//Want to make the output freeable using regular free().
				//XFree() is defined as Xfree() and Xfree is defined as
				//free() in libX11 at the time of writing this comment.

				//output = strndup(AddrHolder, size);
				//XFree(AddrHolder);
				output = AddrHolder;
			}
			XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
		}
	}
	XDestroyWindow(___prut->display, window);

	//Creates an empty string that is freeable. 
	//Creates when function fails. Avoiding reading 
	//garbage data from "output" being uninitialised
	if (!output) {
		output = (char*) calloc(1, sizeof(char)); 
	}

	return output;
}



//"Copy to Clipboard" portion

void * ___tellProcessToDie(void * input) {
	___CopyTextStruct * CTS = (___CopyTextStruct *) input;

	//checks if OG process is alive and clip is open. Else, it tells it to commit die.
	while (kill(CTS->ParentID, 0) == 0) {
		___millisecSleep(10000);
	}
	while (CTS->clipOpen) {
		___millisecSleep(5000);
	}
	___millisecSleep(50);
	CTS->stayAlive = false;
	return 0;
}


void ___ClipboardController() {
	//X11 stuff. Not my code. Won't be documented by me completely.

	//TODO: Actual signal handling for proper exit.
	//Still WIP. This is temporary, I hoping so. 
	//Comment this portion out if you don't like it 
	//Another program owning clipboard/copying would kill daemon
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	//A thread for deciding whether the program should kill itself
	pthread_t Euthaniser;
	pthread_create(&Euthaniser, NULL, ___tellProcessToDie, (void*)___CTS);

	Window window = XCreateSimpleWindow(___prut->display, RootWindow(___prut->display, ___prut->DefaultScreenIDnum), 0, 0, 1, 1, 0, BlackPixel(___prut->display, ___prut->DefaultScreenIDnum), WhitePixel(___prut->display, ___prut->DefaultScreenIDnum));
	XEvent event;

	while (___CTS->stayAlive) {
		//Confusing, I know. Waits till clipboard is open, other part 
		//is for breaking the loop when it will go through an exit step
		while (!(___CTS->clipOpen) && ___CTS->stayAlive) {___millisecSleep(40);}
		
		//read shared memory
		int ShM22 = shm_open(___SharedStringPID, O_RDONLY, 0666);
		char * AddrHolder = (char *) mmap(0, ___CTS->size, PROT_READ, MAP_SHARED, ShM22, 0);

		//Own the clipboard. Reown even if already owned
		XSetSelectionOwner(___prut->display, ___prut->clipSelection, window, CurrentTime);
	
		//"infinite" loop that waits tills and tells another program the clipboard data
		while (___CTS->clipOpen) {
			XNextEvent(___prut->display, &event);
			if ((event.type == SelectionRequest) && (event.xselectionrequest.selection == ___prut->clipSelection)) {
				XSelectionRequestEvent * xsr = &event.xselectionrequest;

				XSelectionEvent ev = {0};
				ev.type = SelectionNotify;
				ev.display = xsr->display;
				ev.requestor = xsr->requestor;
				ev.selection = xsr->selection;
				ev.time = xsr->time;
				ev.target = xsr->target;
				ev.property = xsr->property;

				if (ev.target == ___prut->targets_atom) {
					XChangeProperty(ev.display, ev.requestor, ev.property, XA_ATOM, 32, PropModeReplace, (unsigned char*) &(___prut->TextEncoding), 1);
				}
				else if (ev.target == ___prut->TextEncoding) {
					XChangeProperty(ev.display, ev.requestor, ev.property, ___prut->TextEncoding, 8, PropModeReplace, (const unsigned char *) AddrHolder, ___CTS->size);
				}
				else if (ev.target == ___prut->text_atom) {
					XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING, 8, PropModeReplace, (const unsigned char *) AddrHolder, ___CTS->size);
				}
				else { 
					ev.property = None;
				}
				XSendEvent(___prut->display, ev.requestor, 0, 0, (XEvent*) &ev);
			}
			if (event.type == SelectionClear) {
				___CTS->clipOpen = false;
			}
		}
		//clipboard is being closed. Stopping shared memory
		munmap(AddrHolder, ___CTS->size);
		shm_unlink(___SharedStringPID);
	}
	___SharedStringPID[strlen(___SharedStringPID) - 1] = 'a';
	munmap((void*)___CTS, sizeof(___CopyTextStruct));
	shm_unlink(___SharedStringPID);
	XDestroyWindow(___prut->display, window);
	return;
}

void XCopy(char * input) {
	//avoiding global variables. Still bad code?
	static void * SharedMemLoc;
	
	if(!(___CTS->stayAlive)) {
		//puts("Error: Clipboard managing daemon isn't running. Restart.")
		return;
	}

	//If clipboard is open, close it and destroy data
	if(___CTS->clipOpen) {
		___CTS->clipOpen = false;
		___millisecSleep(20);
		munmap(SharedMemLoc, ___CTS->size);
		___millisecSleep(45);
	}
	
	___CTS->size = strlen(input);

	//Opening shared memory
	int shmFd = shm_open(___SharedStringPID, O_CREAT | O_RDWR, 0666);
	ftruncate(shmFd, ___CTS->size);
	SharedMemLoc = mmap(0, ___CTS->size, PROT_WRITE, MAP_SHARED, shmFd, 0);
	
	//copying and opening clipboard
	memcpy(SharedMemLoc, (void*)input, ___CTS->size);
	___CTS->clipOpen = true;
	___millisecSleep(10);
}

#define clipBoredInit(enableCopying) {\
	if (enableCopying) {\
		___clipInit(&___CTS);\
		pid_t ___child = ___daemonFork();\
		if (___child <= 0) {\
			if (___child < 0) {return ___child;}\
			___X11prereqInit();\
			___ClipboardController();\
			return 0;\
		}\
	}\
	___X11prereqInit();\
}

//returning main with ___child for exitting with error code

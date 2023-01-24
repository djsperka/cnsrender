#include <popt.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <sstream>
#include "transport.h"
#include "actions.h"
#include "RMessage.h"
#include "ncurses.h"
#include "RCommands.h"

using namespace std;
using namespace render;

int args(int argc, char **argv);
int commands(vector<RMessage>& cmds);
int load(char *file, vector<RMessage>& cmds);
int check_ping_response();
void drawscreen();
const char *sendcommand();
int navstep(int navstep);
unsigned char getCommandID();
const char *getcommandstr(int i);

char *f_pHostIP = NULL;
char *f_pInputFile = NULL;
int f_iPort = 2000;
int f_iDump = 0;
int f_iXhack = 0;
vector<RMessage> f_cmds;
int f_topAt;		// which index into f_cmds corresponds to line 0
int f_navAt; 		// current navigation position (line) in window [0...LINES-1]

int args(int argc, char **argv)
{
	int status=0;
	int value;

	struct poptOption optionsTable[] =
	{
		{"render-host", 'r', POPT_ARG_STRING, &f_pHostIP, 0, "Render host ip", "<ip:port>"},
		{"input-file", 'i', POPT_ARG_STRING, &f_pInputFile, 0, "Input filename", "<filename>"},
		{"port", 'p', POPT_ARG_INT, &f_iPort, 0, "Render host port", "[2000]"},
		{"dump-commands", 'd', POPT_ARG_NONE, &f_iDump, 0, "dump commands", ""},
		{"xhack", 'x', POPT_ARG_NONE, &f_iXhack, 0, "Super hack", ""},
		POPT_AUTOHELP
		POPT_TABLEEND
	};
	poptContext optcon;

	optcon = poptGetContext(NULL, argc, (const char **)argv, optionsTable, 0);

	if (argc<2)
	{
		poptPrintUsage(optcon, stderr, 0);
		status = 1;
	}
	else
	{
		value = poptGetNextOpt(optcon);

		if (value== -1)
		{

			if (f_pHostIP)
			{
				// Attempt to open socket and ping
				cerr << "using render host:ip " << f_pHostIP << ":" << f_iPort << endl;

				// parse and establish connection
				if ((status = init_tcpip(NULL, f_pHostIP, f_iPort, 1)))
				{
					cerr << "Error " << status << " connecting to render host." << endl;
					return -1;
				}
				else
				{
					render_send_ping();
					if ((status = check_ping_response()))
					{
						cerr << "Error " << status << " in ping to host." << endl;
						return -1;
					}
				}
			}
			else
			{
				cerr << "no hostIP:port specified (-r)" << endl;
				return -1;
			}

			if (!f_pInputFile)
			{
				cerr << "No input file specified on command line." << endl;
				return -1;
			}
		}
		else
		{
			cerr << "Unrecognized command line arg.";
			return -1;
		}
	}

	return status;
}

int load(char *filename, vector<RMessage>& cmds)
{
	MessageStruct msg;
	char* m((char *)&msg);
	ifstream f;
	int l;
	unsigned int spos;
	f.open(filename, ifstream::in);
	cerr << "Loading input file " << filename << endl;
	if (!f.good())
	{
		cerr << "Cannot open input file " << filename << endl;
		return -1;
	}

	while (f.good())
	{
		spos = f.tellg();
		l = -1;
		f.read(m, 2);
		if (f.good())
		{
			l = AS_AN_INT(m[1]);
			if (f_iXhack && m[0]==0x40) l = 2;// temp super hack
			f.read(m+2, l);
			if (f.good())
			{
				cmds.push_back(msg);
			}
		}
		if (f_iDump)
			cout << "pos " << spos << "(" << std::hex << spos << ") cmd " << std::hex << (unsigned int)m[0] << " l " << std::dec << l << endl;
	}
	f.close();

	return 0;
}

int commands(vector<RMessage>&)
{
	int c;
	int redraw=0;

	// Initialize screen
	initscr();
	cbreak();	// get single char entry
	noecho();	// do not automatically print typed characters
	keypad(stdscr, TRUE);

	drawscreen();

	while(1)
	{
		redraw = 0;
		c = getch();
		switch(c)
		{
		case KEY_UP:
		{
			navstep(-1);
			redraw = 1;
			break;
		}
		case KEY_DOWN:
		{
			navstep(1);
			redraw = 1;
			break;
		}
		case 'f':
		{
			bool bStop = false;
			do
			{
				bStop = (getCommandID() == cmdFRAME || getCommandID() == cmdRENDER);
				sendcommand();
			} while (1 == navstep(1) && !bStop);
			redraw = 1;
			break;
		}
		case 10:
		case 'r':
		{
			sendcommand();
			navstep(1);
			redraw = 1;
			break;
		}
		}
		if (c == 'q') break;
		if (redraw) drawscreen();
	}
	endwin();
	return 0;
}

unsigned char getCommandID()
{
	return f_cmds.at(f_navAt + f_topAt).getHeader().commandID;
}


// step navigation index by n steps (can be negative). If it takes us off the screen,
// then scroll screen. Return value is the change in command position.
int navstep(int n)
{
	int inow = f_topAt+f_navAt;
	if (n>0)
	{
		if(f_navAt+n < LINES)
		{
			f_navAt += n;
		}
		else
		{
			int d = n - (LINES - 1 - f_navAt);
			f_navAt = LINES-1;
			f_topAt += d;
		}
	}
	else if (n<0)
	{
		if(f_navAt+n >= 0)
		{
			f_navAt += n;
		}
		else
		{
			int d = f_navAt + n;
			f_topAt += d;
			if (f_topAt < 0) f_topAt = 0;
			f_navAt = 0;
		}
	}
	return f_navAt+f_topAt-inow;
}

const char *sendcommand()
{
	msg_send((char *)&f_cmds[f_topAt+f_navAt]);
	return (char *)NULL;
}

const char *getcommandstr(int i)
{
	static string s;
	ostringstream oss;
	oss.clear();
	if (i < (int)f_cmds.size())
	{
		oss << " " << f_cmds.at(i);
	}
	else
	{
		oss << "Z";
	}
	s = oss.str();
	return s.c_str();
}

void drawscreen()
{
	for (int line=0; line<LINES; line++)
	{
		if (line != f_navAt)
		{
			mvaddstr(line, 1, getcommandstr(f_topAt + line));
		}
		else
		{
			attron(A_STANDOUT);
			mvaddstr(line, 1, getcommandstr(f_topAt + line));
			attroff(A_STANDOUT);
		}
	}

	// Move cursor
	move(f_navAt, 1);
}


int main(int argc, char **argv)
{

	// Process input args
	if (args(argc, argv))
	{
		cerr << "Error on command line." << endl;
		return -1;
	}

	if (load(f_pInputFile, f_cmds))
	{
		cerr << "Error loading input file" << endl;
		return -1;
	}

	f_topAt = 0;
	if (!f_iDump)
		commands(f_cmds);

	return 0;
}

int check_ping_response()
{
	int status = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000L;

	if (!render_check_for_ping())
	{
		nanosleep(&ts, NULL);
		if (!render_check_for_ping())
		{
			status = -1;
		}
	}
	return status;
}

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
//#include <X11/keysym.h>
#include <X11/XKBlib.h>

// std headers
#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <fstream>
#include <csignal>
#include <pthread.h>
#include <sys/mman.h>
#include <algorithm>
#include <iterator>
#include <exception>
#include <iostream>
//#include <array>

// djs util headers
#include "RLog.h"
#include "StopWatch.h"
#include "RMarkers.h"
#include "RBoundedBuffer.h"
#include "AsyncTcpServer.h"

// boost
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

// rendio header
#include "rendio.h"

// helper functions
#include "viewports.h"

// ren3d headers
#include "RWorld.h"
#include "RWindow.h"
#include "FrameMessageHandler.h"
#include "ResetMessageHandler.h"
#include "StartMessageHandler.h"
#include "GroundPlaneFixture.h"
#include "DotFixture.h"
#include "TestMessageHandler.h"
#include "SocketMessageQueue.h"
#include "DummyMessageQueue.h"
#include "FileMessageQueue.h"
#include "FlashMessageQueue.h"
#include "RQuitMessageHandler.h"
#include "RJSMessageHandler.h"
#include "FrameMessageHandler.h"
#include "GrabMessageHandler.h"
#include "RBackgroundMessageHandler.h"
#include "RUpdateMessageHandler.h"
#include "RPerspectiveMessageHandler.h"
#include "RReleaseMessageHandler.h"
#include "CameraBeaconFixture.h"
#include "PathMessageHandler.h"
#include "MoveObjectMessageHandler.h"
#include "LissajousMessageHandler.h"
#include "FF2DFixture.h"
#include "DotFixture.h"
#include "RectFixture.h"
#include "DotFieldFixture.h"
#include "GratingMessageHandler.h"
#include "DonutMessageHandler.h"
#include "XHairMessageHandler.h"
#include "GridFixture.h"
#include "CalibrationFixture.h"
#include "TestAnimMessageHandler.h"
#include "GaussianDotFixture.h"
#include "TargetTransformFixture.h"
#include "OuterSpaceFixture.h"
#include "PursuitTransformFixture.h"
#include "RTriggerMessageHandler.h"
#include "RCommandListMessageHandler.h"
#include "ROnOffMessageHandler.h"
#include "RScriptMessageHandler.h"
#include "UnitsMessageHandler.h"
#include "PlaidFixture.h"
#include "ConstantDensityPlaneMessageHandler.h"
#ifdef USE_GPIB
#include "GpibMessageQueue.h"
#endif
#include "make_msg.h"

// opensg stuff
#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGThread.h>
#include <OpenSG/OSGXWindow.h>
#include "OSGRenderXWindow.h"
#include "OSGRenderFileGrabForeground.h"
#include <OpenSG/OSGPassiveBackground.h>
#include "DummyFixture.h"
// GLFW3
#include <GLFW/glfw3.h>

#ifdef USING_PARAPIN
// parallel port lib
#include "parapin.h"
#endif

using namespace std;
using namespace render;
using namespace render::util;
using namespace boost;
OSG_USING_NAMESPACE

// Key globals
RWorldP f_worldP;
RMessageQueueP f_msgqP;
RenderXWindowPtr f_window;
SolidBackgroundPtr f_backgroundPtr;
bool f_bAbortQuit = false;

RBoundedBuffer<OpFunc> f_opfuncBuffer(1000, false);		// for function objects - does not block on pop() when empty, returns false instead
boost::array<char, 4096> f_jsBuffer;	// temp buffer for reading async tcp data
std::ostringstream f_ossJSBytes;		// JS is copied here as it is read
size_t f_jsBytesWritten;				// number of bytes actually written to ossJSBytes
size_t f_jsNumBytes;					// number of bytes expected for js
//boost::asio::io_service f_ioservice;
//boost::asio::ip::tcp::endpoint f_tcp_endpoint{boost::asio::ip::tcp::v4(), 1999};
//boost::asio::ip::tcp::acceptor f_tcp_acceptor{f_ioservice, f_tcp_endpoint};
//boost::asio::ip::tcp::socket f_tcp_socket{f_ioservice};
//boost::asio::streambuf f_streambuf;


// Misc handlers needed here. Also map of handlers.
FrameMessageHandler *f_pFrameHandler = NULL;

// file global for vblank callback
PFNGLXGETVIDEOSYNCSGIPROC f_glXGetVideoSync;
PFNGLXWAITVIDEOSYNCSGIPROC f_glXWaitVideoSync;
unsigned int f_uiCount;

// file-globals used with command line arg parsing.
bool f_statusmsgs = false;
bool f_framerate = false;
bool f_bGenerateTiming = false;
bool f_bGenerateTimingStopped = false;

// Input flag for using orthographic-left/right viewports
bool f_orthoLeftRight = false;

// input flags re: file grabbing
string f_sGrabberBasename;
bool f_bGrab = false;
bool f_bGrabAll = false;

// input flag re: saving command stream
bool f_bSaveCommands = false;
bool f_bSaveCommandsStopped = false;
std::ofstream f_commandstream;

// input flag re: display calibration screen
bool f_bCalibrationStartup = false;


// input flags re: logging
int f_console_loglevel = -1;
int f_file_loglevel = -1;
int f_syslog_loglevel = -1;
bool f_dumpcommands = false;

// json input
short unsigned int f_jsport = 0;

// Input flags re: input queue
short unsigned int f_port=0;
bool f_bHaveQueue=false;
int f_queuesize = 25;
std::string f_inputFilename;
bool f_bHaveInputFilename = false;

// input flag for rendio
std::string f_sDioDevice;
bool f_bDioDevice = false;
int f_fdRendio = 0;			// file handle

// Input flags re: window size
int f_fullscreen = 0;
uint16_t f_width = 800;
uint16_t f_height = 600;
std::string f_sScreens;

// input flag re: aspect ratio
double f_aspect = 1.0;

// input flag re: scheduling prio
bool f_bNormalPriority = false;

/* Struct to transfer parameters to threads */
struct thread_param {
	int prio;
	unsigned long interval;		// usecs
	int threadstarted;			// 1 when started, 0 when stopped
	pthread_t thread;
};

struct thread_param input_thread_param = {99, 500, 0, 0};
struct thread_param inputjs_thread_param = {99, 500, 0, 0};
struct thread_param window_thread_param = { 99, 1000000, 0, 0};
struct thread_param command_thread_param = { 99, 500, 0, 0};		// only prio is used.
struct thread_param rendio_thread_param = { 97, 500, 0, 0};		// only prio is used.

#define USEC_PER_SEC		1000000
#define NSEC_PER_SEC		1000000000

// OSG and render initialization
int initRender();

// screen or other types of logging
void initLogging();

void on_geometry(const std::string&);
void on_grab(const std::string&);
void on_dio(const std::string& d);
void conflicting_options(const boost::program_options::variables_map & vm,
                         const std::string & opt1, const std::string & opt2, const std::string& opt3);
void conflicting_options(const boost::program_options::variables_map & vm,
                         const std::string & opt1, const std::string & opt2);

// Analyzes command line args
bool args(int argc, char **argv);

// thread function for input queue
void *input_thread(void *param);

// thread function for rendio
void *rendio_thread(void *param);

// thread function for json input queue
void *inputjs_thread(void *param);

// thread function for window
void *window_thread(void *param);

// helper function for window thread
void windowEvents();

// GLFW replacement for keyboard callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// GLFW error callback
void error_callback(int error, const char* description);

// interrupt handler
void sigint_handler (int signum);

// convenience for generating filenames
std::string getFilenameWithTimestamp(const std::string& extension);

void doJSQueue();
void doOldQueue();

// for js input thread
void inputjs_thread_read_request_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
void inputjs_thread_read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
void inputjs_thread_accept_handler(const boost::system::error_code &ec);
void inputjs_thread_parse();

// for adding message handlers
void addOrthographicMessageHandlers();
void addGenericMessageHandlers();
void addPerspectiveMessageHandlers();

void on_geometry(const std::string& sGeometry)
{
	istringstream iss(sGeometry);
	iss >> f_width;
	if (!iss)
		throw invalid_argument(string("Cannot parse WxH: ") + sGeometry);
	iss.ignore(1);
	iss >> f_height;
	if (!iss)
		throw invalid_argument(string("Cannot parse WxH: ") + sGeometry);
}

void on_dio(const std::string&)
{
	f_bDioDevice = true;
}

void on_grab(const std::string&)
{
	f_bGrab = true;
}

void conflicting_options(const boost::program_options::variables_map & vm,
                         const std::string & opt1, const std::string & opt2, const std::string& opt3)
{
	conflicting_options(vm, opt1, opt2);
	conflicting_options(vm, opt1, opt3);
	conflicting_options(vm, opt2, opt3);
}


void conflicting_options(const boost::program_options::variables_map & vm,
                         const std::string & opt1, const std::string & opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() &&
        vm.count(opt2) && !vm[opt2].defaulted())
    {
        throw std::logic_error(std::string("Conflicting options '") +
                               opt1 + "' and '" + opt2 + "'.");
    }
}


namespace po = boost::program_options;

bool args(int argc, char **argv)
{
	std::string sGeometry;

    try
    {
        po::options_description desc("Program Usage", 1024, 512);
        desc.add_options()
          ("help,h",     "produce help message")
          ("console-loglevel,l", po::value<int>(&f_console_loglevel)->default_value(0), "Set log output threshhold for console. Default=no console logging.")
          ("syslog-loglevel,S", po::value<int>(&f_syslog_loglevel)->default_value(0), "Set log output threshhold for syslog. Default=no syslog logging.")
          ("file-loglevel,L", po::value<int>(&f_file_loglevel)->default_value(0), "Set log output threshhold for log file (render.log). Default=no file logging.")
		  ("verbose-command-dump,V", po::bool_switch(&f_dumpcommands)->default_value(false), "Verbose command dump - commands are logged prior to being handled.")
		  ("port,p", po::value<short unsigned int>(&f_port)->default_value(0), "Listen port for binary messages.")
		  ("jsport,j", po::value<short unsigned int>(&f_jsport)->default_value(0), "Listen port for JSON messages.")	// mut exclusive 'j', 'p'
		  ("ortho-left-right,o", po::value<bool>(&f_orthoLeftRight)->default_value(false), "Use orthographic-only left/right viewports.")
		  ("inputfile,i", po::value<std::string>(&f_inputFilename), "Binary command file to use as input.")
		  ("dio,d", po::value<std::string>(&f_sDioDevice)->notifier(on_dio), "rendio digital IO device")
		  ("fullscreen,f", po::value<std::string>(&f_sScreens)->default_value("")->implicit_value("first"), "fullscreen (screen name optional)")  // mut. exclusive 'f' and 'y'
		  ("geometry,y", po::value<std::string>(&sGeometry)->notifier(on_geometry), "render display geometry in pixels WxH")
		  ("calibration,C", po::bool_switch(&f_bCalibrationStartup)->default_value(false), "display calibration screen on startup")
		  ("timing,t", po::bool_switch(&f_bGenerateTiming)->default_value(false), "Generate timing information")
		  ("timing-stopped,T", po::bool_switch(&f_bGenerateTimingStopped)->default_value(false), "Timing recording is initially stopped. Start with key commands.")
		  ("savecommands,c", po::bool_switch(&f_bSaveCommands)->default_value(false), "Save binary input commands to file.")
		  ("grab,g", po::value<std::string>(&f_sGrabberBasename)->default_value("none")->notifier(on_grab), "Grab and save frames to file (using key commands)")
		  ("grab-all,G", po::bool_switch(&f_bGrabAll)->default_value(false), "Grab all frames. Must also set basename with -g")
		  ("input-delay,I", po::value<unsigned long>(&input_thread_param.interval), "Delay in input loop (usec).")
		  ("window-delay,W", po::value<unsigned long>(&window_thread_param.interval), "Delay in window loop (usec).")
		  ("normal-priority,N", po::bool_switch(&f_bNormalPriority)->default_value(false), "Use normal scheduler and priority for all threads.")
		  ("aspect,a", po::value<double>(&f_aspect)->default_value(1.0), "Aspect ratio (width/height of a pixel). Default 1.0")
		  ("queue-size,q", po::value<int>(&f_queuesize)->default_value(25), "Size of input message queue. Default=25")
        ;
        po::variables_map vm;

        std::cout << "parse and store..." << std::endl;

        po::store(po::parse_command_line(argc, argv, desc), vm);

        std::cout << "notify..." << std::endl;

        po::notify(vm);

        std::cout << "check conflicts..." << std::endl;

        conflicting_options(vm, "port", "jsport", "inputfile");
        conflicting_options(vm, "fullscreen", "geometry");
        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return false;
        }


        // check fullscreen option - on second thought, don't
        // default value of f_sScreens is ""
        // implicit value is "first" - use first screen - get this with just "-f" on command line
        // explicit value is "whatever" - i.e. "-f whatever"

        //std::cout << "fullscreen count " << vm.count("fullscreen") << " defaulted " << vm["fullscreen"].defaulted() << " value " << f_sScreens << std::endl;
        //f_fullscreen = vm.count("fullscreen");

		// set up msgq
        if (vm.count("port") && !vm["port"].defaulted())
        {
        	std::cout << "socket msg q" << std::endl;
			f_msgqP = RMessageQueueP(new SocketMessageQueue(f_port, f_queuesize));
        }
        else if (vm.count("jsport") && !vm["jsport"].defaulted())
        {
        	std::cout << "dummy msg q" << std::endl;
			f_msgqP = RMessageQueueP(new DummyMessageQueue(f_queuesize));
        }
        else if (vm.count("inputfile") && !vm["inputfile"].defaulted())
        {
        	std::cout << "file msg q" << std::endl;
        	f_bHaveInputFilename = true;
			f_msgqP = RMessageQueueP(new FileMessageQueue(f_inputFilename, f_queuesize));
        }
        else
    		throw invalid_argument(string("No input type specified (-p|-j|-i)"));
    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }
    catch(...)
    {
        std::cerr << "Unknown error!" << "\n";
        return false;
    }

    std::cout << "args done" << std::endl;

	return true;
}


#if 0
int args(int argc, char **argv)
{
	int status=0;
	int value;
	char *pgeometry = NULL;
	int input_usecs=0;
	int window_usecs=0;
	bool bHaveGeometryOrFullScreen = false;

	struct poptOption optionsTable[] =
	{
		{"console-loglevel", 'l', POPT_ARG_INT, &f_console_loglevel, 0, "Set log output threshhold for console. Default=no console logging.", "[0..5]"},
		{"file-loglevel", 'L', POPT_ARG_INT, &f_file_loglevel, 0, "Set log output threshhold for logfile. Default = no file logging.", "[0..5]"},
		{"syslog-loglevel", 'S', POPT_ARG_INT, &f_syslog_loglevel, 0, "Set log output threshhold for syslog. Default = no syslog logging.", "[0..5]"},
		{"autoflush", 'A', POPT_ARG_NONE, &f_autoflush, 0, "Log msgs are flushed immediately, not buffered.", ""},
		{"verbose", 'V', POPT_ARG_NONE, &f_dumpcommands, 0, "Verbose command dump - commands are logged prior to being handled.", ""},
		{"ortho-left-right", 'o', POPT_ARG_NONE, &f_orthoLeftRight, 0, "Use orthographic-only left/right viewports.", ""},
		{"pulse", 'P', POPT_ARG_NONE, &f_pulse, 0, "Pulse parport pin 1 on swap", ""},
		{"port", 'p', POPT_ARG_INT, &f_port, 'p', "Listen port", "<port-number>"},
		{"jsport", 'j', POPT_ARG_INT, &f_jsport, 'j', "Listen port for json input", "<port-number>"},
		{"inputfile", 'i', POPT_ARG_STRING, &f_pfilename, 'i', "Use a command file as input.", "<path-to-file>"},
		{"flash", 'F', POPT_ARG_INT, &f_nflashes, 'F', "Flash the screen between black/white.", "<# of flashes>"},
		{"dio", 'd', POPT_ARG_STRING, &f_pdiodevice, 0, "rendio digital IO device", "/dev/rendio"},
		{"fullscreen", 'f', POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL, NULL, 'f', "Full screen window", ""},
		{"geometry", 'y', POPT_ARG_STRING, &pgeometry, 'y', "Specify display geometry in pixels", "WxH"},
		{"calibration", 'C', POPT_ARG_NONE, &f_calibration, 0, "Display calibration screen on startup.", ""},
		{"timing", 't', POPT_ARG_NONE, &f_timing, 0, "Generate timing information.", ""},
		{"timing-stopped", 'T', POPT_ARG_NONE, &f_stopped, 0, "Timing info is stopped at startup.", ""},
		{"savecommand", 'c', POPT_ARG_NONE, &f_bSaveCommands, 0, "Save input command stream", ""},
		{"grab", 'g', POPT_ARG_STRING, &f_pGrabberBasename, 0, "Grab and save frames to file (using key commands)", "<output file base>"},
		{"grab-all", 'G', POPT_ARG_NONE, &f_grabAll, 0, "Grab all frames. Must also set basename with -g", ""},
		{"input-usec-delay", 'I', POPT_ARG_INT, &input_usecs, 'I', "Delay in input loop (usec).", "<usec-delay>"},
		{"window-usec-delay", 'W', POPT_ARG_INT, &window_usecs, 'W', "Delay in window loop (usec).", "<usec-delay>"},
		{"normal", 'N', POPT_ARG_NONE, &f_normalpriority, 0, "Use normal scheduler and priority for all threads.", ""},
		{"aspect", 'a', POPT_ARG_DOUBLE, &f_aspect, 0, "Aspect ratio (width/height of a pixel). Default 1.0", ""},
		{"queuesize", 'q', POPT_ARG_INT, &f_queuesize, 0, "Size of input message queue. Default=25", ""},
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
		//value = poptGetNextOpt(optcon);
		while ((value = poptGetNextOpt(optcon)) > -1)
		{
			switch (value)
			{
				case 'f':
				{
					if (!bHaveGeometryOrFullScreen)
					{
						char *arg = poptGetOptArg(optcon);
						f_fullscreen = 1;
						if (arg) f_sScreens.assign(arg);
						std::cerr << "fullscreen arg? " << (arg ? std::string(arg) : "NULL") << std::endl;
						bHaveGeometryOrFullScreen = true;
					}
					else
					{
						std::cerr << "Can only specify -f [screen] or -y wxh, not both!" << std::endl;
						status = 1;
					}
					break;
				}
				case 'y':
				{
					if (!bHaveGeometryOrFullScreen)
					{
						if (sscanf(pgeometry, "%hux%hu", &f_width, &f_height) == EOF)
						{
							std::string s;
							s.assign(pgeometry);
							std::cerr << "Bad geometry input: " << s << std::endl;;
						}
					}
					else
					{
						std::cerr << "Can only specify -f [screen] or -y wxh, not both!" << std::endl;
						status = 1;
					}
					break;
				}
				case 'p':
				{
					if (f_bHaveQueue)
					{
						std::cerr << "Specify one queue type (-p|-q|-i|-F)." << std::endl;
						status = 1;
					}
					else
					{
						f_msgqP = RMessageQueueP(new SocketMessageQueue(f_port, f_queuesize));
						f_bHaveQueue = true;
					}
					break;
				}
				case 'j':
				{
					if (f_bHaveQueue)
					{
						std::cerr << "Specify one queue type (-p|-q|-i|-F)." << std::endl;
						status = 1;
					}
					else
					{
						f_msgqP = RMessageQueueP(new DummyMessageQueue(f_queuesize));
						f_bHaveQueue = true;
					}
					break;
				}
				case 'i':
				{
					if (f_bHaveQueue)
					{
						std::cerr << "Specify one queue type (-p|-q|-i|-F)." << std::endl;
						status = 1;
					}
					else
					{
						f_msgqP = RMessageQueueP(new FileMessageQueue(f_pfilename, f_queuesize));
						f_bHaveQueue = true;
					}
					break;
				}
				case 'F':
				{
					if (f_bHaveQueue)
					{
						std::cerr << "Specify one queue type (-p|-q|-i|-F)." << std::endl;
						status = 1;
					}
					else
					{
						f_msgqP = RMessageQueueP(new FlashMessageQueue(f_nflashes, f_queuesize));
						f_bHaveQueue = true;
					}
					break;
				}
				case 'I':
				{
					input_thread_param.interval = (unsigned int)input_usecs;
					break;
				}
				case 'W':
				{
					window_thread_param.interval = (unsigned int)window_usecs;
					break;
				}
			}
		}

		if (value < -1)
		{
			/* an error occurred during option processing */
			fprintf(stderr, "%s: %s\n",
					poptBadOption(optcon, POPT_BADOPTION_NOALIAS),
					poptStrerror(value));
		    return 1;
		}

		if (!f_bHaveQueue)
		{
			std::cerr << "No input type specified! (-p|-j|-i|-F)" << std::endl;
			status = 1;
		}

	}
	return status;
}
#endif


int initRender()
{
	std::string sBasename;

    RLog::instance()->info("Instantiate base world...");
    f_worldP.reset(new RWorld(f_opfuncBuffer));

	// The ground plane scene gets a background
	f_backgroundPtr = SolidBackground::create();
	addRefCP(f_backgroundPtr);
	beginEditCP(f_backgroundPtr);
	f_backgroundPtr->setColor(Color3f(0.5, 0.5, 0.5));
	endEditCP(f_backgroundPtr);

	if (!f_orthoLeftRight)
	{
		// add viewports
		ViewportPtr vGPPtr = createGroundPlaneViewport(f_worldP->getHandles());
		ViewportPtr vPPtr = createPerspectiveViewport(f_worldP->getHandles());
		ViewportPtr vOPtr = createOrthoViewport(f_worldP->getHandles(), RenderViewportType::OrthoFull, f_window->getHeight(), f_aspect);

		f_window->addPort(vGPPtr);
		f_window->addPort(vPPtr);
		f_window->addPort(vOPtr);

		ViewportPtr vFirstPtr = f_window->getMFPort()->operator [](0);
		beginEditCP(vFirstPtr);
		vFirstPtr->setBackground(f_backgroundPtr);
		endEditCP(vFirstPtr);

		ViewportPtr vLastPtr = f_window->getMFPort()->operator [](f_window->getMFPort()->size()-1);
		// If there's a grabber requested, create another viewport with the FileGrabForeground
		if (f_bGrab)
		{
			RenderFileGrabForegroundPtr foregroundPtr = RenderFileGrabForeground::create();
			beginEditCP(foregroundPtr);
			foregroundPtr->setOutputPath(f_sGrabberBasename.c_str());
			foregroundPtr->setActive(false);
			foregroundPtr->setIncrement(true);
			foregroundPtr->setAutoResize(true);
			endEditCP(foregroundPtr);
			beginEditCP(vLastPtr);
			vLastPtr->getForegrounds().push_back(foregroundPtr);
			endEditCP(vLastPtr);

			RLog::instance()->info("Add GRAB command...");
			GrabMessageHandler* pgrabber = new GrabMessageHandler();
			f_worldP->getMessageHandlers().addHandler(pgrabber);
			pgrabber->addGrabber(foregroundPtr);
		}

		// need to hang onto frame handler pointer. Could have done this in the subroutine,
		// but this is a little more transparent.
		f_pFrameHandler = new FrameMessageHandler(f_window);
		f_worldP->getMessageHandlers().addHandler(f_pFrameHandler);

		// perspective message handler added here as we need pointer to camera(s)
		f_worldP->getMessageHandlers().addHandler(new RPerspectiveMessageHandler(osg::PerspectiveCameraPtr::dcast(vGPPtr->getCamera()), PerspectiveCameraPtr::dcast(vPPtr->getCamera())));


		// Add message handlers that correspond to ALL viewport types
		addGenericMessageHandlers();

		// Add message handlers that correspond to perspective viewports.
		// Note that render retains the weirdness (my fault) that there are two perspective
		// viewports, each with its own camera. TODO: merge these two viewports!
		addPerspectiveMessageHandlers();


		// Add message handlers that correspond to orthographic viewports
		addOrthographicMessageHandlers();
	}
	else
	{
		// add viewports
		ViewportPtr vLeftPtr = createOrthoViewport(f_worldP->getHandles(), RenderViewportType::OrthoLeft, f_window->getHeight(), f_aspect);
		ViewportPtr vRightPtr = createOrthoViewport(f_worldP->getHandles(), RenderViewportType::OrthoRight, f_window->getHeight(), f_aspect);

		f_window->addPort(vLeftPtr);
		f_window->addPort(vRightPtr);

		beginEditCP(vLeftPtr);
		vLeftPtr->setBackground(f_backgroundPtr);
		endEditCP(vLeftPtr);

		beginEditCP(vRightPtr);
		vRightPtr->setBackground(f_backgroundPtr);
		endEditCP(vRightPtr);

		ViewportPtr vLastPtr = f_window->getMFPort()->operator [](f_window->getMFPort()->size()-1);

		// If there's a grabber requested, create another viewport with the FileGrabForeground
		if (f_bGrab)
		{
			RLog::instance()->info("Add GRAB command...");
			GrabMessageHandler* pgrabber = new GrabMessageHandler();
			f_worldP->getMessageHandlers().addHandler(pgrabber);

			RenderFileGrabForegroundPtr leftForegroundPtr = RenderFileGrabForeground::create();
			beginEditCP(leftForegroundPtr);
			leftForegroundPtr->setOutputPath(f_sGrabberBasename.c_str());
			leftForegroundPtr->setExtraString("L");
			leftForegroundPtr->setActive(false);
			leftForegroundPtr->setIncrement(true);
			leftForegroundPtr->setAutoResize(true);
			endEditCP(leftForegroundPtr);
			beginEditCP(vLeftPtr);
			vLeftPtr->getForegrounds().push_back(leftForegroundPtr);
			endEditCP(vLeftPtr);
			pgrabber->addGrabber(leftForegroundPtr);

			RenderFileGrabForegroundPtr rightForegroundPtr = RenderFileGrabForeground::create();
			beginEditCP(rightForegroundPtr);
			rightForegroundPtr->setOutputPath(f_sGrabberBasename.c_str());
			rightForegroundPtr->setExtraString("R");
			rightForegroundPtr->setActive(false);
			rightForegroundPtr->setIncrement(true);
			rightForegroundPtr->setAutoResize(true);
			endEditCP(rightForegroundPtr);
			beginEditCP(vLeftPtr);
			vLeftPtr->getForegrounds().push_back(rightForegroundPtr);
			endEditCP(vLeftPtr);
			pgrabber->addGrabber(rightForegroundPtr);

		}

		// need to hang onto frame handler pointer. Could have done this in the subroutine,
		// but this is a little more transparent.
		f_pFrameHandler = new FrameMessageHandler(f_window);
		f_worldP->getMessageHandlers().addHandler(f_pFrameHandler);

		// Add message handlers that correspond to ALL viewport types
		addGenericMessageHandlers();

		// Add message handlers that correspond to orthographic viewports
		addOrthographicMessageHandlers();

	}
	return 0;
}

void addGenericMessageHandlers()
{
	RLog::instance()->info("Adding message handlers for all viewport types...");

	f_worldP->getMessageHandlers().addHandler(new RBackgroundMessageHandler(f_backgroundPtr));
	f_worldP->getMessageHandlers().addHandler(new StartStopMessageHandler(f_window, *f_msgqP));
	f_worldP->getMessageHandlers().addHandler(new TestMessageHandler());
	f_worldP->getMessageHandlers().addHandler(new ResetMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new RUpdateMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new RQuitMessageHandler(f_worldP, f_msgqP));
	f_worldP->getMessageHandlers().addHandler(new RReleaseMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new RPingMessageHandler(f_window));
	f_worldP->getMessageHandlers().addHandler(new RDumpMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new PathMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new CircularOrbitMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new MoveObjectMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new LissajousMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new TestAnimMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new RJSMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new RTriggerMessageHandler(f_worldP, f_fdRendio));
	f_worldP->getMessageHandlers().addHandler(new RCommandListMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new ROnOffMessageHandler(f_worldP, f_fdRendio));
	f_worldP->getMessageHandlers().addHandler(new RScriptMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new UnitsMessageHandler(f_window->rate(), f_window->getWidth(), f_window->getHeight()));

}

void addOrthographicMessageHandlers()
{
	RLog::instance()->info("Adding message handlers for orthographic(2D) viewport types...");

	f_worldP->getMessageHandlers().addHandler(new DotMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new RectMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new GratingMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new DonutMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new GridMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new FF2DMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new CalibrationMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new GaussianDotMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new XHairMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new PlaidMessageHandler(f_worldP));
}

void addPerspectiveMessageHandlers()
{
	RLog::instance()->info("Adding message handlers for perspective(3D) viewport types...");

	// these really apply to groundplane viewport
	f_worldP->getMessageHandlers().addHandler(new RGroundPlaneMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new ROuterSpaceMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new TargetMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new TargetTransformMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new PursuitTransformMessageHandler(f_worldP));
	f_worldP->getMessageHandlers().addHandler(new ConstantDensityPlaneMessageHandler(f_worldP));

	// these apply to the generic perspective viewport
	f_worldP->getMessageHandlers().addHandler(new DotFieldMessageHandler(f_worldP));

	// these apply to both gp and generic viewport
	f_worldP->getMessageHandlers().addHandler(new RCameraMessageHandler(f_worldP));

}

/* Signal handler for SIGINT. */
void sigint_handler (int signum)
{
	UNUSED(signum);
	RLog::instance()->warn("signal received");
	f_bAbortQuit = true;
}


void *input_thread(void *param)
{
	struct thread_param *par = (struct thread_param*)param;
	struct sched_param schedp;
	int policy = SCHED_FIFO;
	struct timespec interval;

	interval.tv_sec = par->interval / USEC_PER_SEC;
	interval.tv_nsec = (par->interval % USEC_PER_SEC) * 1000;

	RLog::instance()->info("input_thread: Using sleep interval " + lexical_cast<std::string>((float)interval.tv_sec*1000.0f + (float)interval.tv_nsec/1000000.0f) + " ms");

	if (!f_bNormalPriority)
	{
		memset(&schedp, 0, sizeof(schedp));
		schedp.sched_priority = par->prio;
		if (sched_setscheduler(0, policy, &schedp) < 0)
		{
			RLog::instance()->error("input_thread sched_setscheduler: " + lexical_cast<std::string>(strerror(errno)));
		}
	}
	else
	{
		RLog::instance()->warn("input_thread: using normal priority and scheduler");
	}

	par->threadstarted = 1;

	while (!f_bAbortQuit)
	{
		f_msgqP->checkInput();

		if (f_bNormalPriority)
		{
			usleep((unsigned int)par->interval);
		}
		else
		{
			nanosleep(&interval, NULL);
		}
	}

	if (!f_bNormalPriority)
	{
		/* switch to normal */
		schedp.sched_priority = 0;
		sched_setscheduler(0, SCHED_OTHER, &schedp);
	}

	par->threadstarted = 0;
	RLog::instance()->info("input_thread: done");

	return NULL;
}



void *rendio_thread(void *param)
{
	struct thread_param *par = (struct thread_param*)param;
	struct sched_param schedp;
	int policy = SCHED_FIFO;

	if (!f_bNormalPriority)
	{
		memset(&schedp, 0, sizeof(schedp));
		schedp.sched_priority = par->prio;
		if (sched_setscheduler(0, policy, &schedp) < 0)
		{
			RLog::instance()->error("rendio_thread sched_setscheduler: " + lexical_cast<std::string>(strerror(errno)));
		}
	}
	else
	{
		RLog::instance()->warn("rendio_thread: using normal priority and scheduler");
	}

	par->threadstarted = 1;

	while (!f_bAbortQuit)
	{
		unsigned long trigger;
		int status;
		status = ioctl(f_fdRendio, IOCTL_NEXT_TRIGGER, &trigger);
		RLog::instance()->info(std::string("rendio_thread status ") + lexical_cast<std::string>(status) + std::string(" trigger ") + lexical_cast<std::string>(trigger));

		// Got a trigger; find commands and queue them
		f_worldP->queueOpsForTrigger(trigger);
	}

	if (!f_bNormalPriority)
	{
		/* switch to normal */
		schedp.sched_priority = 0;
		sched_setscheduler(0, SCHED_OTHER, &schedp);
	}

	par->threadstarted = 0;
	RLog::instance()->info("input_thread: done");

	return NULL;
}

#if 0
// receives first read.
// client should send the size of the JS as an int, followed by two '\n', then followed by the JS itself.

void inputjs_thread_read_request_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
	RLog::instance()->info(std::string(__func__) + std::string(" bytes_transferred: ") + lexical_cast<std::string>(bytes_transferred));

	if (ec)
	{
		RLog::instance()->error(ec.message());
	}
	else
	{
		// We read an int, which is the size of the coming string. Also skip over the two "\n".
		std::istream request_stream(&f_streambuf);
//		RLog::instance()->info(std::string("streambuf size before read ") + lexical_cast<std::string>(f_streambuf.size()));
		request_stream >> f_jsNumBytes;
		RLog::instance()->info(std::string("Expecting JS of ") + lexical_cast<std::string>(f_jsNumBytes) + std::string(" bytes"));
//		RLog::instance()->info(std::string("streambuf size after read ") + lexical_cast<std::string>(f_streambuf.size()));

		// read the two \n
		char c[2];
		request_stream.read(c, 2);

		// There may be data remaining after the two \n - write extra stuff to the oss
		do {
			request_stream.read(f_jsBuffer.c_array(), (std::streamsize) f_jsBuffer.size());
			f_ossJSBytes.write(f_jsBuffer.c_array(), request_stream.gcount());
			RLog::instance()->info(std::string(__func__) + std::string(" bytes written to OSS: ") + lexical_cast<std::string>(request_stream.gcount()) + std::string(" total: ") + lexical_cast<std::string>(f_jsBytesWritten));
			f_jsBytesWritten += request_stream.gcount();
		} while (request_stream.gcount() > 0);


		if (f_jsBytesWritten < f_jsNumBytes)
		{
			f_tcp_socket.async_read_some(boost::asio::buffer(f_jsBuffer.c_array(), f_jsBuffer.size()), inputjs_thread_read_handler);
		}
		else
		{
			inputjs_thread_parse();
		}
	}
	return;
}

void inputjs_thread_parse()
{
	// Have complete js. Parse it....
	try
	{
		f_worldP->parseRawJS(f_ossJSBytes.str());
	}
	catch (const std::exception& e)
	{
		RLog::instance()->error(std::string(e.what()));
	}
	// close socket, then go back and wait for more connections
	f_ossJSBytes.str("");
	f_jsNumBytes = 0;
	f_jsBytesWritten = 0;
	f_tcp_socket.close();
	f_tcp_acceptor.async_accept(f_tcp_socket, inputjs_thread_accept_handler);
}

void inputjs_thread_read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
	if (!ec)
	{
		RLog::instance()->info(std::string(__func__) + std::string(" bytes_transferred: ") + lexical_cast<std::string>(bytes_transferred));
		f_ossJSBytes.write(f_jsBuffer.c_array(), bytes_transferred);
		f_jsBytesWritten += bytes_transferred;
		RLog::instance()->info(std::string(__func__) + std::string(" total: ") + lexical_cast<std::string>(f_jsBytesWritten));

		if (f_jsBytesWritten < f_jsNumBytes)
		{
			f_tcp_socket.async_read_some(boost::asio::buffer(f_jsBuffer.c_array(), f_jsBuffer.size()), inputjs_thread_read_handler);
		}
		else
		{
			// Have complete js. Parse it....
			inputjs_thread_parse();
		}
	}
	else
	{
		// Have complete js. Parse it....
		inputjs_thread_parse();
	}
}

void inputjs_thread_accept_handler(const boost::system::error_code &ec)
{
	RLog::instance()->info(__func__);
	if (!ec)
	{
		boost::asio::async_read_until(f_tcp_socket, f_streambuf, "\n\n", inputjs_thread_read_request_handler);
	}
	else
		RLog::instance()->info(ec.message());
}
#endif


void *inputjs_thread(void *param)
{
	struct thread_param *par = (struct thread_param*)param;
	struct sched_param schedp;
	int policy = SCHED_FIFO;

	RLog::instance()->info("inputjs_thread staring...");

	if (!f_bNormalPriority)
	{
		memset(&schedp, 0, sizeof(schedp));
		schedp.sched_priority = par->prio;
		if (sched_setscheduler(0, policy, &schedp) < 0)
		{
			RLog::instance()->error("inputjs_thread sched_setscheduler: " + lexical_cast<std::string>(strerror(errno)));
		}
	}
	else
	{
		RLog::instance()->warn("inputjs_thread: using normal priority and scheduler");
	}

	par->threadstarted = 1;
	boost::asio::io_service ioservice;

#if 0
	f_tcp_endpoint.port(f_jsport);
	f_tcp_acceptor.listen();
	f_tcp_acceptor.async_accept(f_tcp_socket, inputjs_thread_accept_handler);
	f_ioservice.run();
#else
	render::AsyncTcpServer server(ioservice, f_jsport, boost::bind(&render::RWorld::parseRawJS, f_worldP, _1), true);
	ioservice.run();
#endif


	// done doing stuff here

	if (!f_bNormalPriority)
	{
		/* switch to normal */
		schedp.sched_priority = 0;
		sched_setscheduler(0, SCHED_OTHER, &schedp);
	}

	par->threadstarted = 0;
	RLog::instance()->info("inputjs_thread: done");

	return NULL;
}


void *window_thread(void *param)
{
	struct thread_param *par = (struct thread_param *)param;
	struct sched_param schedp;
	int policy = SCHED_FIFO;
	struct timespec interval;

	interval.tv_sec = par->interval / USEC_PER_SEC;
	interval.tv_nsec = (par->interval % USEC_PER_SEC) * 1000;

	RLog::instance()->info("window_thread: Using sleep interval " + lexical_cast<std::string>((float)interval.tv_sec*1000.0f + (float)interval.tv_nsec/1000000.0f) + " ms");

	if (!f_bNormalPriority)
	{
		memset(&schedp, 0, sizeof(schedp));
		schedp.sched_priority = par->prio;
		if (sched_setscheduler(0, policy, &schedp) < 0)
		{
			RLog::instance()->error("window_thread sched_setscheduler: " + lexical_cast<std::string>(strerror(errno)));
		}
	}
	else
	{
		RLog::instance()->warn("window_thread: using normal priority and scheduler");
	}

	par->threadstarted = 1;

	while (!f_bAbortQuit)
	{
		RMarkers::instance()->mark(mtWindowStart);
		windowEvents();
		RMarkers::instance()->mark(mtWindowEnd);

		if (f_bNormalPriority)
		{
			usleep((unsigned int)par->interval);
		}
		else
		{
			nanosleep(&interval, NULL);
		}
	}

	if (!f_bNormalPriority)
	{
		/* switch to normal */
		schedp.sched_priority = 0;
		sched_setscheduler(0, SCHED_OTHER, &schedp);
	}

	par->threadstarted = 0;
	RLog::instance()->info("window_thread: done");

	return NULL;
}

void key_callback(GLFWwindow*, int key, int, int action, int mods)
{
   	MessageStruct msg;

   	if (action != GLFW_PRESS) return;
	switch(key)
	{
	case GLFW_KEY_ESCAPE:
		RLog::instance()->info("KeyPress - ESC - queue QUIT command.");
		make_quit_msg(&msg);
		f_msgqP->push_front(RMessage(msg));
		break;
	case 'V':
		if (!(mods & GLFW_MOD_SHIFT))
		{
			if (f_dumpcommands)
			{
				RLog::instance()->info("KeyPress - V - stop dumping commands.");
				f_dumpcommands = false;
			}
			else
			{
				RLog::instance()->info("KeyPress - V - start dumping commands.");
				f_dumpcommands = true;
			}
		}
		break;
	case 'T':
		if (!(mods & GLFW_MOD_SHIFT))
		{
			if (f_bGenerateTiming)
			{
				if (f_bGenerateTimingStopped)
				{
					RLog::instance()->info("KeyPress - t - start recording timing info.");
					RMarkers::start(true);
					f_bGenerateTimingStopped = false;
				}
				else
				{
					RLog::instance()->info("KeyPress - t - stop recording timing info.");
					RMarkers::start(false);
					f_bGenerateTimingStopped = true;
				}
			}
			else
			{
					RLog::instance()->info("KeyPress - t - You must activate timing on startup with -t command line switch.");
			}
		}
		break;
	case 'F':
		if (!(mods & GLFW_MOD_SHIFT))
		{
			make_dump_msg(&msg, (char)DUMP_FIXTURES, 0);
			f_msgqP->push_front(RMessage(msg));
		}
		break;
	case 'N':
		if (!(mods & GLFW_MOD_SHIFT))
		{
			make_dump_msg(&msg, (char)DUMP_NODES, 0);
			f_msgqP->push_front(RMessage(msg));
		}
		break;
	case 'C':
		if (!(mods & GLFW_MOD_SHIFT))
		{
			if (f_bSaveCommands > 0)
			{
				if (f_bSaveCommandsStopped)
				{
					RLog::instance()->info("KeyPress - c - start saving commands.");
					f_bSaveCommandsStopped = false;
				}
				else
				{
					RLog::instance()->info("KeyPress - c - stop saving commands.");
					f_bSaveCommandsStopped = true;
				}
			}
			else
			{
					RLog::instance()->info("KeyPress - c - You must activate command saving on startup with -c command line switch.");
			}
		}
		break;
	case 'G':
		if (!f_bGrab)
		{
			RLog::instance()->info("KeyPress - g/G - You must activate frame grabbing on startup with -g command line switch.");
		}
		else if (!(mods & GLFW_MOD_SHIFT))
		{
			RLog::instance()->info("KeyPress - g - Grab single frame!");
			make_grab_single_msg(&msg);
			f_msgqP->push_front(RMessage(msg));
		}
		else
		{
			RLog::instance()->info("KeyPress - G - Grab all frames!");
			make_grab_on_msg(&msg);
			f_msgqP->push_front(RMessage(msg));
		}
		break;
	case 'X':
		if (mods & GLFW_MOD_SHIFT)
		{
			if (f_bGrab)
			{
				RLog::instance()->info("KeyPress - X - Stop grabbing!");
				make_grab_off_msg(&msg);
				f_msgqP->push_front(RMessage(msg));
			}
			else
			{
				RLog::instance()->info("KeyPress - X - You must activate frame grabbing on startup with -g command line switch.");
			}
		}
		break;
	case 'H':
		if (!(mods & GLFW_MOD_SHIFT))
		{
			RLog::instance()->info("KeyPress commands: (* = must be activated with a command line switch).");
			RLog::instance()->info("<ESC>\tQuit");
			RLog::instance()->info("v\tToggle command dumping. (* -V)");
			RLog::instance()->info("t\tToggle recording timing info. (* -t)");
			RLog::instance()->info("n\tDump nodes in scenegraph.");
			RLog::instance()->info("f\tDump fixtures in scenegraph.");
			RLog::instance()->info("c\tToggle saving commands to file. (* -c)");
			RLog::instance()->info("g\tGrab next frame to file. (* -g)");
			RLog::instance()->info("G\tGrab ALL subsequent frames to files. (* -g)");
			RLog::instance()->info("X\tStop grabbing. (* -g)");
			RLog::instance()->info("h\tThis message.");
		}
		break;
	default:
		RLog::instance()->info("KeyPress - unknown keysym");
		break;
	}
}

void windowEvents()
{
	glfwPollEvents();
}

std::string getFilenameWithTimestamp(const std::string& extension)
{
	char filename[256];
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(filename, 256, "render-%Y-%m-%d-%H-%S.", timeinfo);
	std::string s(filename);
	s.append(extension);
	return s;
}

void error_callback(int, const char* description)
{
	RLog::instance()->error(description);
}

void doJSQueue()
{
	RMessage msg;
	MessageHandlerStatus msgStatus;
	std::ostringstream oss;

	RLog::instance()->info("Starting JSQueue...");
	while (!f_bAbortQuit)
	{
		// cycle through any animation sequences
		// for (iterator itAnim = animations.begin(); itAnim!=animations.end(); it++) {
		//    if (itAnim->state() == Running)
		//       f_opfuncBuffer.push_front(itAnim->frame(current_frame_number))
		//
		// Use of 'current_frame_number' implies that subclasses should generate OPs
		// for the FRAME WHICH FOLLOWS the 'current_frame_number'
		//
		// }


		OpFunc func;
		while (f_opfuncBuffer.pop_back(&func))
		{
			try
			{
				func();
			}
			catch (std::exception& e)
			{
		    	RLog::instance()->error(std::string(e.what()));
			}
		}

		while (!f_bAbortQuit && f_msgqP->pop(msg))
		{
			CommandKey key = makeCommandKey(msg);
			RMessageHandlerBaseP handlerP = f_worldP->getMessageHandlers().getHandler(key);
			if (!handlerP)
			{
				RLog::instance()->error("Cannot get handler for msg!");
				RLog::instance()->error("cmd: " + boost::lexical_cast<std::string>(key) + " length " + boost::lexical_cast<std::string>(msg.commandLength()));
			}
			else
			{

				if (f_dumpcommands)
				{
					RLog::instance()->debug(handlerP->toString(msg));
				}

				RMarkers::instance()->mark(mtExeStart, makeCommandKey(msg));
				msgStatus = handlerP->handle(key, msg, f_msgqP->responder());
				RMarkers::instance()->mark(mtExeEnd);

				if (msgStatus==MessageHandlerStatus::Quit) f_bAbortQuit = true;
				if (msgStatus==MessageHandlerStatus::Error)
				{
					oss << "Error in command" ;
					RLog::instance()->error(oss.str());
					oss << msg;
					RLog::instance()->error(oss.str());
					f_bAbortQuit = true;
				}
			}
		}

		// render and swap....
		unsigned int missed = f_pFrameHandler->doRender();
		unsigned long enable = 1;

		// enable dig output
		ioctl(f_fdRendio, IOCTL_ENABLE_OUTPUT, enable);

		if (missed > 1)
			RLog::instance()->info("missed " + lexical_cast<std::string>(missed));

	}
	return;
}

void doOldQueue()
{
	RMessage msg;
	MessageHandlerStatus msgStatus;
	int popStatus;
	std::ostringstream oss;
	unsigned int missed;
	while (!f_bAbortQuit)
	{
		RMarkers::instance()->mark(mtInputStart);
		popStatus = f_msgqP->pop(msg);
		RMarkers::instance()->mark(mtInputEnd);
		if (!popStatus)
		{
			RLog::instance()->error("Error reading from message queue.");
			f_bAbortQuit = true;
		}
		else
		{
			// Write the command to file. This is set with the -c switch.
			if (f_bSaveCommands && !f_bSaveCommandsStopped)
			{
				f_commandstream.write((char *)&msg, 2+(unsigned int)msg.header.length);
				f_commandstream.flush();
			}

			CommandKey key = makeCommandKey(msg);
			RMessageHandlerBaseP handlerP = f_worldP->getMessageHandlers().getHandler(key);

			if (handlerP)
			{
				if (f_dumpcommands)
				{
					RLog::instance()->debug(handlerP->toString(msg));
				}

				RMarkers::instance()->mark(mtExeStart, makeCommandKey(msg));
				f_msgqP->responder().clear();
				msgStatus = handlerP->handle(key, msg, f_msgqP->responder());
				RMarkers::instance()->mark(mtExeEnd);

				if (msgStatus==MessageHandlerStatus::Quit)
				{
					f_bAbortQuit = true;
					return;
				}
				else if (msgStatus==MessageHandlerStatus::Error)
				{
					oss << "Error in command" ;
					RLog::instance()->error(oss.str());
					oss << msg;
					RLog::instance()->error(oss.str());
					f_bAbortQuit = true;
					return;
				}
				else if (msgStatus==MessageHandlerStatus::Render)
				{
					missed = f_pFrameHandler->doRender();
				}
				else if (msgStatus == MessageHandlerStatus::Frame)
				{
					missed = f_pFrameHandler->doRender();
				}
				if (f_dumpcommands)
					util::RLog::instance()->info((boost::format{"render::doOldQueue() - responder length: %d"} % f_msgqP->responder().length()).str());
				if (f_msgqP->responder().length() > 0) f_msgqP->responder().send();
			}
			else
			{
				util::RLog::instance()->error((boost::format{"render::doOldQueue() - no command found for command key %x"} % makeCommandKey(msg)).str());
			}
		}
	}
}

void initLogging()
{
	if (f_console_loglevel > -1)
	{
		RLog::instance()->toConsole(f_console_loglevel);
	}

	if (f_file_loglevel > -1)
	{
		RLog::instance()->toFile("render.log", f_console_loglevel);
	}

	if (f_syslog_loglevel > -1)
	{
		RLog::instance()->toSyslog(f_syslog_loglevel);
	}

}



int main(int argc, char **argv)
{
	std::ostringstream oss;
	struct sigaction sa;
	pthread_t marker_thread;
	render::util::RMarkersOutputInfo marker_thread_info;
	struct sched_param schedp;
	int policy = SCHED_FIFO;


	std::cerr << "args " << argc << std::endl;

	// Process input args. OSG has not been initialized yet, so no
	// OSG calls allowed!
	if (!args(argc, argv))
	{
		std::cerr << "Error on command line.";
		return -1;
	}

	initLogging();

	// Open rendio device if requested
	if (f_bDioDevice)
	{
		f_fdRendio = open(f_sDioDevice.c_str(), O_RDONLY, 0);
		if (f_fdRendio > 0)
		{
			RLog::instance()->info(std::string("DIO device ") + f_sDioDevice + std::string(" opened."));
		}
		else
		{
			perror("rendio");
			RLog::instance()->error(std::string("Cannot open DIO device ") + f_sDioDevice);
			return -1;
		}
	}

	// Do OSG initialization. Must do this before any field containers are created.
	RLog::instance()->info("Initialize OSG...");
    osgInit(argc, argv);

	// Create and open window. Remember that we still need to add viewports to this window!
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    RLog::instance()->info("Create window...");
    f_window = RenderXWindow::create();


    // Now init window
    if (f_sScreens.size() > 0)
    {
    	f_window->setFullScreen(f_sScreens);
    	// Width and Height are set when init() called.
    }
    else
    {
    	f_window->setWidth(f_width);	// these have to be called prior to init().
    	f_window->setHeight(f_height);
    }
    f_window->init();
    glfwSetKeyCallback(f_window->getGLFWWindow(), key_callback);


    // Get func pointer for vblank callback
	if (glfwExtensionSupported("GLX_SGI_video_sync"))
	{
	    // The extension is supported by the current context
		RLog::instance()->info("render: Have extension GLX_SGI_video_sync");
		f_glXGetVideoSync = (PFNGLXGETVIDEOSYNCSGIPROC) glfwGetProcAddress("glXGetVideoSyncSGI");
		if (!f_glXGetVideoSync)
			RLog::instance()->error("Get glx func addresses..._glXGetVideoSync is NULL");
		f_glXWaitVideoSync = (PFNGLXWAITVIDEOSYNCSGIPROC) glfwGetProcAddress("glXWaitVideoSyncSGI");
		if (!f_glXWaitVideoSync)
			RLog::instance()->error("Get glx func addresses..._glXWaitVideoSync is NULL");
	}

	// Initialize window class and render world
	if (initRender())
	{
		RLog::instance()->fatal("Cannot initialize render!");
		return -1;
	}

	// If full session grabbing requested, queue the GRAB ON  command
	if (f_bGrabAll)
	{
		MessageStruct msg;
		make_grab_on_msg(&msg);
		f_msgqP->push_front(RMessage(msg));
	}


	// If calibration requested, queue the command(s)
	if (f_bCalibrationStartup)
	{
	   	MessageStruct msg;
	   	BackgroundStruct back;
	   	CalibrationStruct cal;
	   	back.r = back.g = back.b = back.a = 0;
	   	make_background_msg_s(&msg, &back);
		f_msgqP->push_front(RMessage(msg));
		cal.width = f_window->getWidth();
		cal.height = f_window->getHeight();
		cal.square_size = (f_window->getHeight()/200 + 1)*100;
		cal.depth = 10;
		make_calibration_msg_s(&msg, &cal);
		f_msgqP->push_front(RMessage(msg));
		make_render_msg(&msg);
		f_msgqP->push_front(RMessage(msg));
	}

	// Setup signal handlers and sigprocmask.
	// This must be done before any other threads are started, it seems, or else we get killed by a "SIGALRM"
	RLog::instance()->info("Set up interrupt handlers...");
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sigint_handler;
	sigaction (SIGINT, &sa, 0);
	sigaction (SIGTERM, &sa, 0);


	if (f_bSaveCommands)
	{
		f_commandstream.open(getFilenameWithTimestamp("cmd"));
		if (!f_commandstream.is_open())
		{
			RLog::instance()->error("Cannot open command output file. Do you have write access?");
			RLog::instance()->error("Disabling command saving!");
			f_bSaveCommands = false;
		}
	}


	// Set scheduling policy and priority for main thread.
	RLog::instance()->info("Setting scheduling policy and priority...");


	if (!f_bNormalPriority)
	{
		// lock pages into memory
		if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0)
		{
			RLog::instance()->error("Cannot disable paging! Are you root?");
		}
		else
		{
			RLog::instance()->info("Paging is disabled for this process.");
		}

		memset(&schedp, 0, sizeof(schedp));
		schedp.sched_priority = command_thread_param.prio;
		if (sched_setscheduler(0, policy, &schedp) < 0)
		{
			RLog::instance()->error("main sched_setscheduler: " + lexical_cast<std::string>(strerror(errno)));
		}
	}
	else
	{
		RLog::instance()->warn("command_thread: using normal priority and scheduler");
	}


	if (f_bGenerateTiming)
	{
		RLog::instance()->info("Start marker thread...");

		// initialize markers -- should do this before any threads are started to ensure good calibration
		RMarkers::instance()->setup(getFilenameWithTimestamp("tim"));

		if (f_bGenerateTimingStopped)
		{
			RMarkers::start(false);
		}

		// now start thread
		marker_thread_info.msecs = 2000;
		pthread_create(&marker_thread, NULL, RMarkers::output_thread, &marker_thread_info);
	}

	// start rendio thread
	if (f_fdRendio > 0)
	{
		RLog::instance()->info("Start rendio thread...");
		pthread_create(&rendio_thread_param.thread, NULL, rendio_thread, &rendio_thread_param);
	}

	// OK, now start window thread
	RLog::instance()->info("Starting window thread...");
	pthread_create(&window_thread_param.thread, NULL, window_thread, &window_thread_param);

	// Initialize input queue. Start input thread if needed. Then start command queue processing.
	f_msgqP->initialize();
	if (f_port || f_bHaveInputFilename)
	{
		RLog::instance()->info("Starting message queue thread...");
		pthread_create(&input_thread_param.thread, NULL, input_thread, &input_thread_param);
		doOldQueue();
	}
	else if (f_jsport)
	{
		RLog::instance()->info("Starting json input thread...");
		pthread_create(&inputjs_thread_param.thread, NULL, inputjs_thread, &inputjs_thread_param);
		doJSQueue();
	}

	RLog::instance()->info("render exiting.");

	// Dump commands
	if (f_bSaveCommands > 0)
	{
		if (f_commandstream.is_open()) f_commandstream.close();
	}

	// Kill input and window thread
	f_bAbortQuit = true;

	if (input_thread_param.threadstarted == 1)
	{
		RLog::instance()->info("Waiting for input thread to exit...");
//		pthread_kill(input_thread_param.thread, SIGTERM);
	}
	pthread_join(input_thread_param.thread, NULL);

	// Kill window thread
	if (window_thread_param.threadstarted == 1)
	{
		RLog::instance()->info("Waiting for window thread to exit...");
//		pthread_kill(window_thread_param.thread, SIGTERM);
	}
	pthread_join(window_thread_param.thread, NULL);

	if (f_bGenerateTiming)
	{
		RLog::instance()->info("Kill marker thread...");
		RMarkers::instance()->quit();
		pthread_join(marker_thread, NULL);
	}

	// close rendio device and kill thread
	if (f_fdRendio > 0)
	{
		RLog::instance()->info("Close rendio device...");
		close(f_fdRendio);
		RLog::instance()->info("kill rendio thread...");
		pthread_kill(rendio_thread_param.thread, SIGUSR1);
		pthread_join(rendio_thread_param.thread, NULL);
	}

	/* switch this thread to normal */
	schedp.sched_priority = 0;
	sched_setscheduler(0, SCHED_OTHER, &schedp);

	return 0;
}


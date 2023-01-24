/*
 * fs2.cpp
 *
 *  Created on: Aug 31, 2015
 *      Author: dan
 */

#include <iostream>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#include <GLFW/glfw3native.h>
#include <GL/glxext.h>

using namespace std;

int f_w, f_h;
int f_pointsize;
int f_separation;
int f_stepsize;
int f_step = 0;  // incremented each time through display()
int f_primitive = 0;	// 0 == GL_POINTS, 1=GL_TRIANGLES
int f_skipper = 1;

static void error_callback(int error, const char* description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS) return;
	switch(key)
	{
	case 'P':
		if (mods & GLFW_MOD_SHIFT)
			f_pointsize++;
		else
			if (f_pointsize > 1) f_pointsize--;
		break;
	case 'S':
		if (mods & GLFW_MOD_SHIFT)
			f_stepsize++;
		else
			if (f_stepsize > 1) f_stepsize--;
		break;
	case 'N':
		if (mods & GLFW_MOD_SHIFT)
			if (f_separation < f_h/2) f_separation++;
		else
			if (f_separation > 10) f_separation--;
		break;
	case 'D':
		f_primitive = 1 - f_primitive;
		break;
	case 'Q':
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case 'K':
		if (mods & GLFW_MOD_SHIFT)
		{
			f_skipper++;
		}
		else
		{
			if (f_skipper > 1) f_skipper--;
		}
		break;
	}

   	cerr << "pointsize " << f_pointsize << " stepsize " << f_stepsize << " separation " << f_separation << " skipper " << f_skipper << endl;

}

void display()
{
	int x = (f_stepsize * f_step) % f_w;
	int npts = f_h / f_separation;
	if (npts == 0) npts = 1;

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	if (0 == f_primitive)
	{
		//cout << f_step << " " << f_skipper << " " << f_step%f_skipper << endl;
		if (f_step % f_skipper == 0)
		{
			glPointSize(f_pointsize);

			glBegin(GL_POINTS);
			for (int i=0; i<npts; i++)
				glVertex2f(x, (i+0.5)*f_separation);
			glEnd();
		}
	}
	else
	{
		int w = f_stepsize/2;
		int h = f_separation/2;
		glBegin(GL_TRIANGLES);
		for (int i=0; i<npts; i++)
		{
			glVertex2f(x, (i+0.5)*f_separation);
			glVertex2f(x+w, (i+0.5)*f_separation);
			glVertex2f(x, (i+0.5)*f_separation + h);
		}
		glEnd();
	}
	glFlush();
	f_step++;
	if (f_step % 500 == 0)
	{
		double sec = glfwGetTime();
		cerr << "Frames " << f_step << " time " << sec << " rate " << (double)f_step/sec << " FPS" << endl;
	}
}


int main(int argc, char **argv)
{
    GLFWwindow* window;

	if (argc < 6)
	{
		cerr << "usage (all params int): render-dots width height pointsize separation stepsize" << endl;
		return -1;
	}
	else
	{
		f_w = atoi(argv[1]);
		f_h = atoi(argv[2]);
		f_pointsize = atoi(argv[3]);
		f_separation = atoi(argv[4]);
		f_stepsize = atoi(argv[5]);
		cerr << "w " << f_w << " h " << f_h << " pointsize " << f_pointsize << " separation " << f_separation << " stepsize " << f_stepsize << " timerMS " << endl;

	}

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);


    if (f_w < 0 || f_h < 0)
    {
    	GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primary);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
		f_w = mode->width;
		f_h = mode->height;
		cerr << "current window rate " << mode->refreshRate << endl;
		window = glfwCreateWindow(mode->width, mode->height, "No border", primary, NULL);
    }
    else
    {
    	window = glfwCreateWindow(f_w, f_h, "Simple example", NULL, NULL);
    }

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    // Test check for extensions
    if (glfwExtensionSupported("GLX_OML_sync_control") == GL_TRUE)
    {
    	int64_t ust=0, msc=0, sbc=0;
    	Display* d;
    	Window w;
    	cerr << "GLX_OML_sync_control supported " << endl;
    	w = glfwGetX11Window(window);
    	d = glfwGetX11Display();
    	PFNGLXGETSYNCVALUESOMLPROC f = (PFNGLXGETSYNCVALUESOMLPROC)glfwGetProcAddress("glXGetSyncValuesOML");
    	int i = f(d, w, &ust, &msc, &sbc);
    	cerr << "ust " << ust << " msc " << msc << " sbc " << sbc << endl;
    }


    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    glfwSetTime(0.0);
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, f_w, 0, f_h, -1.0, 1.0);

        display();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


#if 0

#include <GL/glut.h>


using namespace std;

int f_w, f_h;
int f_pointsize;
int f_separation;
int f_stepsize;
int f_step = 0;  // incremented each time through display()
int f_timerMS = 16;
bool f_quit = false;


void reshape(GLint w, GLint h)
{
  glViewport(0, 0, w, h);
  GLfloat aspect = (GLfloat)w / (GLfloat)h;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, f_w, 0, f_h, -1.0, 1.0);
}
void display()
{
	int x = (f_stepsize * f_step) % f_w;
	int npts = f_h / f_separation;
	if (npts == 0) npts = 1;

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(f_pointsize);

	glBegin(GL_POINTS);
	for (int i=0; i<npts; i++)
		glVertex2f(x, (i+0.5)*f_separation);
	glEnd();
	glFlush();
	glutSwapBuffers();
	f_step++;
}

void timer(int v)
{
	glutPostRedisplay();
	glutTimerFunc(f_timerMS, timer, v);
}


void keyboard(unsigned char key,int x, int y)
{
	switch(key)
	{
	case 'p':
		if (f_pointsize > 1) f_pointsize--;
		break;
	case 'P':
		f_pointsize++;
		break;
	case 's':
		if (f_stepsize > 1) f_stepsize--;
		break;
	case 'S':
		f_stepsize++;
		break;
	case 'n':
		if (f_separation > 10) f_separation--;
		break;
	case 'N':
		if (f_separation < f_h/2) f_separation++;
		break;
	case 't':
		if (f_timerMS > 10) f_timerMS--;
		break;
	case 'T':
		f_timerMS++;
		break;
	case 'q':
	case 'Q':
		exit(0);
		break;
	}

	cerr << "pointsize " << f_pointsize << " stepsize " << f_stepsize << " timerMS " << f_timerMS << " separation " << f_separation << endl;
}


// Initializes GLUT, the display mode, and main window; registers callbacks;
// enters the main event loop.
int main(int argc, char** argv)
{
	if (argc < 6)
	{
		cerr << "usage (all params int): glutfs w h pointsize separation stepsize" << endl;
		return -1;
	}
	else
	{
		f_w = atoi(argv[1]);
		f_h = atoi(argv[2]);
		f_pointsize = atoi(argv[3]);
		f_separation = atoi(argv[4]);
		f_stepsize = atoi(argv[5]);
		if (argc == 7)
		{
			f_timerMS = atoi(argv[6]);
		}
		cerr << "w " << f_w << " h " << f_h << " pointsize " << f_pointsize << " separation " << f_separation << " stepsize " << f_stepsize << " timerMS " << f_timerMS << endl;

	}

	// double buffer, RGBA window in upper left hand corner.
	// Getwidth, height from command line
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(f_w, f_h);

	glutCreateWindow("glutfs");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(100, timer, 0);
	glutMainLoop();
}

#endif

// Transform assignment
// Name: Frank P Haecker
// NetID: fph7
/*
Additional dependencies include the following:
gdi32.lib, opengl32.lib, glew32.lib, glu32.lib, glfw3.lib

The image of the robot is read-in and color rendered.
The default view is perspective.

Commands:
'F': Swaps between perspective and orthographic view.
'H': Resets to the default matrix view.
Up Arrow: Drive robot Forward
Down Arrow: Drive robot Backwards
Left Arrow: Turn robot left
Right Arrow: Turn robot right
'W': Flip robot 90 degrees forward
'S': Flip robot 90 degrees downward
'A': Rotate robot 90 degrees onto it's left side
'D': Rotate robot 90 degrees onto it's right side

//robot images sources for texture mapping
//robot image source: http://blog.spoongraphics.co.uk/tutorials/create-a-cool-vector-robot-character-in-illustrator
//other robot image source:http://www.clker.com/clipart-cartoon-robot-4.html

Shader code adapted from example at:
http://antongerdelan.net/opengl/hellotriangle.html
*/
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "robot_geometry.hpp"
#include <math.h>
#include <Windows.h>
#include "BMPLoader.h"

#define PI 3.14159265

using namespace std;

// Global variables
GLuint vertexShaderID,fragShaderID, programID,bufferID, arrayID,indexBufferID;
GLint viewportLoc,colorLoc,orthoLoc,modelLoc;
int width=1024,height=600;
GLuint legTexture;
GLuint chestTexture;
GLuint chestTexture2;
GLuint faceTexture;
GLuint genericTexture;
int passes = 0;
float avgx,avgy,avgz,tireAngle;
MiniGeometry mini;     // The mini geometry data
const unsigned short* indices;

// Map of part names to colors
struct ColorEntry
{
	string part;
	float color[3];
	GLuint* texture;
};
ColorEntry colormap[] {
	{ string("LeftClawNail1"), { 1, 0, 0 }, &genericTexture },
	{ string("LeftClawNail2"), { 1, 0, 0 }, &genericTexture },
	{ string("RightClawNail2"), { 1, 0, 0 }, &genericTexture },
	{ string("RightClawNail1"), { 1, 0, 0 }, &genericTexture },
	{ string("LeftClaw2"), { 1, 0, 0 }, &genericTexture },
	{ string("LeftClaw1"), { 1, 0, 0 }, &genericTexture },
	{ string("RightClaw1"), { 1, 0, 0 }, &genericTexture },
	{ string("RightClaw2"), { 1, 0, 0 }, &genericTexture },
	{ string("LeftFoot"), { 1, 1, 1 }, &genericTexture },
	{ string("RightFoot"), { 1, 1, 1 }, &genericTexture },
	{ string("LeftArm"), {1,1,1}, &genericTexture },
	{ string("RightArm"), { 1, 1, 1 }, &genericTexture },
	{ string("LeftForearm"), { 1, 1, 1 }, &genericTexture },
	{ string("RightForearm"), { 1, 1, 1 }, &genericTexture },
	{ string("Mouth"), { 1, 1, 0 }, 0},
	{ string("RightEye"), { 1, 1, 0 }, 0},
	{ string("LeftEye"), { 1, 1, 0 }, 0},
	{ string("Head"), { 1, 1, 1 }, &genericTexture },
	{ string("Neck"), { 1, 1, 1 }, &genericTexture },
	{ string("TorsoFront"), { 1, 1, 1 }, &chestTexture},
	{ string("Face"), { 0.8, 0.8, 0.8 }, &faceTexture },
	{ string("Torso"), { 1, 1, 1 }, &genericTexture },
	{ string("RightLeg"), { 0.6, 0.6, 0.7 }, &legTexture },
	{ string("LeftLeg"), {0.6,0.6,0.7}, &legTexture},
	{ string("White"), { 1, 1, 1 }, 0},
	{ string("Yellow"), { 1, 1, 0 }, 0},
	{ string("Purple"), { 1, 0, 1 }, 0},
	{ string("Red"), { 1, 0, 0 }, 0},
	{ string("Teal"), { 0, 1, 1 }, 0},
	{ string("Green"), { 0, 1, 0 }, 0},
	{ string("Blue"), { 0, 0, 1 }, 0},
	{ string("Black"), { 0, 0, 0 }, 0},
};
ColorEntry getColor(string name)
{
	ColorEntry entry;
	for (int i = 0; i < 30; i++)
	{
		entry = colormap[i];
		if (entry.part == name)
			return entry;
	}
	entry.part = "none";
	entry.color[0] = 0;
	entry.color[1] = 0;
	entry.color[2] = 0;
	return entry;
}

GLfloat defaultView[16] {
	0.000242748894, -8.71418280e-013, 0.000000000, 0.000000000,
		1.56410564e-021, 1.30712747e-012, -0.000242748923, 0,
		8.71418280e-013, 0.000242748894, 4.39256530e-012, 0,
		0, 0.0145649351, -0.498511791, 1
}; 

float *positionData;
float rAngle = 90;
float tAngle = 15;
float c = cos(rAngle * (PI / 180));
float s = sin(rAngle * (PI / 180));
float ct = cos(tAngle *(PI / 180));
float st = sin(tAngle*(PI / 180));
bool perspective = 1; 
GLfloat aspect = (GLfloat)width / (GLfloat)height; // Compute aspect ratio of window

float zRotR[] {
	ct, st, 0, 0,
	-st, ct, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};
float zRotL[] {
	ct, -st, 0, 0,
	st, ct, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};
float xRotR[]{
	1, 0, 0, 0,
	0, c, -s, 0,
	0, s, c, 0,
	0, 0, 0, 1
};
float xRotL[] {
	1, 0, 0, 0,
	0, c, s, 0,
	0, -s, c, 0,
	0, 0, 0, 1
};
float yRotL[] {
	c, 0, -s, 0,
	0, 1, 0, 0,
	s, 0, c, 0,
	0, 0, 0, 1
};
float yRotR[] {
	c, 0, s, 0,
	0, 1, 0, 0,
	-s, 0, c, 0,
	0, 0, 0, 1
};
float translateDown[] {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 20, 0, 1
};
float translateUp[] {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, -20, 0, 1
};
float translateLeft[] {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	-1, 0, 0, 1
};
float translateRight[] {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	1, 0, 0, 1
};
float scaleUp[] {
	1.03, 0, 0, 0,
	0, 1.03, 0, 0,
	0, 0, 1.03, 0,
	0, 0, 0, 1
};
float scaleDown[] {
	0.90, 0, 0, 0,
	0, 0.90, 0, 0,
	0, 0, 0.90, 0,
	0, 0, 0, 1
};
float viewScale[] {
	2, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 2, 0,
		0, 0, 0, 1
};

////////////////////////////////////////////////////
// Prototypes

// Prototypes: Rendering system management
void init();
void cleanup();

// Prototypes: Vertex buffer management
void createBuffers();
void destroyBuffers();

// Prototypes: Shader management
void createShaders();
void destroyShaders();

// Prototypes: Rendering and window callbacks
void render();

typedef void(*GLFWwindowsizefun)(GLFWwindow*, int, int);

////////////////////////////////////////////////////
GLFWwindow* window;
float g_angle = 15.0;
float g_dist = 10;

// Window resized. Tell OpenGL to map a all of the window space
static void window_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	// Always redraw
	render();
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Only worry when released
	if (action == GLFW_PRESS)
		return;

	if (key == GLFW_KEY_ESCAPE)
	{
		cleanup();
		exit(EXIT_SUCCESS);
	}
	else if (key == 'Q')
	{

	}
	else if (key == 'W')
	{
		glMultMatrixf(xRotL);
	}
	else if (key == 'A')
	{
		glMultMatrixf(yRotL);
	}
	else if (key == 'S')
	{
		glMultMatrixf(xRotR);
	}
	else if (key == 'D')
	{
		glMultMatrixf(yRotR);
	}
	else if (key == GLFW_KEY_UP)
	{
		glMultMatrixf(translateUp);
	}
	else if (key == GLFW_KEY_LEFT)
	{
		glMultMatrixf(zRotR);
	}
	else if (key == GLFW_KEY_DOWN)
	{
		glMultMatrixf(translateDown);
	}
	else if (key == GLFW_KEY_RIGHT)
	{
		glMultMatrixf(zRotL);
	}
	else if (key == 'H')//reset the transformations
	{
		glLoadIdentity();
		glMultMatrixf(defaultView);
	}
	else if (key == 'F')//swap views
	{
		if (!perspective)//if not perspective, make it perspective
		{
			perspective = 1;
			glMatrixMode(GL_PROJECTION); // To operate on the Projection matrix
			glPushMatrix();
			glLoadIdentity();
			gluPerspective(45.0f, aspect, 0.001f, 10000.0f); // Perspective projection: fovy, aspect, near, far
			glMatrixMode(GL_MODELVIEW);
		}
		else
		{
			perspective = 0;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glRotatef(180, 0, 1, 0);
			glMultMatrixf(viewScale);
			glMatrixMode(GL_MODELVIEW);
		}
	}
	else if (key == GLFW_KEY_KP_ADD)
	{
		glMultMatrixf(scaleUp);
	}
	else if (key == GLFW_KEY_KP_SUBTRACT)
	{
		glMultMatrixf(scaleDown);
	}
}
void reshape_func(int width, int height)
{
	glViewport(0, 0, width, height);
}

void setupTextures()
{
	LoadBMP("robotChest.bmp", chestTexture);
	LoadBMP("robotLeg.bmp", legTexture);
	LoadBMP("robotHead.bmp", faceTexture);
	LoadBMP("robotGeneric.bmp", genericTexture);
	LoadBMP("robotChest2.bmp", chestTexture2);
}

// Create context, window, and OGL buffers
void init()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		cerr << "glfw failed to initialize" << endl;
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(width, height, "Robot Model", NULL, NULL);
	if (!window)
	{
		cerr << "glfw failed to create window" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}


	//This function makes the context of the specified window current on the calling thread.   
	glfwMakeContextCurrent(window);

	//Sets the key callback  
	glfwSetKeyCallback(window, key_callback);

	//Initialize GLEW  
	GLenum err = glewInit();

	//If GLEW hasn't initialized  
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
		//return -1;
	}

	// Initialize GL State
	glClearColor(1, 1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glMatrixMode(GL_PROJECTION);//used to modify camera
	glLoadIdentity();//loads the identity matrix
	glPushMatrix();
	gluPerspective(45.0f, aspect, 0.001f, 10000.0f); // Perspective projection: fovy, aspect, near, far
	glMatrixMode(GL_MODELVIEW);//how objects are transformed

	// Initialize vertex buffers, the vertex array, and the shaders
	createBuffers();
	createShaders();
	setupTextures();

	glfwSetWindowSizeCallback(window, window_size_callback);
	// Setup callbacks
	glfwPollEvents();
}
// Clean up: Shut down window system
void cleanup()
{
	// Clean up OpenGL (vertex buffers, shaders, etc.)
	destroyShaders();
	destroyBuffers();

	// Close window and terminate GLFW
	glfwTerminate();    
}

void createBuffers()
{
	indices = mini.indices();
	positionData = new float[mini.numVertices() * 3];
	for (int i = 0; i < mini.numVertices(); i++)
	{
		positionData[i*3] = mini.vertexdata()[i*3];
		positionData[i*3+1] = mini.vertexdata()[i*3+1];
		positionData[i*3+2] = mini.vertexdata()[i*3+2];
		//out << positionData[i * 3] << " " << positionData[i * 3 + 1] << " " << positionData[i * 3 + 2] << endl;
	}

	int dataSize = mini.numVertices() * 3 * sizeof(float);
	glGenBuffers(1, &bufferID);//creates 1 buffer at bufferID
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);//use bufferID whenever GL_ARRAY_BUFFER is needed
	glBufferData(GL_ARRAY_BUFFER, dataSize, positionData, GL_STATIC_DRAW);//stores vertexData in GL_ARRAY_BUFFER

	glGenVertexArrays(1, &arrayID);//1 vertex array object created at arrayID
	glBindVertexArray(arrayID);//binds arrayID as a vertex array
	glEnableVertexAttribArray(0);//enables the vertex array at index 0, tell opengl to use vertex attribute arrays
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0);//specifies the location and data format of vertex attributes at index

	int indicesSize = mini.numFaces() * 4 * sizeof(float);
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

	glMultMatrixf(defaultView);//set default view
}
//After you are done w/ your buffers, you have to deallocate them
void destroyBuffers()
{
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &indexBufferID);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &arrayID);
}
void createShaders()
{
	/*Shader code adapted from example
	at http://antongerdelan.net/opengl/hellotriangle.html
	*/
	const char* vertex_shader =
		"#version 400\n"
		"uniform vec3 color;"
		"out vec3 outColor;"
		"in vec3 vp;"
		"void main () {"
		"  gl_Position = vec4 (vp, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 400\n"
		"in vec3 outColor;"
		"out vec4 frag_colour;"
		"void main () {"
		"  frag_colour = vec4 (color, 1.0);"
		"}";

	colorLoc = glGetUniformLocation(programID, "color");
	glUniform3f(colorLoc, 1, 1, 1);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_shader, NULL);
	glCompileShader(vertexShader);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment_shader, NULL);
	glCompileShader(fragmentShader);

	GLuint programID = glCreateProgram();
	glAttachShader(programID, fragmentShader);
	glAttachShader(programID, vertexShader);
	glLinkProgram(programID);
}
// Disable the shaders
void destroyShaders()
{
	// Disable the current program
	glUseProgram(0);

	// Detach the shaders
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragShaderID);

	// Destroy the shaders
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragShaderID);

	// Destroy the program
	glDeleteProgram(programID);
}

void sortQuadraticPoints()
{

}

// Render OpenGL, We just clear the screen and draw the line
void render()
{
	// OpenGL rendering goes here...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	list<PartEntry> groups = mini.groups();
    list<PartEntry>::const_iterator it;
	PartEntry entry;
	ColorEntry temp;
	int offset, count;
	string name;
	passes++;
	for (it = groups.begin(); it != groups.end(); ++it)
	{
		entry = *it;
		name = entry.name;
		offset = sizeof(unsigned short)* 4 * entry.start;
		count = 4 * (entry.end - entry.start);
		temp = getColor(entry.name);
		glColor3f(temp.color[0], temp.color[1], temp.color[2]);
		//check for defined texture mapping
		if (temp.texture != 0)
			{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, *temp.texture);
			if(entry.name == "TorsoFront")
			{
				if (passes > 10)
					glBindTexture(GL_TEXTURE_2D, chestTexture2);
				if (passes > 20)
					passes = 0;
			}
			float* s = positionData;
			for (int i = entry.start; i < entry.end; i++)
			{
				glBegin(GL_POLYGON);
				int i1 = indices[i * 4], i2 = indices[i * 4 + 1], i3 = indices[i * 4 + 2], i4 = indices[i * 4 + 3];
				glTexCoord2d(0.0, 0.0); glVertex3d(positionData[i1 * 3], positionData[i1 * 3 + 1], positionData[i1 * 3 + 2]);
				glTexCoord2d(1.0, 0.0); glVertex3d(positionData[i2 * 3], positionData[i2 * 3 + 1], positionData[i2 * 3 + 2]);
				glTexCoord2d(1.0, 1.0); glVertex3d(positionData[i3 * 3], positionData[i3 * 3 + 1], positionData[i3 * 3 + 2]);
				glTexCoord2d(0.0, 1.0); glVertex3d(positionData[i4 * 3], positionData[i4 * 3 + 1], positionData[i4 * 3 + 2]);
				glEnd();
			}
			
			}
		else//otherwise, simply color and render
			{
			glDisable(GL_TEXTURE_2D);
			for (int i = 0; i < entry.end-entry.start; i++)
				{
				glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_SHORT, (void*)offset);
				offset += sizeof(unsigned short)* 4;
				}
			}
	}

	glfwPollEvents();
	// Swap front and back rendering buffers
	glfwSwapBuffers(window);
}
// Entry point
int main(int argc, char** argv) 
{
	// Initialize OpenGL and create our window
	init();

	// Main loop
	int running = GL_TRUE; 
	do
	{
		// Render the scene 
		glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
		render();
	} 
	while (!glfwWindowShouldClose(window));


	// Exit program
	cleanup();
	return EXIT_SUCCESS;
}


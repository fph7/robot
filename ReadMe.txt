Program compiled using C++ Visual Studio 2013
Additional dependencies include the following:
gdi32.lib, opengl32.lib, glew32.lib, glu32.lib, glfw3.lib
Image files necessary for texture mapping are included.
BMPLoader.cpp and BMPLoader.h are included and required to load BMP files as resources.

I have created and defined a model for a robot. The robot model has been rendered with numerous textures
mapped to it. A variety of commands are available to manipulate the robot.

Commands:
'F': Swaps between perspective and orthographic view.
'H': Resets to the default matrix view.
Up Arrow: Move robot Forward
Down Arrow: Move robot Backwards
Left Arrow: Turn robot left
Right Arrow: Turn robot right
'W': Flip model 90 degrees forward
'S': Flip model 90 degrees downward
'A': Rotate model 90 degrees onto it's left side
'D': Rotate model 90 degrees onto it's right side
'-': Zoom out
'+': Zoom in
ESC: exit program

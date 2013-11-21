CSCI 480 Computer Graphics
Assignment 2: Simulating a Roller Coaster
Author: Tianyi Luo
USC ID: 8272332523
Developing Environment: Mac OS starter kit

How to run program:
	Please ensure that the images folder is located in the same folder as assign2.cpp. You can pick the spline file and change the file name in the track.txt. This program runs great with "goodRide" and "rollerCoaster" spline models.

	Compile: use Makefile the starter code provides.
	Run this program in Mac OS X Terminal app as: ./assign2 track.txt

Basic features: 
	Level 1: Spline curves. Represented by two rails.
	Level 2: Ground. Implemented as cloud to achieve the 360 degree panorama scene above the cloud.
	Level 3: Sky. Implemented as parts of the 360 degree panorama scene above the cloud.
	Level 4: More realistic tracks. Implemented two rails and cross sections are textured by wood.
	Level 5: Manipulate the camera. Implemented the camera moving in a realistic manner.

Extra credit features:
	1. There is no seams in the skybox. I am using a series of environment pictures consisted as following. And I then can combine them into one box with no seams in it.
		  5
		1 2 3 4
		  6

	2. The roller coaster's car is physically realistic, because now it has acceleration variations. I am using the formula: 
		u_new = u_old + (dt)(sqrt(2gh)/mag(dp/du))
	This can be observed better when using the upsidedown.sp spline model.

	3. Make my track circular and close it with C1 continuity.
		The track is based on C1 continuity, because I am using Catmull-Rom. And the circular track can be observed by using circle.sp spline model

	4. I have implemented these additional features of my track.
		1) double rails: first rail, splinePoint-d/2*normal, second rail splinePoint+d/2*normal. d is the distance between two rails.
		2) wooden crossbar: 6 sides are textured with wood picture.
		3) support structure: I have implemented to render columns in each control point. But the default animation is without rendering support structure, because when the roller coaster's car heads downward, you will feel you are hitting columns. Press 'c' can open the support structure rendering mode.

Keyboard functions:
	- 'q' = 'Q': quit the application
	- 'a' = 'A': start or stop the animation screenshots
	- 'c' = 'C': open or close the support structure rendering
	- 'l' = 'L': turn the light mode on or off

Mouse functions:
	Rotate: drag the mouse
	Translate: Press "Ctrl", and drag the mouse
	Scale: Press "Shift", and drag the mouse

Other application functions:
	Right click: pop up menu for user to quit
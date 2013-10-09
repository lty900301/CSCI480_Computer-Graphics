CSCI 480 Computer Graphics
Assignment 1: Height Fields
Author: Tianyi Luo
USC ID: 8272332523

Transform functions:
	Rotate: drag the mouse
	Translate: Press "Ctrl", and drag the mouse
	Scale: Press "Shift", and drag the mouse

Keyboard functions:
	- 'q' = 'Q': quit the application
	- 'p' = 'P': render the height field as points
	- 'l' = 'L': render the height field as lines
	- 'f' = 'F': render the height field as solid triangles
	- 'o' = 'O': render the height field as wireframe on top of solid triangles
	- 'i' = 'I': render or cancel the bpp3 height field into blue scale
	- 'c' = 'C': open or close to color the vertices based on color values taken from another image of equal size. This image may passed in by a second argument. e.g. ./assign1 spiral.jpg colormesh.jpg
	- 'g' = 'G': open or close the light, it lit the above face of the object.
	- 'a' = 'A': start or stop the animation screenshots

Other application functions:
	Right click: pop up menu for user to quit

Extra credit functions realized:
	1. Render wireframe on top of solid triangles (use glPolygonOffset to avoid z-buffer fighting). (use key 'o' as above)
	2. Support color (bpp=3) in input images. (use key 'i' as above) First, parse in the color image as argument. Then press 'i', it will render according to RGB three values instead of only 1 value.
	3. Color the vertices based on color values taken from another image of equal size. (parse in two arguments, and use key 'c' as above)
	4. Experiment with material and lighting properties. (use key 'g' as above).

References:
	http://profs.sci.univr.it/~colombar/html_openGL_tutorial/en/06depth_014.html
	http://www.glprogramming.com/red/chapter05.html
	http://www.glprogramming.com/red/chapter06.html
	http://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/
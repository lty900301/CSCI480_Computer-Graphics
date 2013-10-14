/*
  CSCI 480 Computer Graphics
  Assignment 1: Height Fields
  C++ starter code
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* Define the windows size */
const int WIDTH = 640;
const int HEIGHT = 480;
const GLdouble FOVY = 60.0;

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;
Pic * g_pColorData;

GLenum toRender = GL_TRIANGLE_STRIP; // how to render surface

// if enable the glPolygonOffset to avoid z-fighting
bool offset = false;
bool bpp3Drawing = false;
bool colorRender = false;
bool light = false;
bool animation = false;

//Material
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 50.0 };
// light position
GLfloat light_position[] = { 0.0, 0.0, 1.5, 1.0 };

int screenShotsCount = 0;





/***** Display methods *****/

void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOVY, WIDTH / HEIGHT, 0.1, 1000.0);
  glMatrixMode(GL_MODELVIEW);

  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_DEPTH_TEST);
}

// To discuss by catogarizing the rendering method
// THis method is not good though. It takes more code than below
// void drawHF2(){
//   int width = g_pHeightData->nx, height = g_pHeightData->ny;
  
//   // decide how the vertices are going to be drawn
//   if(toRender == GL_POINTS){
//     glBegin(toRender);
//     for(int z = 0; z < height; z++){
//       for(int x = 0; x < width; x++){
//         GLfloat heightVal = PIC_PIXEL(g_pHeightData, x, z, 0);
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*z/height-1, heightVal/255.0);
//       }
//     }
//     glEnd();
//   } 

//   else if (toRender == GL_LINES) {
//     glBegin(toRender);
//     for(int z=0; z<height-1; z++) {
//       for(int x=0; x<width-1; x++) {

//         // The Top Left Vertex
//         GLfloat heightVal = PIC_PIXEL(g_pHeightData, x, z, 0);
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*z/height-1, heightVal/255.0);

//         // The Bottom Left Vertex
//         heightVal = PIC_PIXEL(g_pHeightData, x, z+1, 0); 
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*(z+1)/height-1, heightVal/255.0);

//         // The Top Right Vertex
//         heightVal = PIC_PIXEL(g_pHeightData, x+1, z, 0); 
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*(x+1)/width-1, (GLfloat)2*z/height-1, heightVal/255.0);

//         // The Top Left Vertex
//         heightVal = PIC_PIXEL(g_pHeightData, x, z, 0);
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*z/height-1, heightVal/255.0);
//       }// next pixel in current row
//     }// next row
//     glEnd();
//   } 

//   else if (toRender == GL_TRIANGLE_STRIP) {
//     for(int z=0; z<height-1; z++) {
//       glBegin(toRender);
//       for(int x=0; x<width; x++) {
//         GLfloat heightVal = PIC_PIXEL(g_pHeightData, x, z, 0);
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*z/height-1, heightVal/255.0);

//         heightVal = PIC_PIXEL(g_pHeightData, x, z+1, 0); 
//         glColor3f(heightVal/255.0, heightVal/255.0, 1.0);
//         glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*(z+1)/height-1, heightVal/255.0);
//       }// next pixel in current row
//       glEnd();
//     }// next row
//   }
// }

GLfloat getColor(Pic *g_pHeightData, int x, int z){
  if(bpp3Drawing) {
    /* The luminosity method is a more sophisticated version of the average method.
     * It also averages the values, but it forms a weighted average to account for
     * human perception. We’re more sensitive to green than other colors, so green
     * is weighted most heavily. The formula for luminosity is
     * 0.21 R + 0.71 G + 0.07 B.
     */
    unsigned char red = PIC_PIXEL(g_pHeightData, x, z, 0);
    unsigned char green = PIC_PIXEL(g_pHeightData, x, z, 1);
    unsigned char blue = PIC_PIXEL(g_pHeightData, x, z, 2);
    return (GLfloat)(0.21*red+0.71*green+0.07*blue)/(255.0);
  }
  
  return (GLfloat) PIC_PIXEL(g_pHeightData, x, z, 0) / 255.0;
}

void drawColorHF(){
  int width = g_pHeightData->nx, height = g_pHeightData->ny;
  for(int z = 0; z < height; z++){
    glBegin(GL_TRIANGLE_STRIP);
    for(int x = 0; x < width; x++){
      // Draw the current node
      unsigned char red = PIC_PIXEL(g_pColorData, x, z, 0);
      unsigned char green = PIC_PIXEL(g_pColorData, x, z, 1);
      unsigned char blue = PIC_PIXEL(g_pColorData, x, z, 2);
      glColor3f(red/255.0, green/255.0, blue/255.0);
      glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*z/height-1, 
      getColor(g_pHeightData, x, z));

      if(toRender != GL_POINTS){
        if(z == height-1) break;
        red = PIC_PIXEL(g_pColorData, x, z+1, 0);
        green = PIC_PIXEL(g_pColorData, x, z+1, 1);
        blue = PIC_PIXEL(g_pColorData, x, z+1, 2);
        glColor3f(red/255.0, green/255.0, blue/255.0);
        glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*(z+1)/height-1, 
        getColor(g_pHeightData, x, z));
      }
    }
    glEnd();
  }
}

/* Height Field drawing function */
// Much easier way to implement
void drawHF(){
  int width = g_pHeightData->nx, height = g_pHeightData->ny;
  for(int z = 0; z < height; z++){
    glBegin(GL_TRIANGLE_STRIP);
    for(int x = 0; x < width; x++){
      // Draw the current node
      GLfloat color = getColor(g_pHeightData, x, z);
      glColor3f(color, color, 1.0);
      glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*z/height-1, color);

      if(toRender != GL_POINTS){
        if(z == height-1) break;
        GLfloat color = getColor(g_pHeightData, x, z+1);
        glColor3f(color, color, 1.0);
        glVertex3f((GLfloat)2*x/width-1, (GLfloat)2*(z+1)/height-1, color);
      }
    }
    glEnd();
  }
}

/* display callback */
void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  /* Transform to the current world state */
  glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
  glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
  glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
  glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
  glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

  // draw the height filed image

  if(light){
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
  } else {
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
  }

  if (offset) { // Render wireframe on top of solid triangles
    // Draw solid Triangles and set a light to remove the solid color
    toRender = GL_TRIANGLE_STRIP;
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (colorRender) drawColorHF();
    else drawHF();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    //Draw wireframes
    toRender = GL_LINES;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (colorRender) drawColorHF();
    else drawHF();
    // Disable offset
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } 

  else {  // Draw according to toRender
    if (colorRender) drawColorHF();
    else drawHF();
  }

  glFlush();
  glutSwapBuffers();
}

void doIdle()
{
  /* make the screen update */
  glutPostRedisplay();
}






/***** Interaction callback methods *****/
/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01; // ?
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/* keyboard callback */
void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q': case 'Q':
    offset = false;
    exit(0);
    break;
  case 'p': case 'P':
    offset = false;
    toRender = GL_POINTS;
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;
  case 'l': case 'L':
    offset = false;
    toRender = GL_LINES;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case 'f': case 'F':
    offset = false;
    toRender = GL_TRIANGLE_STRIP;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case 'o': case 'O':
    offset = true;
    break;
  case 'i': case 'I':
    bpp3Drawing = !bpp3Drawing;
    break;
  case 'c': case 'C':
    colorRender = !colorRender;
    if(!g_pColorData) colorRender = false;
    break;
  case 'g': case 'G':
    light = !light;
    break;
  case 'a': case 'A':
    animation = !animation;
    break;
  default: break;
  }
  glutPostRedisplay();
}





/***** Helper Method *****/
/* set projection to aspect ratio of window */
void reshape(int w, int h){
  GLfloat aspect = (GLfloat) w / (GLfloat) h;
  /* scale viewport with window */
  glViewport(0, 0, w, h);

  /* prevent distortion */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (w <= h) /* aspect <= 1 */
    glOrtho(-2.0, 2.0, -2.0 / aspect, 2.0 / aspect, -10.0, 10.0);
  else /* aspect > 1 */
    glOrtho(-2.0 * aspect, 2.0 * aspect, -2.0, 2.0, -10.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void timer(int value){
  switch(value){
    case 0: 
      if(!animation) break;
      char filename[40];
      sprintf(filename, "../screenshots/%03d.jpg", screenShotsCount);
      saveScreenshot(filename);
      screenShotsCount++;
      if(screenShotsCount == 300) animation = false;
      break;
    default:
      break;
  }
  glutTimerFunc(100, timer, 0);
}







int main (int argc, char ** argv)
{
  int argvCount = 0;
  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }
  argvCount++;

  if(strcmp(argv[argvCount],"-a") == 0) {
    animation = true;
    argvCount++;
  }

  g_pHeightData = jpeg_read(argv[argvCount], NULL);
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[argvCount]);
    exit(1);
  }
  argvCount++;

  if(argc > argvCount){
    g_pColorData = jpeg_read(argv[argvCount], NULL);
    if (!g_pColorData) {
      printf ("error reading %s.\n", argv[argvCount]);
      exit(1);
    }
    colorRender = true;
  }

  glutInit(&argc,argv);
  /*
    create a window here..should be double buffered and use depth testing
    the code past here will segfault if you don't have a window set up....
    replace the exit once you add those calls.
  */
  // exit(0);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutCreateWindow("Assignment 1 - Height Field");

  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);

  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  /* replace with any animate code */
  glutIdleFunc(doIdle);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);

  /* callback for when the windows size has rechaped */
  glutReshapeFunc(reshape);

  /* callback for keybaord */
  glutKeyboardFunc(keyboard);

  /* callback for a timer to save screenshots */
  glutTimerFunc(100, timer, 0);

  /* do initialization */
  myinit();

  glutMainLoop();
  return(0);
}
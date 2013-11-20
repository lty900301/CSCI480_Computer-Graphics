/*
  CSCI 480
  Assignment 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <math.h>
using namespace std;

/** Define the physics static variable **/
const double G = 9.78;

/** Define the state of the window information **/
const int WIDTH = 640; // Define the window's width
const int HEIGHT = 480; // Define the window's height
const char* WIN_TITLE = "Assignment 2: Simulating a Roller Coaster";
const float WORLD_SCALE = 256.0;

/** Define the state of the camera **/
const GLdouble FOVY = 73.0; // Define the FOVY for camera

/** Define the variable used to control animation screenshots **/
int screenShotsCount = 0; // defined as the file WIN_TITLname of screenshots
bool animation = false; // defined as the control of taking screenshots

/** variables for interactions **/
int g_iMenuId;
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

/** state of the world **/
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/** data structures that are relevant to splines **/
struct point { // represents one control point along the spline
   double x;
   double y;
   double z;
};
// spline struct which contains how many control points, 
// and an array of control points
struct spline {
   int numControlPoints;
   struct point *points;
};
struct spline *g_Splines; // the spline array
int g_iNumOfSplines; // total number of splines 

/** data structures that are relevant to textures **/
// 0: up 1: down 2: front 3: back 4: right 5: left 6: wood
GLuint textures[] = {0, 1, 2, 3, 4, 5, 6};
const int SKY_UP = 0, SKY_DOWN = 1, SKY_FRONT = 2, SKY_BACK = 3, 
  SKY_RIGHT = 4, SKY_LEFT = 5, WOOD = 6;
char* TEXTURE_UP = "images/sky/up.jpg";
char* TEXTURE_DOWN = "images/sky/down.jpg";
char* TEXTURE_FRONT = "images/sky/front.jpg";
char* TEXTURE_BACK = "images/sky/back.jpg";
char* TEXTURE_RIGHT = "images/sky/right.jpg";
char* TEXTURE_LEFT = "images/sky/left.jpg";
char* TEXTURE_WOOD = "images/wood.jpg";

/** animation constant variables **/
const int NUM_TO_DIVIDE_U = 1000;
const double SPEED = 3.0;
const double TIMESTEP = SPEED / NUM_TO_DIVIDE_U;
const int CAR_LENGTH = 20; 
const double DISTANCE_BETWEEN_TRACKS = 0.2;
const double EYE_UP = DISTANCE_BETWEEN_TRACKS / 10 * 6;
const double RAIL_WIDTH_HEIGHT = DISTANCE_BETWEEN_TRACKS / 15;
const int DISTANCE_BETWEEN_CROSSES = NUM_TO_DIVIDE_U / 20;
const double CROSS_HEIGHT = DISTANCE_BETWEEN_TRACKS / 10;
const double CROSS_WIDTH = DISTANCE_BETWEEN_TRACKS / 3 * 5;
const int CROSS_LENGTH = DISTANCE_BETWEEN_CROSSES / 10 * 3; // should be even
const int COLUMN_LENGTH_WIDTH = 2; // should be even
/** animation variables **/
int animationTimer = 0;
double MAX_HEIGHT = 0.0;
double uCurrent = 0.0;
bool displayColumn = false;
// use these spaces to trade for less computation
int numOfCatMullSplinePoints;
point* splinePoints; 
point* firstSplinePoints; 
point* secondSplinePoints; 
point* splineTangents;
point* splineNormals;
point* splineBiNormals;
// use this vector as the initial sudo binormal used to calculate first normal
point V0;
// use these to store the displaylist
GLuint trackID, groundSkyID, crossID, columnID;

/** lighting information **/
bool lighting = true;
// Sun light
GLfloat light0_position[] = {0.0, 0.0, -1.0, 0.0};
GLfloat lignt0_ambient[] = {0.0, 0.0, 0.0, 0.0};
GLfloat light0_diffuse[] = {0.0, 1.0, 1.0, 1.0};
GLfloat light0_specular[] = {1.0, 1.0, 1.0, 1.0};



/***** Calculation methods *****/
/* This function implements the formula to calculate for spline point.
  References: http://www.mvps.org/directx/articles/catmull/*/
double catmullRomPointCalc(double p0, double p1, 
  double p2, double p3, double t) {
    return 0.5 * ((2*p1) +
                  (-p0 + p2) * t +
                  (2*p0 - 5*p1 + 4*p2 - p3) * pow(t,2) +
                  (-p0 + 3*p1 - 3*p2 + p3) * pow(t,3));
}

/* This function implements the formula to calculate for spline tangent.
  References: http://www.mvps.org/directx/articles/catmull/*/
/* deviation from catmullRom formula */
double catmullRomTangentCalc(double p0, double p1, 
  double p2, double p3, double t) {
    return 0.5 * ((-p0 + p2) +
                  (2*p0 - 5*p1 + 4*p2 - p3) * 2 * t +
                  (-p0 + 3*p1 - 3*p2 + p3) * 3 * pow(t,2)); 
}

/* This function normalize the vector */
point normalize(point p) {
  double length = sqrt(pow(p.x, 2) + pow(p.y, 2) + pow(p.z, 2));
  p.x /= length;
  p.y /= length;
  p.z /= length;
  return p;
}

/* This function add two vectors */
point addTwoVectors(point a, point b) {
  point c;
  c.x = a.x + b.x;
  c.y = a.y + b.y;
  c.z = a.z + b.z;
  return c;
}

/* This function performs cross product of two vectors */
point crossProduct(point a, point b) {
  point c;
  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;
  return c;
}



/***** Helper methods *****/

/* read the spline curve file into array */
int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
                  &g_Splines[j].points[i].x, 
                  &g_Splines[j].points[i].y, 
                  &g_Splines[j].points[i].z) != EOF) {
        i++;
    }
  }

  free(cName);

  return 0;
}

/* read the texture map file into array and bind it into the GLuint*/
void loadTexture(char *filename, GLuint textureID) {
  Pic* textureFile = jpeg_read(filename, NULL);
  if (textureFile == NULL) {
    printf("can't open image: %s.\n", filename);
    exit(1);
  }

  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_CLAMP);
  gluBuild2DMipmaps(GL_TEXTURE_2D, 4, textureFile->nx, 
    textureFile->ny, GL_RGB, GL_UNSIGNED_BYTE, textureFile->pix);
}

/* generate the spline's array informations */
void generateSplineInfo(spline* g_Splines) {
  int firstSpline = 0; 
  int numOfSplinePoints = g_Splines[firstSpline].numControlPoints;
  float uIncrement = 1.0 / NUM_TO_DIVIDE_U;
  // init the vector to calculate first normal
  V0.x = 0.0; V0.y = 0.0; V0.z = -1.0; 

  // initialize the spline's information Arrays
  splinePoints = new point[numOfSplinePoints * NUM_TO_DIVIDE_U];
  firstSplinePoints = new point[numOfSplinePoints * NUM_TO_DIVIDE_U];
  secondSplinePoints = new point[numOfSplinePoints * NUM_TO_DIVIDE_U];
  splineTangents = new point[numOfSplinePoints * NUM_TO_DIVIDE_U];
  splineNormals = new point[numOfSplinePoints * NUM_TO_DIVIDE_U];
  splineBiNormals = new point[numOfSplinePoints * NUM_TO_DIVIDE_U];

  /** brute force method **/
  // fill in all the spline's information arrays
  int arrayIndex = 0;
  for(int i = 0; i < numOfSplinePoints-3; i++) {
    for(float u = 0.0; u < 1.0; u += uIncrement) {
      point p0, p1, p2, p3;
      p0 = g_Splines[firstSpline].points[i];
      p1 = g_Splines[firstSpline].points[i+1];
      p2 = g_Splines[firstSpline].points[i+2];
      p3 = g_Splines[firstSpline].points[i+3];

      // calculate the spline point
      point splinePoint;
      splinePoint.x = catmullRomPointCalc(p0.x, p1.x, p2.x, p3.x, u);
      splinePoint.y = catmullRomPointCalc(p0.y, p1.y, p2.y, p3.y, u);
      splinePoint.z = catmullRomPointCalc(p0.z, p1.z, p2.z, p3.z, u);
      splinePoints[arrayIndex] = splinePoint;
      if(splinePoint.z > MAX_HEIGHT) MAX_HEIGHT = splinePoint.z;

      // calculate the spline tangent
      point splineTangent;
      splineTangent.x = catmullRomTangentCalc(p0.x, p1.x, p2.x, p3.x, u);
      splineTangent.y = catmullRomTangentCalc(p0.y, p1.y, p2.y, p3.y, u);
      splineTangent.z = catmullRomTangentCalc(p0.z, p1.z, p2.z, p3.z, u);
      splineTangents[arrayIndex] = splineTangent; // store the spline tangent
      // normalize tangent for following computation
      splineTangent = normalize(splineTangent); 

      // calculate the spline normal and binormal. Reference: 
      // https://dl.dropboxusercontent.com/u/91263185/CS480-f13/projects/
      // HW2/assign2_camera.html
      point splineNormal, splineBinormal;
      if(arrayIndex == 0) {
        point T0 = splineTangent;
        splineNormal = normalize(crossProduct(T0, V0));
        splineBinormal = normalize(crossProduct(T0, splineNormal));
      } else {
        point B0 = splineBiNormals[arrayIndex-1];
        point T1 = splineTangent;
        splineNormal = normalize(crossProduct(B0, T1));
        splineBinormal = normalize(crossProduct(T1, splineNormal));
      }
      splineNormals[arrayIndex] = splineNormal;
      splineBiNormals[arrayIndex] = splineBinormal;

      double HALF_DISTANCE_BETWEEN_TRACKS = DISTANCE_BETWEEN_TRACKS / 2.0;
      // Generate the first spline points
      point firstSplinePoint;
      firstSplinePoint.x = splinePoint.x - 
        HALF_DISTANCE_BETWEEN_TRACKS * splineNormal.x;
      firstSplinePoint.y = splinePoint.y - 
        HALF_DISTANCE_BETWEEN_TRACKS * splineNormal.y;
      firstSplinePoint.z = splinePoint.z - 
        HALF_DISTANCE_BETWEEN_TRACKS * splineNormal.z;
      firstSplinePoints[arrayIndex] = firstSplinePoint;
      // Generate the second spline points
      point secondSplinePoint;
      secondSplinePoint.x = splinePoint.x + 
        HALF_DISTANCE_BETWEEN_TRACKS * splineNormal.x;
      secondSplinePoint.y = splinePoint.y + 
        HALF_DISTANCE_BETWEEN_TRACKS * splineNormal.y;
      secondSplinePoint.z = splinePoint.z + 
        HALF_DISTANCE_BETWEEN_TRACKS * splineNormal.z;
      secondSplinePoints[arrayIndex] = secondSplinePoint;

      // increment the array's index
      arrayIndex++;
    }
  }
  numOfCatMullSplinePoints = arrayIndex;
}

/* generate the textures information */
void generateTextures() {
  // 0: up 1: down 2: front 3: back 4: right 5: left 6: wood
  glGenTextures(1, &textures[SKY_UP]);
  loadTexture(TEXTURE_UP, textures[SKY_UP]);
  glGenTextures(1, &textures[SKY_DOWN]);
  loadTexture(TEXTURE_DOWN, textures[SKY_DOWN]);
  glGenTextures(1, &textures[SKY_FRONT]);
  loadTexture(TEXTURE_FRONT, textures[SKY_FRONT]);
  glGenTextures(1, &textures[SKY_BACK]);
  loadTexture(TEXTURE_BACK, textures[SKY_BACK]);
  glGenTextures(1, &textures[SKY_RIGHT]);
  loadTexture(TEXTURE_RIGHT, textures[SKY_RIGHT]);
  glGenTextures(1, &textures[SKY_LEFT]);
  loadTexture(TEXTURE_LEFT, textures[SKY_LEFT]);
  glGenTextures(1, &textures[WOOD]);
  loadTexture(TEXTURE_WOOD, textures[WOOD]);
}

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename) {
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

// glutTimerFunc
void timer(int value){
  switch(value){
    case 0: 
      if(!animation) break;
      char filename[40];
      sprintf(filename, "../screenshots/%03d.jpg", screenShotsCount);
      saveScreenshot(filename);
      screenShotsCount++;
      if(screenShotsCount == 1000) {
        screenShotsCount = 0;
        animation = false;
      }
      break;
    default:
      break;
  }
  glutTimerFunc(100, timer, 0);
}



/***** Display  methods *****/

/* display list for the ground and sky*/
// Create 6 faces box, bottom face to be ground, other 5 to be sky
void groundSkyDisplayList() {
  groundSkyID = glGenLists(1);
  glNewList(groundSkyID, GL_COMPILE);
  glEnable(GL_TEXTURE_2D); 

  glBindTexture(GL_TEXTURE_2D,textures[SKY_DOWN]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(-WORLD_SCALE,-WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(1,0); glVertex3f(+WORLD_SCALE,-WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(1,1); glVertex3f(+WORLD_SCALE,+WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(0,1); glVertex3f(-WORLD_SCALE,+WORLD_SCALE,-WORLD_SCALE);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,textures[SKY_UP]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(-WORLD_SCALE,+WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,0); glVertex3f(+WORLD_SCALE,+WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,1); glVertex3f(+WORLD_SCALE,-WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(0,1); glVertex3f(-WORLD_SCALE,-WORLD_SCALE,+WORLD_SCALE);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,textures[SKY_FRONT]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(+WORLD_SCALE,-WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,0); glVertex3f(+WORLD_SCALE,+WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,1); glVertex3f(+WORLD_SCALE,+WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(0,1); glVertex3f(+WORLD_SCALE,-WORLD_SCALE,-WORLD_SCALE);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,textures[SKY_BACK]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(-WORLD_SCALE,+WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,0); glVertex3f(-WORLD_SCALE,-WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,1); glVertex3f(-WORLD_SCALE,-WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(0,1); glVertex3f(-WORLD_SCALE,+WORLD_SCALE,-WORLD_SCALE);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,textures[SKY_LEFT]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(+WORLD_SCALE,+WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,0); glVertex3f(-WORLD_SCALE,+WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,1); glVertex3f(-WORLD_SCALE,+WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(0,1); glVertex3f(+WORLD_SCALE,+WORLD_SCALE,-WORLD_SCALE);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,textures[SKY_RIGHT]);
  glBegin(GL_QUADS);
  glTexCoord2f(1,0); glVertex3f(+WORLD_SCALE,-WORLD_SCALE,+WORLD_SCALE);
  glTexCoord2f(1,1); glVertex3f(+WORLD_SCALE,-WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(0,1); glVertex3f(-WORLD_SCALE,-WORLD_SCALE,-WORLD_SCALE);
  glTexCoord2f(0,0); glVertex3f(-WORLD_SCALE,-WORLD_SCALE,+WORLD_SCALE);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glEndList();
}

void drawFourLines(point* spline) {
  // (1,1) line
  glBegin(GL_LINE_STRIP);  
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints; j++) {
      double f = RAIL_WIDTH_HEIGHT / 2;
      point p = spline[j];
      point n = splineNormals[j];
      point b = splineBiNormals[j];
      glVertex3d(p.x+f*(n.x+b.x), p.y+f*(n.y+b.y), p.z+f*(n.z+b.z));
    }
  }
  glEnd();
  // (0,1) line
  glBegin(GL_LINE_STRIP);
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints; j++) {
      double f = RAIL_WIDTH_HEIGHT / 2;
      point p = spline[j];
      point n = splineNormals[j];
      point b = splineBiNormals[j];
      glVertex3d(p.x+f*(b.x-n.x), p.y+f*(b.y-n.y), p.z+f*(b.z-n.z));
    }
  }
  glEnd();
  // (0,0) line
  glBegin(GL_LINE_STRIP);
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints; j++) {
      double f = RAIL_WIDTH_HEIGHT / 2;
      point p = spline[j];
      point n = splineNormals[j];
      point b = splineBiNormals[j];
      glVertex3d(p.x+f*(-b.x-n.x), p.y+f*(-b.y-n.y), p.z+f*(-b.z-n.z));
    }
  }
  glEnd();
  // (1,0) line
  glBegin(GL_LINE_STRIP);
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints; j++) {
      double f = RAIL_WIDTH_HEIGHT / 2;
      point p = spline[j];
      point n = splineNormals[j];
      point b = splineBiNormals[j];
      glVertex3d(p.x+f*(n.x-b.x), p.y+f*(n.y-b.y), p.z+f*(n.z-b.z));
    }
  }
  glEnd();
}

void drawFourFaces(point p, point n, point b, 
  point p1, point n1, point b1, double f) {
      // up
      glVertex3d(p.x+f*(n.x+b.x), p.y+f*(n.y+b.y), p.z+f*(n.z+b.z));
      glVertex3d(p1.x+f*(n1.x+b1.x), p1.y+f*(n1.y+b1.y), p1.z+f*(n1.z+b1.z));
      glVertex3d(p1.x+f*(b1.x-n1.x), p1.y+f*(b1.y-n1.y), p1.z+f*(b1.z-n1.z));
      glVertex3d(p.x+f*(b.x-n.x), p.y+f*(b.y-n.y), p.z+f*(b.z-n.z));
      // down
      glVertex3d(p.x+f*(n.x-b.x), p.y+f*(n.y-b.y), p.z+f*(n.z-b.z));
      glVertex3d(p1.x+f*(n1.x-b1.x), p1.y+f*(n1.y-b1.y), p1.z+f*(n1.z-b1.z));
      glVertex3d(p1.x+f*(-b1.x-n1.x), p1.y+f*(-b1.y-n1.y), p1.z+f*(-b1.z-n1.z));
      glVertex3d(p.x+f*(-b.x-n.x), p.y+f*(-b.y-n.y), p.z+f*(-b.z-n.z));
      // left
      glVertex3d(p.x+f*(-n.x+b.x), p.y+f*(-n.y+b.y), p.z+f*(-n.z+b.z));
      glVertex3d(p1.x+f*(-n1.x+b1.x), p1.y+f*(-n1.y+b1.y), p1.z+f*(-n1.z+b1.z));
      glVertex3d(p1.x+f*(-b1.x-n1.x), p1.y+f*(-b1.y-n1.y), p1.z+f*(-b1.z-n1.z));
      glVertex3d(p.x+f*(-b.x-n.x), p.y+f*(-b.y-n.y), p.z+f*(-b.z-n.z));
      // right
      glVertex3d(p.x+f*(n.x+b.x), p.y+f*(n.y+b.y), p.z+f*(n.z+b.z));
      glVertex3d(p1.x+f*(n1.x+b1.x), p1.y+f*(n1.y+b1.y), p1.z+f*(n1.z+b1.z));
      glVertex3d(p1.x+f*(n1.x-b1.x), p1.y+f*(n1.y-b1.y), p1.z+f*(n1.z-b1.z));
      glVertex3d(p.x+f*(n.x-b.x), p.y+f*(n.y-b.y), p.z+f*(n.z-b.z));
}

/** display list for the roller coaster **/
void trackDisplayList() {
  // Store the following calls into displayList
  trackID = glGenLists(1);
  glNewList(trackID, GL_COMPILE);

  /* Draw the tow rails' 4 black edges */
  glLineWidth(3.0);
  glColor3f(0.0f,0.0f,0.0f); 
  drawFourLines(firstSplinePoints); // first rail
  drawFourLines(secondSplinePoints); // second rail

  // Draw the tow rails
  glColor3f(0.3f,0.3f,0.3f);
  glBegin(GL_QUADS);
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints - 1; j++) {
      double f = RAIL_WIDTH_HEIGHT;
      // first rail
      point p = firstSplinePoints[j];
      point n = splineNormals[j];
      point b = splineBiNormals[j];
      point p1 = firstSplinePoints[j + 1];
      point n1 = splineNormals[j + 1];
      point b1 = splineBiNormals[j + 1];
      drawFourFaces(p, n, b, p1, n1, b1, RAIL_WIDTH_HEIGHT / 2);

      // second rail
      p = secondSplinePoints[j];
      p1 = secondSplinePoints[j + 1];
      drawFourFaces(p, n, b, p1, n1, b1, RAIL_WIDTH_HEIGHT / 2);
    }
  }
  glEnd();

  glColor3f(1.0f,1.0f,1.0f);
  glEndList();
}

/** display list for the crosses on the roller coaster **/
void crossDisplayList() {
  // Store the following calls into displayList
  crossID = glGenLists(1);
  glNewList(crossID, GL_COMPILE);
  glEnable(GL_TEXTURE_2D);

  int count = 0;
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints - CROSS_LENGTH; j++) {
      if(count == DISTANCE_BETWEEN_CROSSES) {
        count = 0;
        point n = splineNormals[j];
        point b = splineBiNormals[j];
        point p = splinePoints[j];
        point np = splinePoints[j + CROSS_LENGTH];
        double h = CROSS_HEIGHT;
        double w = CROSS_WIDTH;

        // move p to the bottom of the trail
        p.x = p.x - RAIL_WIDTH_HEIGHT * b.x;
        p.y = p.y - RAIL_WIDTH_HEIGHT * b.y;
        p.z = p.z - RAIL_WIDTH_HEIGHT * b.z;
        np.x = np.x - RAIL_WIDTH_HEIGHT * b.x;
        np.y = np.y - RAIL_WIDTH_HEIGHT * b.y;
        np.z = np.z - RAIL_WIDTH_HEIGHT * b.z;

        glBindTexture(GL_TEXTURE_2D, textures[WOOD]); // 2 - Wood
        glBegin(GL_QUADS);
        // front
        glTexCoord2f(0.0, 0.0); 
        glVertex3d(p.x-w/2.0*n.x, p.y-w/2.0*n.y, p.z-w/2.0*n.z);
        glTexCoord2f(0.0, 1.0); 
        glVertex3d(p.x-w/2.0*n.x-h*b.x, p.y-w/2.0*n.y-h*b.y, p.z-w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 1.0); 
        glVertex3d(p.x+w/2.0*n.x-h*b.x, p.y+w/2.0*n.y-h*b.y, p.z+w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 0.0); 
        glVertex3d(p.x+w/2.0*n.x, p.y+w/2.0*n.y, p.z+w/2.0*n.z);
        // back
        glTexCoord2f(0.0, 0.0); 
        glVertex3d(np.x-w/2.0*n.x, np.y-w/2.0*n.y, np.z-w/2.0*n.z);
        glTexCoord2f(0.0, 1.0); 
        glVertex3d(np.x-w/2.0*n.x-h*b.x, np.y-w/2.0*n.y-h*b.y, np.z-w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 1.0); 
        glVertex3d(np.x+w/2.0*n.x-h*b.x, np.y+w/2.0*n.y-h*b.y, np.z+w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 0.0); 
        glVertex3d(np.x+w/2.0*n.x, np.y+w/2.0*n.y, np.z+w/2.0*n.z);
        // bottom
        glTexCoord2f(0.0, 0.0); 
        glVertex3d(p.x-w/2.0*n.x-h*b.x, p.y-w/2.0*n.y-h*b.y, p.z-w/2.0*n.z-h*b.z);
        glTexCoord2f(0.0, 1.0); 
        glVertex3d(np.x-w/2.0*n.x-h*b.x, np.y-w/2.0*n.y-h*b.y, np.z-w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 1.0); 
        glVertex3d(np.x+w/2.0*n.x-h*b.x, np.y+w/2.0*n.y-h*b.y, np.z+w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 0.0); 
        glVertex3d(p.x+w/2.0*n.x-h*b.x, p.y+w/2.0*n.y-h*b.y, p.z+w/2.0*n.z-h*b.z);
        // up
        glTexCoord2f(0.0, 0.0); 
        glVertex3d(p.x-w/2.0*n.x, p.y-w/2.0*n.y, p.z-w/2.0*n.z);
        glTexCoord2f(0.0, 1.0); 
        glVertex3d(np.x-w/2.0*n.x, np.y-w/2.0*n.y, np.z-w/2.0*n.z);
        glTexCoord2f(1.0, 1.0); 
        glVertex3d(np.x+w/2.0*n.x, np.y+w/2.0*n.y, np.z+w/2.0*n.z);
        glTexCoord2f(1.0, 0.0); 
        glVertex3d(p.x+w/2.0*n.x, p.y+w/2.0*n.y, p.z+w/2.0*n.z);
        // right
        glTexCoord2f(0.0, 0.0); 
        glVertex3d(p.x+w/2.0*n.x, p.y+w/2.0*n.y, p.z+w/2.0*n.z);
        glTexCoord2f(0.0, 1.0); 
        glVertex3d(np.x+w/2.0*n.x, np.y+w/2.0*n.y, np.z+w/2.0*n.z);
        glTexCoord2f(1.0, 1.0); 
        glVertex3d(np.x+w/2.0*n.x-h*b.x, np.y+w/2.0*n.y-h*b.y, np.z+w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 0.0); 
        glVertex3d(p.x+w/2.0*n.x-h*b.x, p.y+w/2.0*n.y-h*b.y, p.z+w/2.0*n.z-h*b.z);
        // left
        glTexCoord2f(0.0, 0.0); 
        glVertex3d(p.x-w/2.0*n.x, p.y-w/2.0*n.y, p.z-w/2.0*n.z);
        glTexCoord2f(0.0, 1.0); 
        glVertex3d(np.x-w/2.0*n.x, np.y-w/2.0*n.y, np.z-w/2.0*n.z);
        glTexCoord2f(1.0, 1.0); 
        glVertex3d(np.x-w/2.0*n.x-h*b.x, np.y-w/2.0*n.y-h*b.y, np.z-w/2.0*n.z-h*b.z);
        glTexCoord2f(1.0, 0.0); 
        glVertex3d(p.x-w/2.0*n.x-h*b.x, p.y-w/2.0*n.y-h*b.y, p.z-w/2.0*n.z-h*b.z);
        glEnd();
      }
      count++;
    }
  }

  glDisable(GL_TEXTURE_2D);
  glEndList();
}

void drawColumn(point p1, point p2, point n1, point n2, double f) {
  glBegin(GL_QUADS);
    // left
    glVertex3d(p1.x-f*n1.x, p1.y-f*n1.y, p1.z-f*n1.z);
    glVertex3d(p2.x-f*n2.x, p2.y-f*n2.x, p2.z-f*n2.x);
    glVertex3d(p2.x-f*n2.x, p2.y-f*n2.x, -HEIGHT);
    glVertex3d(p1.x-f*n1.x, p1.y-f*n1.y, -HEIGHT);
    // right
    glVertex3d(p1.x+f*n1.x, p1.y+f*n1.y, p1.z+f*n1.z);
    glVertex3d(p2.x+f*n2.x, p2.y+f*n2.x, p2.z+f*n2.x);
    glVertex3d(p2.x+f*n2.x, p2.y+f*n2.x, -HEIGHT);
    glVertex3d(p1.x+f*n1.x, p1.y+f*n1.y, -HEIGHT);
    // front
    glVertex3d(p1.x-f*n1.x, p1.y-f*n1.y, p1.z-f*n1.z);
    glVertex3d(p1.x+f*n1.x, p1.y+f*n1.y, p1.z+f*n1.z);
    glVertex3d(p1.x+f*n1.x, p1.y+f*n1.y, -HEIGHT);
    glVertex3d(p1.x-f*n1.x, p1.y-f*n1.y, -HEIGHT);
    // back
    glVertex3d(p2.x-f*n2.x, p2.y-f*n2.y, p2.z-f*n2.z);
    glVertex3d(p2.x+f*n2.x, p2.y+f*n2.y, p2.z+f*n2.z);
    glVertex3d(p2.x+f*n2.x, p2.y+f*n2.y, -HEIGHT);
    glVertex3d(p2.x-f*n2.x, p2.y-f*n2.y, -HEIGHT);
  glEnd();
}

/** display columns **/
void columnDisplayList() {
  // Store the following calls into displayList
  columnID = glGenLists(1);
  glNewList(columnID, GL_COMPILE);

  glColor3f(0.4f, 0.4f, 0.4f);
  int count = 0;
  for(int i = 0; i < g_iNumOfSplines; i++) {
    for(int j = 0; j < numOfCatMullSplinePoints - CROSS_LENGTH / 2; j++) {
      if(count == NUM_TO_DIVIDE_U) {
        count = 0;
        double f = RAIL_WIDTH_HEIGHT;
        // first column
        point p1 = firstSplinePoints[j + CROSS_LENGTH / 2 - COLUMN_LENGTH_WIDTH / 2];
        point p2 = firstSplinePoints[j + CROSS_LENGTH / 2 + COLUMN_LENGTH_WIDTH / 2];
        point n1 = splineNormals[j + CROSS_LENGTH / 2 - COLUMN_LENGTH_WIDTH / 2];
        n1.x = -n1.x; n1.y = -n1.y; n1.z = -n1.z;
        point n2 = splineNormals[j + CROSS_LENGTH / 2 + COLUMN_LENGTH_WIDTH / 2];
        n2.x = -n2.x; n2.y = -n2.y; n2.z = -n2.z;
        drawColumn(p1, p2, n1, n2, f);
        // second column
        p1 = secondSplinePoints[j + CROSS_LENGTH / 2 - COLUMN_LENGTH_WIDTH / 2];
        p2 = secondSplinePoints[j + CROSS_LENGTH / 2 + COLUMN_LENGTH_WIDTH / 2];
        n1.x = -n1.x; n1.y = -n1.y; n1.z = -n1.z;
        n2.x = -n2.x; n2.y = -n2.y; n2.z = -n2.z;
        drawColumn(p1, p2, n1, n2, f);
      }
      count++;
    }
  }

  glColor3f(1.0f, 1.0f, 1.0f);
  glEndList();
}

/** compute new speed **/ 
/* http://dl.dropboxusercontent.com/u/91263185/CS480-f13/projects/
 * HW2/RollerCoasterVelocity.pdf 
 */
double computeNewSpeed(double uCurrent, point p, point t) {
  double divideFrom = sqrt(2 * G * (MAX_HEIGHT - p.z));
  double divideBy = sqrt(pow(t.x, 2) + pow(t.y, 2) + pow(t.z, 2));
  return uCurrent + TIMESTEP * (divideFrom / divideBy);
}

/** set up the camera **/
void setUpCamera() {
  point p, t, n, b, e, c;
  p = splinePoints[(int)(animationTimer + NUM_TO_DIVIDE_U * uCurrent)];
  t = normalize(splineTangents[(int)(animationTimer + NUM_TO_DIVIDE_U * uCurrent)]);
  n = splineNormals[(int)(animationTimer + NUM_TO_DIVIDE_U * uCurrent)];
  b = splineBiNormals[(int)(animationTimer + NUM_TO_DIVIDE_U * uCurrent)];

  e.x = p.x + EYE_UP * b.x;
  e.y = p.y + EYE_UP * b.y;
  e.z = p.z + EYE_UP * b.z;

  c.x = p.x + EYE_UP * b.x + t.x;
  c.y = p.y + EYE_UP * b.y + t.y;
  c.z = p.z + EYE_UP * b.z + t.z;

  gluLookAt(e.x, e.y, e.z, c.x, c.y, c.z, b.x, b.y, b.z);

  // Compute the new animationTimer
  t = splineTangents[(int)(animationTimer + NUM_TO_DIVIDE_U * uCurrent)];
  double uNew = computeNewSpeed(uCurrent, p, t);
  if(uNew - uCurrent < SPEED / NUM_TO_DIVIDE_U) // avoid the car stopping
    uNew = SPEED / NUM_TO_DIVIDE_U + uCurrent;
  if(uNew >= 1.0) {
    animationTimer += NUM_TO_DIVIDE_U;
    uNew = 0.0;
  }
  if(animationTimer >= numOfCatMullSplinePoints) {
    animationTimer = 0;
  }
  uCurrent = uNew;
}

/*
 * Sets up the lighting for the OpenGL scene.
 */
void setUpLight() {
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lignt0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
}

/** display callback **/
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // if(lighting) glEnable(GL_LIGHTING);
  // else glDisable(GL_LIGHTING);
  // glEnable(GL_LIGHT0);

  glPushMatrix();
  glLoadIdentity();
  /* Transform to the current world state */
  glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
  glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
  glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
  glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
  glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

  // start the main display
  setUpCamera(); // set up the camera based on the viewmode
  glCallList(groundSkyID);
  glCallList(trackID);
  glCallList(crossID);
  if(displayColumn) glCallList(columnID);

  glPopMatrix();
  glFlush();
  glutSwapBuffers();
}

/** idle callback **/
void doIdle() {
  /* make the screen update */
  glutPostRedisplay();
}

/* reshape callback **/
/* set projection to aspect ratio of window */
void reshape(int w, int h){
  GLfloat aspect = (GLfloat) w / (GLfloat) h;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOVY, aspect, 0.01, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}



/***** Interaction callback methods *****/

/* converts mouse drags into information about rotation/translation/scaling */
void mousedrag(int x, int y) {
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

void mouseidle(int x, int y) {
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y) {

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
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'q': case 'Q':
    exit(0);
    break;
  case 'a': case 'A':
    animation = !animation;
    break;
  case 'l': case 'L':
    lighting = !lighting;
    break;
  case 'c': case 'C':
    displayColumn = !displayColumn;
    break;
  default: break;
  }
  glutPostRedisplay();
}

/* menu function callback */
void menufunc(int value) {
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}



/** init function **/
void myInit() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  generateSplineInfo(g_Splines); // generate spline's relevant arrays
  generateTextures(); // generate the textures according to the files

  trackDisplayList(); // create the display list of the track
  groundSkyDisplayList(); // create the display list of the gound and sky
  crossDisplayList(); // create the display list of the crosses
  columnDisplayList(); // create the columns

  setUpLight();
}

/** main in assignment 2 **/
int main (int argc, char ** argv) {
  if (argc<2)
  {  
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

  loadSplines(argv[1]);

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutCreateWindow(WIN_TITLE);

  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  /* callback for keyboard function */
  glutKeyboardFunc(keyboard);

  /* callbacks for mouse events */
  glutMotionFunc(mousedrag); // mouse drag
  glutPassiveMotionFunc(mouseidle); // mouse movement
  glutMouseFunc(mousebutton); // mouse button changes

  /* callbacks for displays */
  glutReshapeFunc(reshape);
  glutIdleFunc(doIdle);
  glutDisplayFunc(display);
  glutTimerFunc(100, timer, 0);

  myInit();
  glutMainLoop();
  return 0;
}

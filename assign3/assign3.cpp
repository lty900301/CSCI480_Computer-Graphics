/*
CSCI 480
Assignment 3 Raytracer

Name: Tianyi Luo
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <float.h>
#include <string>
using namespace std;

// SuperSampling value
#define SAMPLING_PIXEL 3
#define RECURSION_LEVEL 3

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define FOV 60.0

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

typedef struct _Triangle
{
  struct Vertex v[3];
} Triangle;

typedef struct _Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
} Sphere;

typedef struct _Light
{
  double position[3];
  double color[3];
} Light;

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

// data structure to store a vector with three double values
struct vector {
   double x;
   double y;
   double z;
};
// data structure to store a ray
struct ray {
  vector origin;
  vector direction;
};
// Camera variables
vector cameraPosition;
double CAMERA_POS[3] = {0.0, 0.0, 0.0};
// define the four corners of the image plane
vector topLeft;
vector topRight;
vector bottomLeft;
vector bottomRight;
// data structure to store a scree matrix, 
// and each element contains a color value
// which can be represented using a vector
vector** screenPixel;
// image plane width and height
double imagePlane_WidthHeight[2] = {0.0, 0.0};
// define the pi value
const double PI = 3.14159265;
// To see if user wants to do reflection
bool isReflection = false;

// decleration of methods
vector normalize(vector p);
vector vectorMinus(vector a, vector b);
vector crossProduct(vector a, vector b);
vector getPosAtT(ray thisRay, double t);
vector getReflection(vector i, vector n);
double dotProduct(vector a, vector b);
double distance(vector a, vector b);
void trace(ray thisRay, vector &color, int count);
void save_jpg();
bool intersectsLightSource(ray thisRay, int index_Light, double &distance);
bool intersectsSphere(ray thisRay, vector &normal, int index_Sphere, double &distanceToSphere);
bool intersectsTriangle(ray thisRay, vector &normal, int index_Triangle, double &distance, double &alpha, double &beta, double &gamma);
void calculateShading(vector intersection, vector normal, vector &phongColor, vector kd, vector ks, double sh, vector v);

//MODIFY THIS FUNCTION
void draw_scene()
{
  // shoot the ray and trace the ray
  double y = bottomLeft.y;
  for (int m = 0; m < HEIGHT * SAMPLING_PIXEL; m++) { // y axies
    double x = bottomLeft.x;
    for(int n = 0; n < WIDTH * SAMPLING_PIXEL; n++) { // x axies
      vector pixelPos; 
      pixelPos.x = x; pixelPos.y = y; pixelPos.z = -1.0;
      // Calculate the direction of the ray
      vector direction = vectorMinus(pixelPos, cameraPosition);
      direction = normalize(direction);
      // Create a ray according to the direction
      ray newRay;
      newRay.origin = cameraPosition;
      newRay.direction = direction;
      // trace the ray
      vector color; color.x = 0.0; color.y = 0.0; color.z = 0.0;
      trace(newRay, color, 0);
      screenPixel[m][n].x = color.x;
      screenPixel[m][n].y = color.y;
      screenPixel[m][n].z = color.z;
      // increase x to the next column
      x += imagePlane_WidthHeight[0] / (WIDTH * SAMPLING_PIXEL);
    }
    // increase y to the upper row
    y += imagePlane_WidthHeight[1] / (HEIGHT * SAMPLING_PIXEL);
  }

  // image output
  int col = 0;
  for(int n = 0; n < WIDTH * SAMPLING_PIXEL; n += SAMPLING_PIXEL) {
    glPointSize(2.0);
    glBegin(GL_POINTS);
    int row = 0;
    for(int m = 0; m < HEIGHT * SAMPLING_PIXEL; m += SAMPLING_PIXEL) {
      // Do the super sampling
      double r = 0.0, g = 0.0, b = 0.0;
      for(int i = 0; i < SAMPLING_PIXEL; i++) {
        for(int j = 0; j < SAMPLING_PIXEL; j++) {
          r += screenPixel[m + j][n + i].x;
          g += screenPixel[m + j][n + i].y;
          b += screenPixel[m + j][n + i].z;
        }
      }
      r /= pow(SAMPLING_PIXEL, 2);
      g /= pow(SAMPLING_PIXEL, 2);
      b /= pow(SAMPLING_PIXEL, 2);
      plot_pixel(col, row, r, g, b);
      row++;
    }
    glEnd();
    glFlush();
    col++;
  }
  if(mode == MODE_JPEG)
  {
    save_jpg();
  }
  printf("Done!\n"); fflush(stdout);
}

/* trace the ray according to the ray's start Location and direction
   Also, set up the color in the screen matrix for each pixel. */
void trace(ray thisRay, vector &color, int count) {
  if(count > RECURSION_LEVEL) // count for recursive function
      return;
  bool isIntersect = false;
  /* If a ray intersects a light source
     return color-of-the-light-source */
  double furthest = DBL_MAX;

  // intersects a light source
  for(int index  = 0; index < num_lights; index++) {
    double distanceToLightSource;
    if(intersectsLightSource(thisRay, index, distanceToLightSource)) {
      if(distanceToLightSource < furthest) {
        isIntersect = true;
        furthest = distanceToLightSource;
        color.x = lights[index].color[0] * 255.0;
        color.y = lights[index].color[1] * 255.0;
        color.z = lights[index].color[2] * 255.0;
      }
    }
  }

  // intersects a shere
  for(int index = 0; index < num_spheres; index++) {
    double distanceToSphere = 0.0;
    vector normal;
    if(intersectsSphere(thisRay, normal, index, distanceToSphere)) {
      if(distanceToSphere < furthest) {
        isIntersect = true;
        furthest = distanceToSphere;
        /* Compute local shading (including shadow determination) */
        vector intersection = getPosAtT(thisRay, distanceToSphere);
        // Initiate phong color model
        // Get the ambient, You only add the ambient color once
        vector phongColor;
        phongColor.x = 0.0; 
        phongColor.y = 0.0; 
        phongColor.z = 0.0;
        // Get the diffuse and specular colors;
        vector kd, ks;
        kd.x = spheres[index].color_diffuse[0];
        kd.y = spheres[index].color_diffuse[1];
        kd.z = spheres[index].color_diffuse[2];
        ks.x = spheres[index].color_specular[0];
        ks.y = spheres[index].color_specular[1];
        ks.z = spheres[index].color_specular[2];
        // Get the sh, which is the shininess of the sphere
        double sh = spheres[index].shininess;
        // Get the v vector
        vector v;
        v.x = -thisRay.direction.x; 
        v.y = -thisRay.direction.y;
        v.z = -thisRay.direction.z;
        v = normalize(v);
        // calculate the phongColor
        calculateShading(intersection, normal, phongColor, kd, ks, sh, v);
        // Set the color to phongColor
        if(!isReflection){
          color.x = phongColor.x * 255.0;
          color.y = phongColor.y * 255.0;
          color.z = phongColor.z * 255.0;
        } else {
          color.x = pow(1 - ks.x, count + 1) * phongColor.x * 255.0;
          color.y = pow(1 - ks.y, count + 1) * phongColor.y * 255.0;
          color.z = pow(1 - ks.z, count + 1) * phongColor.z * 255.0;
        }

        /* Compute refl. dir. (if applicable), trace() [RECURSE!] */
        if(isReflection) {
          vector reflectColor; reflectColor.x = 0.0; reflectColor.y = 0.0; reflectColor.z = 0.0;
          trace(thisRay, reflectColor, count + 1);
          color.x += ks.x * reflectColor.x;
          color.y += ks.y * reflectColor.y;
          color.z += ks.z * reflectColor.z;
        }

        /* Compute xmission dir. (if applicable), trace() [RECURSE!] */
        /* return local+refl+xmsn color */
      }
    }
  }

  // intersects a triangle
  for(int index = 0; index < num_triangles; index++) {
    vector normal;
    double alpha, beta, gamma;
    double distanceToTriangle = DBL_MAX;
    if(intersectsTriangle(thisRay, normal, index, distanceToTriangle, alpha, beta, gamma)){
      if(distanceToTriangle < furthest) {
        isIntersect = true;
        furthest = distanceToTriangle;
        // Compute local shading (including shadow determination)
        vector intersection = getPosAtT(thisRay, distanceToTriangle);
        // Get the normal vector
        normal.x = triangles[index].v[0].normal[0] * alpha + 
            triangles[index].v[1].normal[0] * beta + 
            triangles[index].v[2].normal[0] * gamma;
        normal.y = triangles[index].v[0].normal[1] * alpha + 
            triangles[index].v[1].normal[1] * beta + 
            triangles[index].v[2].normal[1] * gamma;
        normal.z = triangles[index].v[0].normal[2] * alpha + 
            triangles[index].v[1].normal[2] * beta + 
            triangles[index].v[2].normal[2] * gamma;
        // Initiate phong color model
        vector phongColor;
        phongColor.x = 0.0; 
        phongColor.y = 0.0; 
        phongColor.z = 0.0;
        // Get the diffuse and specular colors;
        vector kd, ks;
        kd.x = triangles[index].v[0].color_diffuse[0] * alpha + 
            triangles[index].v[1].color_diffuse[0] * beta + 
            triangles[index].v[2].color_diffuse[0] * gamma;
        kd.y = triangles[index].v[0].color_diffuse[1] * alpha + 
            triangles[index].v[1].color_diffuse[1] * beta + 
            triangles[index].v[2].color_diffuse[1] * gamma;
        kd.z = triangles[index].v[0].color_diffuse[2] * alpha + 
            triangles[index].v[1].color_diffuse[2] * beta + 
            triangles[index].v[2].color_diffuse[2] * gamma;
        ks.x = triangles[index].v[0].color_specular[0] * alpha + 
            triangles[index].v[1].color_specular[0] * beta + 
            triangles[index].v[2].color_specular[0] * gamma;
        ks.y = triangles[index].v[0].color_specular[1] * alpha + 
            triangles[index].v[1].color_specular[1] * beta + 
            triangles[index].v[2].color_specular[1] * gamma;
        ks.z = triangles[index].v[0].color_specular[2] * alpha + 
            triangles[index].v[1].color_specular[2] * beta + 
            triangles[index].v[2].color_specular[2] * gamma;
        // Get the sh, which is the shininess of the sphere
        double sh = triangles[index].v[0].shininess * alpha + 
            triangles[index].v[0].shininess * beta + 
            triangles[index].v[0].shininess * gamma;
        // Get the v vector
        vector v; 
        v.x = -thisRay.direction.x; 
        v.y = -thisRay.direction.y;
        v.z = -thisRay.direction.z;
        v = normalize(v);
        // calculate the phongColor
        calculateShading(intersection, normal, phongColor, kd, ks, sh, v);
        // Set the color to phongColor
        if(!isReflection){
          color.x = phongColor.x * 255.0;
          color.y = phongColor.y * 255.0;
          color.z = phongColor.z * 255.0;
        } else {
          color.x = pow(1 - ks.x, count + 1) * phongColor.x * 255.0;
          color.y = pow(1 - ks.y, count + 1) * phongColor.y * 255.0;
          color.z = pow(1 - ks.z, count + 1) * phongColor.z * 255.0;
        }

        /* Compute refl. dir. (if applicable), trace() [RECURSE!] */
        if(isReflection) {
          vector reflectColor; reflectColor.x = 0.0; reflectColor.y = 0.0; reflectColor.z = 0.0;
          trace(thisRay, reflectColor, count + 1);
          color.x += ks.x * reflectColor.x;
          color.y += ks.y * reflectColor.y;
          color.z += ks.z * reflectColor.z;
        }

        // compute xmission dir. (if applicable), trace() [RECURSE!]
        // return local+refl+xmsn color
      }
    }
  }
  if(isIntersect) {
    color.x += ambient_light[0] * 255;
    color.y += ambient_light[1] * 255;
    color.z += ambient_light[2] * 255;
  }
  if(!isIntersect) {
    color.x = 255.0; color.y = 255.0; color.z = 255.0;
  }
  color.x = max(min(color.x, 255.0), 0.0);
  color.y = max(min(color.y, 255.0), 0.0);
  color.z = max(min(color.z, 255.0), 0.0);
}

// Check if the intersection vector is in shadow
// if not, calculate the other color channel seperately
void calculateShading(vector intersection, vector normal, vector &phongColor, 
    vector kd, vector ks, double sh, vector v) {
  for(int index_Light = 0; index_Light < num_lights; index_Light++) {
    bool isInShadow = false;
    // launching a shadow ray to each of the lights
    vector lightPos, origin, direction;
    lightPos.x = lights[index_Light].position[0];
    lightPos.y = lights[index_Light].position[1];
    lightPos.z = lights[index_Light].position[2];
    origin.x = intersection.x; 
    origin.y = intersection.y; 
    origin.z = intersection.z;
    direction.x = lightPos.x - origin.x;
    direction.y = lightPos.y - origin.y;
    direction.z = lightPos.z - origin.z;
    direction = normalize(direction);
    ray shadowRay; shadowRay.origin = origin; shadowRay.direction = direction;
    // use existing function to calculate distanceToLightSource
    double distanceToLightSource = distance(lightPos, origin);
    // see if it intersects to other sphere before reaching light
    for(int index_Sphere = 0; index_Sphere < num_spheres; index_Sphere++) {
      double distanceToSphere;
      vector dummy_normal;
      if(intersectsSphere(shadowRay, dummy_normal, index_Sphere, distanceToSphere)) {
        vector p = getPosAtT(shadowRay, distanceToSphere);
        distanceToSphere = distance(p, origin);
        if(distanceToSphere <= distanceToLightSource){
          isInShadow = true;
        }
      }
    }
    // see if it intersects to other triangle before reaching light
    for(int index_Triangle = 0; index_Triangle < num_triangles; index_Triangle++) {
      double distanceToTriangle, alpha, beta, gamma;
      vector tempNormal;
      if(intersectsTriangle(shadowRay, tempNormal, index_Triangle, distanceToTriangle, alpha, beta, gamma)) {
        vector p = getPosAtT(shadowRay, distanceToTriangle);
        distanceToTriangle = distance(p, origin);
        if(distanceToTriangle <= distanceToLightSource){
          isInShadow = true;
        }
      }
    }
    if(!isInShadow) {
      // Calculate the phong model color
      double LdotN = dotProduct(direction, normal);
      if(LdotN < 0.0) LdotN = 0.0;
      vector r = normalize(getReflection(direction, normal));
      double RdotV = dotProduct(r, v);
      if(RdotV < 0.0) RdotV = 0.0;
      phongColor.x += lights[index_Light].color[0] * (kd.x * LdotN + ks.x * pow(RdotV, sh));
      phongColor.y += lights[index_Light].color[1] * (kd.y * LdotN + ks.y * pow(RdotV, sh));
      phongColor.z += lights[index_Light].color[2] * (kd.z * LdotN + ks.z * pow(RdotV, sh));
    }
  }
}

// Check if the ray is intersected with the light source
bool intersectsLightSource(ray thisRay, int index_Light, double &distance) {
  /* p(t) = p0 + dt;
     in which, t is a scaler, so for x, y, z their computed t should
     all be the same. 
  */
  // if the light is in the same location as the camera, it's not called intersection
  if(lights[index_Light].position[0] == thisRay.origin.x && 
      lights[index_Light].position[1] == thisRay.origin.y && 
      lights[index_Light].position[2] == thisRay.origin.z)
    return false;

  // otherwise
  distance = (lights[index_Light].position[0] - thisRay.origin.x)
               / thisRay.direction.x;
  if(distance != (lights[index_Light].position[1] - thisRay.origin.y)
               / thisRay.direction.y)
    return false;
  if(distance != (lights[index_Light].position[2] - thisRay.origin.z)
               / thisRay.direction.z)
    return false;
  return true;
}

// Check if the ray is intersected with one of the sphere
bool intersectsSphere(ray thisRay, vector &normal, int index_Sphere, double &distance) {
  double r = spheres[index_Sphere].radius;
  double b = 2.0 * (
      thisRay.direction.x * (thisRay.origin.x - spheres[index_Sphere].position[0]) + 
      thisRay.direction.y * (thisRay.origin.y - spheres[index_Sphere].position[1]) + 
      thisRay.direction.z * (thisRay.origin.z - spheres[index_Sphere].position[2]));
  double c = pow(thisRay.origin.x - spheres[index_Sphere].position[0], 2.0) + 
      pow(thisRay.origin.y - spheres[index_Sphere].position[1], 2.0) + 
      pow(thisRay.origin.z - spheres[index_Sphere].position[2], 2.0) - 
      pow(r, 2.0);
  // solve to obtain t0 and t1
  double delta = pow(b, 2.0) - 4.0 * c;
  if(delta < 0.0) return false;
  double t0 = (-b + sqrt(delta)) / 2;
  double t1 = (-b - sqrt(delta)) / 2;
  // Check if t0, t1 both < 0, abort if so
  if(t0 <= 0 && t1 <= 0) return false;
  // Get the minimal intersection
  if(t0 > 0 && t1 > 0) distance = min(t0, t1); 
  else distance = max(t0, t1); // ray originates inside the sphere
  if(distance < 0.0001) return false;
  // For lighting, calculate unit normal
  vector i = getPosAtT(thisRay, distance);
  normal.x = i.x - spheres[index_Sphere].position[0]; 
  normal.y = i.y - spheres[index_Sphere].position[1]; 
  normal.z = i.z - spheres[index_Sphere].position[2];
  normal = normalize(normal);
  return true;
}

// Check if the ray is intersected with the triangle
/* Referencing Steve Marschner's note
http://www.cs.cornell.edu/courses/cs465/2003fa/homeworks/raytri.pdf */
bool intersectsTriangle(ray thisRay, vector &normal, int index_Triangle, double &distance, double &alpha, double &beta, double &gamma) {
  // triangle three points
  vector p0; 
  p0.x = triangles[index_Triangle].v[0].position[0];
  p0.y = triangles[index_Triangle].v[0].position[1];
  p0.z = triangles[index_Triangle].v[0].position[2];
  vector p1; 
  p1.x = triangles[index_Triangle].v[1].position[0];
  p1.y = triangles[index_Triangle].v[1].position[1];
  p1.z = triangles[index_Triangle].v[1].position[2];
  vector p2; 
  p2.x = triangles[index_Triangle].v[2].position[0];
  p2.y = triangles[index_Triangle].v[2].position[1];
  p2.z = triangles[index_Triangle].v[2].position[2];
  // calculate the normal
  normal = normalize(crossProduct(vectorMinus(p1, p0), vectorMinus(p2, p0)));

  // if n*d = 0, no intersection (ray parallel to plane)
  double denominator = dotProduct(thisRay.direction, normal);
  if(denominator == 0.0) {
    return false;
  }
  // calculate t = -((o-p)*n / (d*n));, t is distance in this function
  distance = -1.0 * (dotProduct(vectorMinus(thisRay.origin, p0), normal)) / denominator;
  if(distance <= 0.01){ // the intersection is behind ray origin
    return false;
  }
  /* test if vector inside polygon */
  // calculate the intersection
  vector x = getPosAtT(thisRay, distance);
  // calculate a0 = ((p1 - p0) cross (x - p0)) dot n
  double a0 = dotProduct(crossProduct(vectorMinus(p1, p0), vectorMinus(x, p0)), normal);
  // calculate a1 = ((p2 - p1) cross (x - p1)) dot n
  double a1 = dotProduct(crossProduct(vectorMinus(p2, p1), vectorMinus(x, p1)), normal);
  // calculate a2 = ((p0 - p2) cross (x - p2)) dot n
  double a2 = dotProduct(crossProduct(vectorMinus(p0, p2), vectorMinus(x, p2)), normal);
  // if a0, a1, a2 areas all larger than 0, then intersection is inside triangle
  if(a0 >= 0 && a1 >= 0 && a2 >= 0){
    // calculate the whole triangle area
    double a = dotProduct(crossProduct(vectorMinus(p1, p0), vectorMinus(p2, p0)), normal);
    // generate the alpha beta gamma value
    alpha = a1 / a; beta = a2 / a; gamma = 1.0 - alpha - beta;
    return true;
  }
  return false;
}

/* This function will use p(t) = p0 + dt to get the ray position at time t */
vector getPosAtT(ray thisRay, double t) {
  vector pos;
  pos.x = thisRay.origin.x + thisRay.direction.x * t;
  pos.y = thisRay.origin.y + thisRay.direction.y * t;
  pos.z = thisRay.origin.z + thisRay.direction.z * t;
  return pos;
}

/* This function is used to get the reflection vector according to normal */
vector getReflection(vector i, vector n) {
  double IdotN = dotProduct(i, n);
  vector r;
  r.x = 2 * IdotN * n.x - i.x;
  r.y = 2 * IdotN * n.y - i.y;
  r.z = 2 * IdotN * n.z - i.z;
  return r;
}

/* This function normalize the vector */
vector normalize(vector p) {
  double length = sqrt(pow(p.x, 2) + pow(p.y, 2) + pow(p.z, 2));
  p.x /= length;
  p.y /= length;
  p.z /= length;
  return p;
}

/* This function is used to do the minus calculation for two vectors */
vector vectorMinus(vector a, vector b) {
  vector c;
  c.x = a.x - b.x;
  c.y = a.y - b.y;
  c.z = a.z - b.z;
  return c;
}

/* This function performs cross product of two vectors */
vector crossProduct(vector a, vector b) {
  vector c;
  c.x = a.y * b.z - a.z * b.y;
  c.y = a.z * b.x - a.x * b.z;
  c.z = a.x * b.y - a.y * b.x;
  return c;
}

/* This function performs dot product of two vectors */
double dotProduct(vector a, vector b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* This function calculates the distance between point a and point b */
double distance(vector a, vector b) {
  return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
      plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  Pic *in = NULL;

  in = pic_alloc(640, 480, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      

}

void parse_check(char *expected,char *found)
{
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }
}

void parse_doubles(FILE*file, char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE *file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
	{

	  printf("found triangle\n");
	  int j;

	  for(j=0;j < 3;j++)
	    {
	      parse_doubles(file,"pos:",t.v[j].position);
	      parse_doubles(file,"nor:",t.v[j].normal);
	      parse_doubles(file,"dif:",t.v[j].color_diffuse);
	      parse_doubles(file,"spe:",t.v[j].color_specular);
	      parse_shi(file,&t.v[j].shininess);
	    }

	  if(num_triangles == MAX_TRIANGLES)
	    {
	      printf("too many triangles, you should increase MAX_TRIANGLES!\n");
	      exit(0);
	    }
	  triangles[num_triangles++] = t;
	}
      else if(strcasecmp(type,"sphere")==0)
	{
	  printf("found sphere\n");

	  parse_doubles(file,"pos:",s.position);
	  parse_rad(file,&s.radius);
	  parse_doubles(file,"dif:",s.color_diffuse);
	  parse_doubles(file,"spe:",s.color_specular);
	  parse_shi(file,&s.shininess);

	  if(num_spheres == MAX_SPHERES)
	    {
	      printf("too many spheres, you should increase MAX_SPHERES!\n");
	      exit(0);
	    }
	  spheres[num_spheres++] = s;
	}
      else if(strcasecmp(type,"light")==0)
	{
	  printf("found light\n");
	  parse_doubles(file,"pos:",l.position);
	  parse_doubles(file,"col:",l.color);

	  if(num_lights == MAX_LIGHTS)
	    {
	      printf("too many lights, you should increase MAX_LIGHTS!\n");
	      exit(0);
	    }
	  lights[num_lights++] = l;
	}
      else
	{
	  printf("unknown type in scene description:\n%s\n",type);
	  exit(0);
	}
    }
  return 0;
}

void display()
{

}

void getFourCorners(double a, double fov) {
  double x = a * tan(fov / 2 * (PI / 180));
  double y = tan(fov / 2 * (PI / 180));
  double z = -1.0;
  topLeft.x = -x;    topLeft.y = y;       topLeft.z = z;
  topRight.x = x;    topRight.y = y;      topRight.z = z;
  bottomLeft.x = -x; bottomLeft.y = -y;   bottomLeft.z = z;
  bottomRight.x = x; bottomRight.y = -y;  bottomRight.z = z;

  // calculate the image plane width and height
  imagePlane_WidthHeight[0] = 2.0 * x;
  imagePlane_WidthHeight[1] = 2.0 * y;
}

// Initialize the screen pixel to have color of black in each pixel
void initScreenPixel() {
  // allocate the memory of HEIGHT * WIDTH matrix
  screenPixel = new vector* [HEIGHT * SAMPLING_PIXEL];
  for(int i = 0; i < HEIGHT * SAMPLING_PIXEL; i++) {
    screenPixel[i] = new vector[WIDTH * SAMPLING_PIXEL];
  }
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

  // init the camera position
  cameraPosition.x = CAMERA_POS[0]; 
  cameraPosition.y = CAMERA_POS[1]; 
  cameraPosition.z = CAMERA_POS[2];

  // Get the four corners
  double a = (double) WIDTH / (double) HEIGHT;
  getFourCorners(a, FOV);

  // Generate the empty image into screenPixel
  initScreenPixel();
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
	  save_jpg();
  }
  once=1;
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 4)
  {  
    printf ("usage: %s <scenefile> [jpegname] [-r to turn on reflection]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    string arg = argv[argc - 1];
    if(arg == "-r") isReflection = true;
    else {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  } else if(argc == 4){
    string arg = argv[argc - 1];
    if(arg == "-r") isReflection = true;
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

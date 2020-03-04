#include <windows.h>  // for MS Windows
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include <math.h>
#include<iostream>
#include<stack>

#include "imageloader.h"

using namespace std;
class Cube{
public:
    Cube(){};
    setCoor(float l, float h,float w, float yH){
        x = l;
        y = h;
        z = w;
        yHeight = yH;
    };
    void render();
protected:
    float x;
    float y;
    float z;
    float yHeight;
};
class Floor: public Cube{
public:
    Floor(){};
    void render();
};
class Cylinder{
public:
    Cylinder(){};
    Cylinder(float X){
        poleX = X;
    };
    void render(int colorI);
private:

    float poleX;
    int colorI;
};
class Disk: public Cylinder{
public:
    Disk(){};
    Disk(float x, float y, float z, float r, int diskId);
    int getPoleNo(){
        return poleNo;
    }
    void setCoordinate();
    void moveTo(int poleNo);
    void render(int colorI);
    float x;
    float y;
    float z;
    float r;
    int colorI;
    int poleNo = 1;
    int targetPole;
    int diskId;
};
//Methods prototype
void initGL();
void display();
void reshape(int, int);
void keyboard(unsigned char key,int x, int y);
void specialKey(int key, int x, int y);
void timer(int);
bool isPos(int x){if(x >= 0) return true;else return false;}
void text();
GLuint loadTexture(Image* image);
string ExePath();
static void myShadowMatrix(float ground[4], float light[4]);

//Global variable
char Text[] = "You Win!";
char title[] = "Tower of Hanoi";

bool Move = false;
bool moveUp = true;
bool moveHorizontal= false;
bool moveDown = false;
bool offLight = false;
bool won = false;

enum COLOR {RED = 0, ORANGE, YELLOW, CYAN, BLACK, WHITE};
const float DEG2RAD = 3.14159/180.0;
float tohZ = 5.0f;
float tohX = 0.0f;
float lightX = 0.0f;
float color[6][3] = {{1.0, 0.0, 0.0},
                     {1.0, 0.5, 0.0},
                     {1.0, 1.0, 0.0},
                     {0.0, 1.0, 1.0},
                     {0.0, 0.0, 0.0},
                     {1.0, 1.0, 1.0}};

static int useRGB = 1;
static int useLighting = 1;
static int useQuads = 1;

static int moving = 1;

int diskOnPole[3] = { 3, 0, 0 };
stack<int> *stackPointer;
stack<int> stackPole1;
stack<int> stackPole2;
stack<int> stackPole3;

GLfloat g_light_pos[4] =   {-5.0f, 7.0f, 5.0f, 0.0f };
GLfloat light_ambient[] =  { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat light_diffuse[] =  { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float groundPlane[4] = {0.0, 1.0,0.0, 1.0};

//Floor and base
Cube cubeBase;
Floor cubeFloor;
Floor *floorPlane = &cubeFloor;
Cube *base = &cubeBase;

//Pole and disk
Cylinder pole1(-1.5f);
Cylinder pole2( 0.0f);
Cylinder pole3( 1.5f);

Disk diskC( -1.50, 0.0, -1.00, 0.70, 3 );
Disk diskB( -1.50, 0.0, -0.85, 0.50, 2 );
Disk diskA( -1.50, 0.0, -0.70, 0.35, 1 );
Disk *diskPointer;
GLUquadricObj *quadObj;

//Texture Ids
GLuint _textureId1;     //Floor
GLuint _textureId2;     //Base
GLuint _textureId3;     //Background


int main(int argc, char** argv) {
   glutInit(&argc, argv);            // Initialize GLUT
   glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH | GLUT_RGBA );


   glutInitWindowSize(1366, 720);   // Set the window's initial width & height
   glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
   glutCreateWindow(title);          // Create window with the given title
   initGL();                       // Our own OpenGL initialization
   glutDisplayFunc(display);       // Register callback handler for window re-paint event
   glutReshapeFunc(reshape);       // Register callback handler for window re-size event
   glutKeyboardFunc(keyboard);
   glutTimerFunc(0, timer, 0);
   glutMainLoop();                 // Enter the infinite event-processing loop
   return 0;
}

/* Initialize OpenGL Graphics */
void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
    glClearDepth(1.0f);                   // Set background depth to farthest

    string temp = ExePath() + "/Pictures/ground.bmp";
    Image* image = loadBMP(temp.c_str());

	_textureId1 = loadTexture(image);
	delete image;

	temp = ExePath() + "/Pictures/base.bmp";
    image = loadBMP(temp.c_str());
	_textureId2 = loadTexture(image);
	delete image;

	temp = ExePath() + "/Pictures/background.bmp";
    image = loadBMP(temp.c_str());
	_textureId3 = loadTexture(image);
	delete image;

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv( GL_LIGHT0, GL_POSITION, g_light_pos );
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);

    glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
    glShadeModel(GL_SMOOTH);   // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
    quadObj = gluNewQuadric();

    base->setCoor(3.0f, -1.0f, 1.0f, 0.3f);             //Initialize both base of tower of hanoi
    floorPlane->setCoor(10.0f, -1.3f, 10.0f, 15.0f);    //Initialize floor plane
}

/* Handler for window-repaint event. Called back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {

    GLfloat cubeXform[4][4];

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix
    glLoadIdentity();

    glPushMatrix();
        glRotatef(lightX, 0.0, 1.0, 0.0);
        glLightfv( GL_LIGHT0, GL_POSITION, g_light_pos );
    glPopMatrix();
    //on off light
    if(offLight)
        glDisable(GL_LIGHT0);
    else
        glEnable(GL_LIGHT0);

    //Floor and background
    floorPlane->render();

    //Tower of Hanoi
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, tohZ);
    glRotatef(tohX, 0.0f, 1.0f, 0.0f);
        base->render();
        pole1.render(RED);
        pole2.render(ORANGE);
        pole3.render(YELLOW);
        diskA.render(CYAN);
        diskB.render(YELLOW);
        diskC.render(WHITE);
    glScalef(1.0, 2.0, 1.0);
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) cubeXform);
    glPopMatrix();

    if(!offLight){
        glDepthMask(GL_FALSE);
        //Render shadow
        glPushMatrix();
          myShadowMatrix(groundPlane, g_light_pos);
          glTranslatef(0.9, -0.1, -1.1);
          glMultMatrixf((const GLfloat *) cubeXform);
            pole1.render(BLACK);
            pole2.render(BLACK);
            pole3.render(BLACK);
            diskA.render(BLACK);
            diskB.render(BLACK);
            diskC.render(BLACK);
        glPopMatrix();
        glDepthMask(GL_TRUE);
    }
    //Banner, render if won
    if(stackPole1.empty() && stackPole2.empty() && stackPole3.top() == 1 && !Move){
        won = true;
        glTranslatef( 0.0, 3.0, 0.0);
        glBegin(GL_QUADS);
            glColor3f(0.0, 0.5, 1.0);
            glVertex3f( 3.0,  0.5, 3.0);
            glColor3f(1.0, 0.0,0.5);
            glVertex3f(-3.0,  0.5, 3.0);
            glColor3f(1.0, 0.0,1.0);
            glVertex3f(-3.0, -0.5, 3.0);
            glColor3f(0.0, 1.0, 1.0);
            glVertex3f( 3.0, -0.5, 3.0);
        glEnd();
        glLoadIdentity();
        text();
    }
    glFlush();
}
/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
    // Compute aspect ratio of the new window
    if (height == 0) height = 1;                // To prevent divide by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);
    // Set the aspect ratio of the clipping volume to match the viewport
    glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
    glLoadIdentity();             // Reset
    // Enable perspective projection with fovy, aspect, zNear and zFar
    gluPerspective(45.0f, aspect, 1.0f, 100.0f);
    gluLookAt(0.0, 0.0, 15.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//Press l to turn on/off light
//Press r to reset after you won the game
//Press a,b or c to select disk
//Press 1, 2 or 3 to select destination pole after selected the disk
void keyboard(unsigned char key,int x, int y){
    if(key == 'l' && !offLight){
        offLight = true ;
    } else if(key == 'l' && offLight){
        offLight = false;
    }
    //Reset the tower of hanoi
    if(key == 'r' && won){
        won = false;
        tohX = 0.0;
        stackPole3.pop();
        stackPole3.pop();
        stackPole3.pop();

        stackPole1.push(diskC.diskId);
        stackPole1.push(diskB.diskId);
        stackPole1.push(diskA.diskId);

        diskA.x = -1.5;diskA.poleNo = 1;
        diskB.x = -1.5;diskB.poleNo = 1;
        diskC.x = -1.5;diskC.poleNo = 1;

        diskOnPole[0] = 3;
        diskOnPole[2] = 0;
    }
    if(!Move && !won){
        switch(key)
        {
        case 'a':
            diskPointer = &diskA;
            break;
        case 'b':
            diskPointer = &diskB;
            break;
        case 'c':
            diskPointer = &diskC;
            break;
        default:
            break;
        }
        if(diskPointer){
            if(diskPointer->poleNo == 1) stackPointer = &stackPole1;
            if(diskPointer->poleNo == 2) stackPointer = &stackPole2;
            if(diskPointer->poleNo == 3) stackPointer = &stackPole3;
            switch(key)
            {
            case '1':
                if(diskPointer->getPoleNo() == 1)
                    break;
                if(stackPointer->top() != diskPointer->diskId)
                    break;
                if(!stackPole1.empty()&& diskPointer->diskId > stackPole1.top())
                    break;
                stackPointer->pop();
                stackPole1.push(diskPointer->diskId);

                diskPointer->targetPole = 1;
                moveUp = true;
                cout<<"Move from "<<diskPointer->poleNo<<" to "<<diskPointer->targetPole<<endl;
                diskPointer->moveTo(1);
                break;
            case '2':
                if(diskPointer->getPoleNo() == 2)
                    break;
                if(stackPointer->top() != diskPointer->diskId)
                    break;
                if(!stackPole2.empty() && diskPointer->diskId > stackPole2.top())
                    break;
                stackPointer->pop();
                stackPole2.push(diskPointer->diskId);

                diskPointer->targetPole = 2;
                moveUp = true;
                cout<<"Move from "<<diskPointer->poleNo<<" to "<<diskPointer->targetPole<<endl;
                diskPointer->moveTo(2);
                break;
            case '3':
                if(diskPointer->getPoleNo() == 3)
                    break;
                if(stackPointer->top() != diskPointer->diskId)
                    break;
                if(!stackPole3.empty() && diskPointer->diskId >stackPole3.top())
                    break;
                stackPointer->pop();
                stackPole3.push(diskPointer->diskId);

                diskPointer->targetPole = 3;
                moveUp = true;
                cout<<"Move from "<<diskPointer->poleNo<<" to "<<diskPointer->targetPole<<endl;
                diskPointer->moveTo(3);
                break;
            default:
                break;
            }
        }
    }
    glutPostRedisplay();
}
void timer(int){
    //Move action starts
    if(Move){
        glutTimerFunc(10, timer, 0);
        glutPostRedisplay();
        //Move up to remove from pole
        if(diskPointer->z < 2.0 && moveUp){
            diskPointer->z += 0.1;
        } else if(moveUp){
            //When removed from pole, start moving in horizontal, either left or right
            moveUp = false;
            moveHorizontal = true;
        }
        //Start moving left, right
        if(moveHorizontal && moveDown == false){
            //Distance between target pole and original pole
            int distanceOfPole = diskPointer->targetPole - diskPointer->poleNo;
            //Static count holder
            static int count = 0;
            //If move to right pole
            if(count < 15 * distanceOfPole && isPos(distanceOfPole)){
                diskPointer->x += 0.1;
                count++;
            } else if(count < -15 * distanceOfPole && isPos(distanceOfPole) == false){
                //if move to left
                diskPointer->x -= 0.1;
                count++;
            }
            else {
                //Reset counter and flags
                count = 0;
                moveHorizontal = false;
                moveDown = true;
            }
        }
        if(moveDown){
            //height of one disk
            float heightOfDisk = 0.15;
            //Check number of
            int numberOfDisk = diskOnPole[diskPointer->targetPole - 1];
            if(diskPointer->z > (-1.0 + numberOfDisk * heightOfDisk))
                diskPointer->z -= 0.05;
            else{
                moveDown = false;
                Move = false;
                diskOnPole[diskPointer->poleNo - 1]--;
                diskOnPole[diskPointer->targetPole - 1]++;
                diskPointer->poleNo = diskPointer->targetPole;
            }
        }
    }
    if(won){
        tohX += 1.0f;
        if(tohX == 360.0f)
            tohX = 0.0f;
        glutTimerFunc(1000/60, timer, 0);
        glutPostRedisplay();
    }
}
/* Draw text */
void text(){
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos3f( -0.6 , 2.75 , 3.5);
    for(int i = 0; Text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, Text[i]);
}

/* Load the texture image */
GLuint loadTexture(Image* image){
    GLuint textureId;
    //Make room for our texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);	//Tell OpenGL which texture to edit

    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGB,						    //Format OpenGL uses for image
            image->width, image->height,	//Width and height
            0,								//The border of the image
            GL_RGB,						    //GL_RGB, because pixels are stored in RGB format
            GL_UNSIGNED_BYTE,				//GL_UNSIGNED_BYTE, because pixels are stored as unsigned numbers
            image->pixels);				    //The actual pixel data
    return textureId;						//Returns the id of the texture
}
/* Get the execution path */
string ExePath() {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}
void drawEllipse(float xradius, float yradius){
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < 360; i++){
		 //convert degrees into radians
		float degInRad = i*DEG2RAD;
		glVertex2f(cos(degInRad)*xradius, sin(degInRad)*yradius);
	}
	glEnd();
}
static void myShadowMatrix(float ground[4], float light[4]){
    float dot;
    float shadowMat[4][4];

    dot = ground[0] * light[0] +
    ground[1] * light[1] +
    ground[2] * light[2] +
    ground[3] * light[3];

    shadowMat[0][0] = dot - light[0] * ground[0];
    shadowMat[1][0] = 0.0 - light[0] * ground[1];
    shadowMat[2][0] = 0.0 - light[0] * ground[2];
    shadowMat[3][0] = 0.0 - light[0] * ground[3];

    shadowMat[0][1] = 0.0 - light[1] * ground[0];
    shadowMat[1][1] = dot - light[1] * ground[1];
    shadowMat[2][1] = 0.0 - light[1] * ground[2];
    shadowMat[3][1] = 0.0 - light[1] * ground[3];

    shadowMat[0][2] = 0.0 - light[2] * ground[0];
    shadowMat[1][2] = 0.0 - light[2] * ground[1];
    shadowMat[2][2] = dot - light[2] * ground[2];
    shadowMat[3][2] = 0.0 - light[2] * ground[3];

    shadowMat[0][3] = 0.0 - light[3] * ground[0];
    shadowMat[1][3] = 0.0 - light[3] * ground[1];
    shadowMat[2][3] = 0.0 - light[3] * ground[2];
    shadowMat[3][3] = dot - light[3] * ground[3];

    glMultMatrixf((const GLfloat *) shadowMat);
}
/* Class methods definitions */
Disk::Disk(float x, float y, float z, float r, int diskId){
    this->x = x;
    this->y = y;
    this->z = z;
    this->r = r;
    this->diskId = diskId;
    stackPole1.push(diskId);
}
void Cube::render(){

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
      //Top
      glColor3fv(color[WHITE]);

      glVertex3f( x, y, -z);   glTexCoord2f( 1.0, 1.0);
      glVertex3f(-x, y, -z);   glTexCoord2f( 0.0, 1.0);
      glVertex3f(-x, y,  z);   glTexCoord2f( 0.0, 0.0);
      glVertex3f( x, y,  z);   glTexCoord2f( 1.0, 0.0);

      // Bottom
      glVertex3f( x, y - yHeight,  z);    glTexCoord2f( 1.0, 1.0);
      glVertex3f(-x, y - yHeight,  z);    glTexCoord2f( 0.0, 1.0);
      glVertex3f(-x, y - yHeight, -z);    glTexCoord2f( 0.0, 0.0);
      glVertex3f( x, y - yHeight, -z);    glTexCoord2f( 1.0, 0.0);

      // Front
      glVertex3f(-x, y          , z);    glTexCoord2f( 1.0, 1.0);
      glVertex3f(-x, y - yHeight, z);    glTexCoord2f( 0.0, 1.0);
      glVertex3f( x, y - yHeight, z);    glTexCoord2f( 0.0, 0.0);
      glVertex3f( x, y          , z);    glTexCoord2f( 1.0, 0.0);

      // Back
      glVertex3f( x, y - yHeight, -z);    glTexCoord2f( 1.0, 1.0);
      glVertex3f(-x, y - yHeight, -z);    glTexCoord2f( 0.0, 1.0);
      glVertex3f(-x, y          , -z);    glTexCoord2f( 0.0, 0.0);
      glVertex3f( x, y          , -z);    glTexCoord2f( 1.0, 0.0);

      // Left
      glVertex3f(-x, y          ,  z);    glTexCoord2f( 1.0, 1.0);
      glVertex3f(-x, y          , -z);    glTexCoord2f( 0.0, 1.0);
      glVertex3f(-x, y - yHeight, -z);    glTexCoord2f( 0.0, 0.0);
      glVertex3f(-x, y - yHeight,  z);    glTexCoord2f( 1.0, 0.0);

      // Right
      glVertex3f(x, y          , -z);    glTexCoord2f( 1.0, 1.0);
      glVertex3f(x, y          ,  z);    glTexCoord2f( 0.0, 1.0);
      glVertex3f(x, y - yHeight,  z);    glTexCoord2f( 0.0, 0.0);
      glVertex3f(x, y - yHeight, -z);    glTexCoord2f( 1.0, 0.0);

   glEnd();  // End of drawing color-cube
   glDisable(GL_TEXTURE_2D);
}
void Floor::render(){
    glColor3fv(color[WHITE]);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Floor plane
    glBegin(GL_QUADS);
        glTexCoord2f( 3.0, 3.0);glVertex3f( x, y, -z);
        glTexCoord2f( 0.0, 3.0);glVertex3f(-x, y, -z);
        glTexCoord2f( 0.0, 0.0);glVertex3f(-x, y,  z);
        glTexCoord2f( 3.0, 0.0);glVertex3f( x, y,  z);
    glEnd();

    //Background
    glBindTexture(GL_TEXTURE_2D, _textureId3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBegin(GL_QUADS);
        glTexCoord2f( 1.0, 0.9);    glVertex3f( x, y + yHeight, -z);
        glTexCoord2f( 0.0, 0.9);    glVertex3f(-x, y + yHeight, -z);
        glTexCoord2f( 0.0, 0.0);    glVertex3f(-x, y          , -z);
        glTexCoord2f( 1.0, 0.0);    glVertex3f( x, y          , -z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
void Cylinder::render(int colorI){
    glColor3fv(color[colorI]);
    glPushMatrix();
        glRotatef(-90, 1.0, 0.0, 0.0);
        glTranslatef( poleX, 0.0, -1.0);
        gluCylinder(quadObj, 0.1, 0.1, 2.5, 20, 20);
    glPopMatrix();
}
void Disk::render(int colorI){
    glColor3fv(color[colorI]);
    glPushMatrix();
        glRotatef(-90, 1.0, 0.0, 0.0);
        glTranslatef(  x, y, z);
        drawEllipse(r,r);
        gluCylinder(quadObj, r, r, 0.2, 20, 20);
    glPopMatrix();
    glPushMatrix();
        glRotatef(-90, 1.0, 0.0, 0.0);
        glTranslatef(  x, y , z + 0.2);
        drawEllipse(r,r);
    glPopMatrix();
}
void Disk:: moveTo(int poleNo){
    Move = true;
    glutTimerFunc(0,timer, 0);
}


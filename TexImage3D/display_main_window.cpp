//
//  display_main_window.cpp
//  TexImage3D
//
//  Created by Uddipan Mukherjee on 9/25/15.
//  Copyright (c) 2015 Uddipan Mukherjee. All rights reserved.
//

#include "essentials.h"
#include "setShaders.h"
#include "ReadVolume.h"

//------------------globals----------------------
GLuint texName, texTrans, locH, locW, locTheta, locPhi, locScale;
//float theta=0.0, phi=0.0, scale=0.5; //rotation and scale params;

//nucleon
float theta = -1.4;
float phi = 3.1;
float scale = 0.55;

Vec2 curr_pos;
Vec2 last_pos;

int frame=0,time_elapsed,timebase=0;
char s[256];
//----------------------------------------------------------------------

//------------------variables and GLSL equivalents----------------------
extern int WindowH, WindowW;
extern int window1,window2;
extern GLuint v,f,f2,p;
//----------------------------------------------------------------------

//-------------volume-------------------------
extern uchar *volume;
extern float *volumeFloat;
extern short *volume16int;
extern int Xdim,Ydim,Zdim;//volume dimensions
extern vector<float4> knot_color;

extern void WindowDump(void);
bool DumpCheck = false;
//--------------------------------------------

//---------------VOLUME----------------------------------
//setup the volume data as a 3D texture
void SetupVolTexture(){
    glGenTextures(1, &texName);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, texName);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, Xdim, Ydim, Zdim, 0, GL_RED, GL_UNSIGNED_BYTE, volume);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, Xdim, Ydim, Zdim, 0, GL_RED, GL_UNSIGNED_SHORT, volume16int);
    
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, Xdim, Ydim, Zdim, 0, GL_RED, GL_FLOAT, volumeFloat);
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, Xdim, Ydim, Zdim, 0, GL_RED, GL_SHORT, volumeInt);
    glUniform1i(glGetUniformLocation(p, "texVol"), 0);
    
    glEnable(GL_TEXTURE_3D);
}
//------------------------------------------------------------


//setup the transfer function as a 1D texture
void SetupTransferTexture(){
    glGenTextures(1, &texTrans);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, texTrans);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // set up Tx fn. from the other window
    float *TF = &knot_color[0].x;
    
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 9, 0, GL_RGBA, GL_FLOAT, TF);
    glUniform1i(glGetUniformLocation(p, "texTrans"), 1);
    
    glEnable(GL_TEXTURE_1D);
}


//Main display function for primary window
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);
    glVertex3f(-2,-2,0);
    glVertex3f(-2,2,0);
    glVertex3f(2,2,0);
    glVertex3f(2,-2,0);
    glEnd();
    glFlush();
    if(!DumpCheck){
        //WindowDump();
        DumpCheck = true;
    }
}



void init()
{
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    
    setup_volume();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    SetupVolTexture();
    SetupTransferTexture();
    
    locW = glGetUniformLocation(p, "winW");
    locH = glGetUniformLocation(p, "winH");
    glUniform1f(locW, float(WindowW));
    glUniform1f(locH, float(WindowH));
    
    locTheta = glGetUniformLocation(p, "theta");
    glUniform1f(locTheta, theta);
    
    locPhi = glGetUniformLocation(p, "phi");
    glUniform1f(locPhi, phi);
    
    locScale = glGetUniformLocation(p, "scale");
    glUniform1f(locScale, scale);
    
}



void reshape(int w, int h)
{
    glUniform1f(locW, float(w));
    glUniform1f(locH, float(h));
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -4.0);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            exit(0);
            break;
        case 'z':
            theta+=0.01;
            glUniform1f(locTheta, theta);
            glutPostRedisplay();
            break;
        case 'x':
            theta-=0.01;
            glUniform1f(locTheta, theta);
            glutPostRedisplay();
            break;
        case 'q':
            phi+=0.01;
            glUniform1f(locPhi, phi);
            glutPostRedisplay();
            break;
        case 'a':
            phi-=0.01;
            glUniform1f(locPhi, phi);
            glutPostRedisplay();
            break;
        case 'o':
            scale+=0.01;
            if(scale<0.2)scale=0.2;
            glUniform1f(locScale, scale);
            glutPostRedisplay();
            break;
        case 'p':
            scale-=0.01;
            //if(scale<0.2)scale=0.2;
            glUniform1f(locScale, scale);
            glutPostRedisplay();
            break;
        default:
            break;
            
    }
}



Vec2 model_coords( int x, int y )
{
    double d( 1.0 - scale );
    return Vec2(
                d * ( (2.0 * x) / WindowW - 1.0 ),
                d * ( 1.0 - (2.0 * y) / WindowH )
                );
}

// This is the function that we will tell OpenGL to call
// whenever the cursor is moved WHILE a button is held down.
void mouse_motion( int x, int y )
{
    Vec2 P( model_coords( x, y ) );
    Vec2 delta( P - curr_pos );
    last_pos = curr_pos;
    curr_pos = P;
    float old_theta = theta, old_phi = phi;
    theta -= 1.5 * delta.y;
    phi   += 1.5 * delta.x;
    if(theta != old_theta)
    {
        cout << "theta: " << theta << "\n";
    }
    if(phi != old_phi)
    {
        cout << "phi: " << phi << "\n";
    }
    glUniform1f(locTheta, theta);
    glUniform1f(locPhi, phi);
    glutPostRedisplay();
}

// This is the function that we will tell OpenGL to call
// whenever a mouse button is pressed or released.
void mouse_button( int button, int state, int x, int y )
{
    Vec2 P( model_coords( x, y ) );
    if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        last_pos = P;
        curr_pos = P;
        glUniform1f(locTheta, theta);
        glUniform1f(locPhi, phi);
        glutPostRedisplay();
    }
    else if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        glUniform1f(locTheta, theta);
        glUniform1f(locPhi, phi);
        glutPostRedisplay();
    }
    else if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
    }
}

/*------------------------------------------------------------------------------
 idle function used to display frame rate
 ------------------------------------------------------------------------------*/
void idle()
{
    glutSetWindow(window1);
    frame++;
    time_elapsed=glutGet(GLUT_ELAPSED_TIME);
    if (time_elapsed - timebase > 1000) {
        sprintf(s,"volUMrender: FPS:%3.1f",
                frame*1000.0/(time_elapsed-timebase));
        timebase = time_elapsed;
        frame = 0;
    }
    glutSetWindowTitle(s);
    glutPostRedisplay();
}
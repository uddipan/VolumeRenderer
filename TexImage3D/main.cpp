/**********************************************************************************
 *  Simple Volume Render using GLSL shaders
 *  This program demonstrates rendering a three-dimensional texture.
 *  It loads a 3D volume as a 3D texture and a transfer fn. as a 1D texture.
 *  Ray marching is used to collect the scalar values and render the color.
 *********************************************************************************/

#include "essentials.h"
#include "setShaders.h"
#include "ReadVolume.h"

#ifdef GL_VERSION_1_2

int WindowH = 600, WindowW = 600; //window width and height
int window2_width=650, window2_height= 432;
int window1, window2;

//-----------GLUI-----------------------------
GLUI *glui, *glui2;
GLUI_Panel *object_panel,*d_panel,*a_panel;
GLUI_Scrollbar *sb_red,*sb_green,*sb_blue,*sb_pos;
GLUI_Spinner *spinner_red,*spinner_green,*spinner_blue,*spinner_pos;
GLUI_Button *del_button,*add_button;
#define RED_SCROLL 1
#define GREEN_SCROLL 2
#define BLUE_SCROLL 3
#define RED_SPIN 4
#define GREEN_SPIN 5
#define BLUE_SPIN 6
#define POS_SCROLL 7
#define POS_SPIN 8
#define ADD_ID 9
#define DELETE_ID 10
GLfloat inten01[]={0.0,0.0,0.0};
GLint vox_pos,inten255[]={0,0,0};

//--------------------------------------------


//------------extern functions-----------------
extern void display(void);
extern void init();
extern void keyboard(unsigned char key, int x, int y);
extern void reshape(int w, int h);
extern void mouse_motion( int x, int y );
extern void mouse_button( int button, int state, int x, int y );
extern void idle();

extern void display_second(void);
extern void control_cb( int control );
extern void window2_mouse(int button, int state, int x, int y);
extern void window2_motion(int x,int y);
extern void window2_keyboard(unsigned char key, int x, int y);
extern void window2_passive_motion(int x, int y);
//---------------------------------------------



int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE| GLUT_RGB | GLUT_DEPTH);
    
    // --------------------------------------------------------
    //----------------MAIN WINDOW------------------------------
    
    glutInitWindowSize(WindowH, WindowW);
    glutInitWindowPosition(100, 100);
    window1 = glutCreateWindow(argv[0]);
    setShaders();
    init();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc (keyboard);
    glutMotionFunc( mouse_motion );
    glutMouseFunc( mouse_button );
    glutIdleFunc(idle);
    
    
    //---------------------------------------------------------
    //window 2: transfer characteristics window
    glutInitWindowSize(window2_width,window2_height);
    window2 = glutCreateWindow("Transfer function characteristics");
    glutPositionWindow(700,100);
    glutDisplayFunc(display_second);
    glutMouseFunc(window2_mouse);
    glutMotionFunc(window2_motion);
    glutKeyboardFunc(window2_keyboard);
    glutPassiveMotionFunc(window2_passive_motion);
    
    /*** Create the side subwindow ***/
    glui = GLUI_Master.create_glui_subwindow( window2,GLUI_SUBWINDOW_BOTTOM );
    //editing key color
    //----------------------------------------------------------------------------------------------------
    object_panel=glui->add_panel("SELECTED KEY");
    sb_red=new GLUI_Scrollbar(object_panel,"red",GLUI_SCROLL_HORIZONTAL,&inten01[0],RED_SCROLL,control_cb);
    sb_red->set_float_limits(0,1);
    sb_green=new GLUI_Scrollbar(object_panel,"green",GLUI_SCROLL_HORIZONTAL,&inten01[1],GREEN_SCROLL,control_cb);
    sb_green->set_float_limits(0,1);
    sb_blue=new GLUI_Scrollbar(object_panel,"blue",GLUI_SCROLL_HORIZONTAL,&inten01[2],BLUE_SCROLL,control_cb);
    sb_blue->set_float_limits(0,1);
    glui->add_column_to_panel(object_panel,true);
    spinner_red = new GLUI_Spinner( object_panel, "RED", &inten255[0], RED_SPIN,control_cb );
    spinner_red->set_int_limits(0,255);spinner_red->set_speed(0.008);
    spinner_green = new GLUI_Spinner( object_panel, "GREEN", &inten255[1], GREEN_SPIN,control_cb );
    spinner_green->set_int_limits(0,255);spinner_green->set_speed(0.008);
    spinner_blue = new GLUI_Spinner( object_panel, "BLUE", &inten255[2], BLUE_SPIN,control_cb );
    spinner_blue->set_int_limits(0,255);spinner_blue->set_speed(0.008);
    glui->add_column_to_panel(object_panel,true);
    char name[]={"    VOXEL POSITION"};
    glui->add_statictext_to_panel(object_panel,name);
    sb_pos=new GLUI_Scrollbar(object_panel,"pos",GLUI_SCROLL_HORIZONTAL,&vox_pos,POS_SCROLL,control_cb);
    sb_pos->set_int_limits(0,255);
    glui->add_separator_to_panel(object_panel);
    spinner_pos= new GLUI_Spinner( object_panel, "VALUE", &vox_pos, POS_SPIN,control_cb );
    sb_pos->set_int_limits(0,255);
    object_panel->disable();
    
    //deleting a key
    //-------------------------------------------------------------------------
    glui->add_column(true);
    d_panel=glui->add_panel("DEL SELECTED KEY");
    del_button=new GLUI_Button( d_panel, "Delete key", DELETE_ID, control_cb );
    d_panel->disable();
    
    //adding a key
    //------------------------------------------------------------
    a_panel=glui->add_panel("ADD KEY");
    add_button=new GLUI_Button( a_panel, "Add key", ADD_ID, control_cb );
    glui->set_main_gfx_window( window2 );
    
    //-----------------------------------------------------------
    glutMainLoop();

    return 0;
}
#else
int main(int argc, char** argv)
{
    fprintf (stderr, "This program demonstrates a feature which is not in OpenGL Version 1.0 or 1.1.\n");
    fprintf (stderr, "If your implementation of OpenGL has the right extensions,\n");
    fprintf (stderr, "you may be able to modify this program to make it run.\n");
    return 0;
}
#endif

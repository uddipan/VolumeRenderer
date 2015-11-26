//
//  display_secondary_window.cpp
//  TexImage3D
//
//  Created by Uddipan Mukherjee on 9/25/15.
//  Copyright (c) 2015 Uddipan Mukherjee. All rights reserved.
//

#include "essentials.h"

extern vector<int> histogram;

extern int highest_bin;//tally of the highest bin
//int bin_limits[]={0,2,4,8,12,16,24,32,48,64,80,96,128,160,192,224,256};
float knotpos[] = {0.0f,32.0f,64.0f,96.0f,128.0f,160.0f,192.0f,224.0f,256.0f};
vector<float> knots (knotpos, knotpos + sizeof(knotpos) / sizeof(float) );//knot positions
float4 ind_color[] = {
    {  0.0, 0.0, 0.0, 0.0, },
    {  1.0, 0.0, 0.0, 0.8, },
    {  1.0, 0.5, 0.0, 0.8, },
    {  1.0, 1.0, 0.0, 0.7, },
    {  0.0, 1.0, 0.0, 0.5, },
    {  0.0, 1.0, 1.0, 0.6, },
    {  0.0, 0.0, 1.0, 0.8, },
    {  1.0, 0.0, 1.0, 0.2, },
    {  0.0, 0.0, 0.0, 0.0, },
};
vector<float4> knot_color(ind_color,ind_color+sizeof(ind_color)/(4*sizeof(float)));
bool KEY=false;
bool HIGHLIGHT=false;
bool ADD_KEY=false;
bool KEY_VICINITY=false;
unsigned int KEY_SELECTED=0;
bool KEY_DELETED=false;
float2 position,new_position,passive_position;//for mouse movement in tx window
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

extern int window1,window2;
extern int window2_width, window2_height;
extern GLfloat inten01[];
extern GLint vox_pos,inten255[];

extern GLUI *glui, *glui2;
extern GLUI_Panel *object_panel,*d_panel,*a_panel;
extern GLUI_Scrollbar *sb_red,*sb_green,*sb_blue,*sb_pos;
extern GLUI_Spinner *spinner_red,*spinner_green,*spinner_blue,*spinner_pos;
extern GLUI_Button *del_button,*add_button;

extern GLuint texName, texTrans, locH, locW, locTheta, locPhi, locScale;
extern float theta, phi, scale;
extern void SetupTransferTexture();

void set_key_position(vector<float> keys){
    float *knots=(float*)malloc(keys.size()*sizeof(float));
    copy(keys.begin(),keys.end(),knots);
    delete knots;
}

//the color is put on a texture memory
void init_color(vector<float4> a){
    float4 *transferFunc;
    transferFunc=(float4*)malloc(a.size()*4*sizeof(float));
    copy( a.begin(), a.end(), transferFunc);
    delete transferFunc;
}

void window1_callback()
{
    set_key_position(knots);
    init_color(knot_color);
    glutSetWindow(window1);
    SetupTransferTexture();
    glUniform1f(locTheta, theta);
    glutPostRedisplay();
}

/*-------------------------------------------------------------------------------
 GLUI CONTROL CALLBACK
 -------------------------------------------------------------------------------*/
void control_cb( int control )
{
    if(control==RED_SCROLL){
        knot_color[KEY_SELECTED].x=inten01[0];
        spinner_red->set_int_val(int(255*inten01[0]));
        window1_callback();
    }
    else if(control==GREEN_SCROLL){
        knot_color[KEY_SELECTED].y=inten01[1];
        spinner_green->set_int_val(int(255*inten01[1]));
        window1_callback();
    }
    else if(control==BLUE_SCROLL){
        knot_color[KEY_SELECTED].z=inten01[2];
        spinner_blue->set_int_val(int(255*inten01[2]));
        window1_callback();
    }
    else if(control==RED_SPIN){
        knot_color[KEY_SELECTED].x=float(inten255[0])/255.0f;
        sb_red->set_float_val(knot_color[KEY_SELECTED].x);
        window1_callback();
    }
    else if(control==GREEN_SPIN){
        knot_color[KEY_SELECTED].y=float(inten255[1])/255.0f;
        sb_green->set_float_val(knot_color[KEY_SELECTED].y);
        window1_callback();
    }
    else if(control==BLUE_SPIN){
        knot_color[KEY_SELECTED].z=float(inten255[2])/255.0f;
        sb_blue->set_float_val(knot_color[KEY_SELECTED].z);
        window1_callback();
    }
    else if(control==POS_SCROLL){
        knots[KEY_SELECTED]=vox_pos;
        spinner_pos->set_int_val(vox_pos);
        window1_callback();
    }
    else if (control==ADD_ID){
        ADD_KEY=true;
        HIGHLIGHT=false;
        object_panel->disable();
        d_panel->disable();
        glutPostRedisplay();
    }
    else if(control==DELETE_ID){
        ////delete the selected key
        if((KEY_SELECTED!=0)&&(KEY_SELECTED!=knots.size()-1)){
            knots.erase(knots.begin()+KEY_SELECTED);
            knot_color.erase(knot_color.begin()+KEY_SELECTED);
            HIGHLIGHT=false;
            object_panel->disable();
            d_panel->disable();
            glutPostRedisplay();
            window1_callback();
        }
    }
}




/*-------------------------------------------------------------------------------
 draw a circle-required for drawing transfer function knots
 -------------------------------------------------------------------------------*/
void draw_circle(float2 center,float4 color){
    glColor4f(0.0f,0.0f,0.0f,1.0f);
    glPointSize(14);
    glBegin(GL_POINTS);
    glVertex2f(center.x,center.y);
    glEnd();
    
    glColor3f(color.x,color.y,color.z);
    glPointSize(12);
    glBegin(GL_POINTS);
    glVertex2f(center.x,center.y);
    glEnd();
}



void display_second(void)
{
    glClearColor( .9f, .9f, .9f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    //first display the bins
    //float left=-0.95,right=left+0.025,up,down=-0.3;
    float left=-0.75,right=left+0.025,up,down=-0.3;
    void * font = GLUT_BITMAP_HELVETICA_12;
    for(int i=0;i<64;i++){
        up=(float(histogram[i])/float(highest_bin))*1.2+down;
        glColor4f(1,0.3,0.3,0.95f);
        glBegin(GL_POLYGON);
        glVertex2f(left,down);
        glVertex2f(right,down);
        glVertex2f(right,up);
        glVertex2f(left,up);
        glEnd();
        //bbox of each bin
        glColor3f(0.1,0.1,0.1);
        //glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        glVertex2f(left,down);
        glVertex2f(right,down);
        glVertex2f(right,up);
        glVertex2f(left,up);
        glEnd();
        
        //marker for each bin
        glColor3f(0.0,0.0,0.0);
        glRasterPos2f(left, down-0.1f);
        vector<char> str;
        int tally=4*i;
        if(tally%16==0){
            if (tally/100!=0){
                str.push_back(char(tally/100+48));
                str.push_back(char((tally%100)/10+48));
            }
            else if (tally/10!=0)str.push_back(char(tally/10+48));
            str.push_back(char(tally%10+48));
            for (unsigned ii=0;ii<str.size();ii++){
                glutBitmapCharacter(font, str[ii]);
            }
        }
        
        left=right;
        right=left+0.025;
    }
    
    //display the overall bbox
    glColor3f(0.1,0.1,0.1);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.75f,-0.3f);
    glVertex2f(0.85f,-0.3f);
    glVertex2f(0.85f,0.9f);
    glVertex2f(-0.75f,0.9f);
    glEnd();
    
    
    //display text
    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2f(-0.1f, -0.48f);
    string s = "VOXEL VALUES";
    for (string::iterator i = s.begin(); i != s.end(); ++i){
        char c = *i;
        glutBitmapCharacter(font, c);
    }
    
    glColor3f(0.0, 0.0, 0.0);
    string trans="TRANSPARENCY";
    float count=0.0f;
    for (string::iterator i = trans.begin(); i != trans.end(); ++i){
        char c = *i;
        glRasterPos2f(-0.9f, 0.82f-count);
        glutBitmapCharacter(font, c);
        count+=0.1f;
    }
    
    string tr_scale="00.51";
    glRasterPos2f(-0.8f, -0.3);
    glutBitmapCharacter(font, tr_scale[0]);
    glRasterPos2f(-0.82f, 0.3);
    glutBitmapCharacter(font, tr_scale[1]);
    glutBitmapCharacter(font, tr_scale[2]);
    glutBitmapCharacter(font, tr_scale[3]);
    glRasterPos2f(-0.8f, 0.88);
    glutBitmapCharacter(font, tr_scale[4]);
    
    //now draw the knots and line between them
    glEnable(GL_LINE_SMOOTH);
    glColor3f(0.5,0.5,0.5);
    glBegin(GL_LINE_STRIP);
    for(unsigned u=0;u<knots.size();u++){
        glVertex2f((1.6f/256.0f)*knots[u]-0.75f,knot_color[u].w*1.2-0.3);
    }
    glEnd();
    
    for(unsigned u=0;u<knots.size();u++){
        float2 center;
        center.x=(1.6f/256.0f)*knots[u]-0.75f;
        center.y=knot_color[u].w*1.2-0.3;
        draw_circle(center,knot_color[u]);
    }
    
    
    //now draw a highlight around the selected marker
    if(HIGHLIGHT){
        float2 center;
        center.x=(1.6f/256.0f)*knots[KEY_SELECTED]-0.75f;
        center.y=knot_color[KEY_SELECTED].w*1.2-0.3;
        glColor3f(0.0,0.0,0.0);
        glBegin(GL_LINE_LOOP);
        glVertex2f(center.x-0.032f,center.y-0.044f);
        glVertex2f(center.x-0.032f,center.y+0.044f);
        glVertex2f(center.x+0.032f,center.y+0.044f);
        glVertex2f(center.x+0.032f,center.y-0.044f);
        glEnd();
    }
    
    if((ADD_KEY)&&(KEY_VICINITY)){
        glColor3f(0.0,0.0,1.0);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1,1110);
        glBegin(GL_LINES);
        glVertex2f(-0.75f,passive_position.y);
        glVertex2f(passive_position.x,passive_position.y);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(passive_position.x,passive_position.y);
        glVertex2f(passive_position.x,-0.3f);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }
    
    
    glutSwapBuffers();
}

void window2_mouse(int button, int state, int x, int y)
{
    //check proximity of any key position
    //if within a bound, make that position the selected one
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        position.x=-1.0f+(2.0f*float(x)/float(window2_width));
        position.y=-1.0f+(2.0f-2.0f*float(y)/float(window2_height));
        if((ADD_KEY)&&(KEY_VICINITY))
        {
            //add a key
            //first find the position of the new key
            unsigned pos2ins;
            for(unsigned u=0;u<knots.size();u++){
                float posx=(1.6f/256.0f)*knots[u]-0.75f;
                if(posx>passive_position.x){
                    pos2ins=u-1;
                    break;
                }
            }
            
            float4 color2ins;
            float d=((passive_position.x+0.75f)*(256.0f/1.6f)-knots[pos2ins])/(knots[pos2ins+1]-knots[pos2ins]);
            color2ins.x=knot_color[pos2ins].x+d*(knot_color[pos2ins+1].x-knot_color[pos2ins].x);
            color2ins.y=knot_color[pos2ins].y+d*(knot_color[pos2ins+1].y-knot_color[pos2ins].y);
            color2ins.z=knot_color[pos2ins].z+d*(knot_color[pos2ins+1].z-knot_color[pos2ins].z);
            color2ins.w=(passive_position.y+0.3f)/1.2f;
            knot_color.insert(knot_color.begin()+pos2ins+1,color2ins);
            knots.insert(knots.begin()+pos2ins+1,(passive_position.x+0.75f)*(256.0f/1.6f));
            KEY=true;
            HIGHLIGHT=true;
            KEY_SELECTED=pos2ins+1;
            ADD_KEY=false;KEY_VICINITY=false;
            object_panel->enable();
            d_panel->enable();
            sb_red->set_float_val(knot_color[KEY_SELECTED].x);
            sb_green->set_float_val(knot_color[KEY_SELECTED].y);
            sb_blue->set_float_val(knot_color[KEY_SELECTED].z);
            spinner_red->set_int_val(int(255*knot_color[KEY_SELECTED].x));
            spinner_green->set_int_val(int(255*knot_color[KEY_SELECTED].y));
            spinner_blue->set_int_val(int(255*knot_color[KEY_SELECTED].z));
            cout<<knots[KEY_SELECTED]<<"\n";
            sb_pos->set_int_limits(knots[KEY_SELECTED-1],knots[KEY_SELECTED+1]);
            spinner_pos->set_int_limits(knots[KEY_SELECTED-1],knots[KEY_SELECTED+1]);
            sb_pos->set_int_val(knots[KEY_SELECTED]);
            spinner_pos->set_int_val(knots[KEY_SELECTED]);
            glutPostRedisplay();
            window1_callback();
        }
        else{
            for(unsigned u=0;u<knots.size();u++){
                float posx=(1.6f/256.0f)*knots[u]-0.75f-position.x;
                //float posy=-0.61f-position.y;
                float posy=knot_color[u].w*1.2-0.3-position.y;
                if((posx*posx+posy*posy)<0.001f){//this key is selected
                    KEY=true;
                    HIGHLIGHT=true;
                    KEY_SELECTED=u;
                    object_panel->enable();
                    d_panel->enable();
                    sb_red->set_float_val(knot_color[u].x);
                    sb_green->set_float_val(knot_color[u].y);
                    sb_blue->set_float_val(knot_color[u].z);
                    spinner_red->set_int_val(int(255*knot_color[u].x));
                    spinner_green->set_int_val(int(255*knot_color[u].y));
                    spinner_blue->set_int_val(int(255*knot_color[u].z));
                    sb_pos->set_int_val(knots[u]);
                    spinner_pos->set_int_val(knots[u]);
                    if(u==0){
                        sb_pos->set_int_limits(0,knots[u+1]);
                        spinner_pos->set_int_limits(0,knots[u+1]);
                    }
                    else if(u==knots.size()-1){
                        sb_pos->set_int_limits(knots[u-1],knots[u]);
                        spinner_pos->set_int_limits(knots[u-1],knots[u]);
                    }
                    else{
                        sb_pos->set_int_limits(knots[u-1],knots[u+1]);
                        spinner_pos->set_int_limits(knots[u-1],knots[u+1]);
                    }
                    glutPostRedisplay();
                    break;
                }
                else {
                    HIGHLIGHT=false;
                    sb_red->set_float_val(0.0f);
                    sb_green->set_float_val(0.0f);
                    sb_blue->set_float_val(0.0f);
                    spinner_red->set_int_val(0);
                    spinner_green->set_int_val(0);
                    spinner_blue->set_int_val(0);
                    spinner_pos->set_int_limits(0,0);
                    spinner_pos->set_int_val(0);
                    object_panel->disable();
                    d_panel->disable();
                    glutPostRedisplay();
                }
            }
        }
    }
    if(button==GLUT_LEFT_BUTTON && state==GLUT_UP){
        //ADD_KEY=false;
        KEY=false;
    }
}



void window2_motion(int x,int y)
{
    //if any key is selected, move it around
    if(KEY==true){
        float posx=(1.6f/256.0f)*knots[KEY_SELECTED]-0.75f-position.x;
        float posy=knot_color[KEY_SELECTED].w*1.2-0.3-position.y;
        if((posx*posx+posy*posy)<0.001f){
            new_position.x=-1.0f+(2.0f*float(x)/float(window2_width));
            new_position.y=-1.0f+(2.0f-2.0f*float(y)/float(window2_height));
            float delta_x=new_position.x-position.x;
            float delta_y=new_position.y-position.y;
            if((KEY_SELECTED!=0)&&(KEY_SELECTED!=knots.size()-1))
                knots[KEY_SELECTED]+=delta_x/(1.6f/256.0f);
            knot_color[KEY_SELECTED].w+=delta_y/(1.2f);
            if(knot_color[KEY_SELECTED].w<=0.0f)knot_color[KEY_SELECTED].w=0.0f;
            if(knot_color[KEY_SELECTED].w>=1.0f)knot_color[KEY_SELECTED].w=1.0f;
            if(knots[KEY_SELECTED]<=0.0f)knots[KEY_SELECTED]=0.0f;
            if(knots[KEY_SELECTED]>256.0f)knots[KEY_SELECTED]=256.0f;
            //cout<<KEY_SELECTED<<"\t"<<knots.size();
            if((KEY_SELECTED!=0)&&(knots[KEY_SELECTED]<=knots[KEY_SELECTED-1]))
                knots[KEY_SELECTED]=knots[KEY_SELECTED-1];
            if((KEY_SELECTED!=knots.size()-1)&&(knots[KEY_SELECTED]>=knots[KEY_SELECTED+1]))
                knots[KEY_SELECTED]=knots[KEY_SELECTED+1];
            position.x=new_position.x;
            position.y=new_position.y;
            sb_pos->set_int_val(knots[KEY_SELECTED]);
            spinner_pos->set_int_val(knots[KEY_SELECTED]);
            glutPostRedisplay();
            window1_callback();
        }
    }
}

void window2_keyboard(unsigned char key, int x, int y){
    switch(key){
        case 'p':
            for(unsigned u=0; u<knot_color.size(); u++)
            {
                cout << knot_color[u].x << "," << knot_color[u].y << ","
                     << knot_color[u].z << "," << knot_color[u].w << "\n";
            }
            glutPostRedisplay();
            window1_callback();
            break;
        case 'r':
            float knotpos2[] = {0.0f,32.0f,64.0f,96.0f,128.0f,160.0f,192.0f,224.0f,256.0f};
            vector<float> knots2 (knotpos, knotpos + sizeof(knotpos) / sizeof(float) );//knot positions
            knots=knots2;
            float4 ind_color2[] = {
                {  0.0, 0.0, 0.0, 0.0, },
                {  1.0, 0.0, 0.0, 1.0, },
                {  1.0, 0.5, 0.0, 1.0, },
                {  1.0, 1.0, 0.0, 1.0, },
                {  0.0, 1.0, 0.0, 1.0, },
                {  0.0, 1.0, 1.0, 1.0, },
                {  0.0, 0.0, 1.0, 1.0, },
                {  1.0, 0.0, 1.0, 1.0, },
                {  0.0, 0.0, 0.0, 0.0, },
            };
            vector<float4> knot_color2(ind_color,ind_color+sizeof(ind_color)/(4*sizeof(float)));
            knot_color=knot_color2;
            glutPostRedisplay();
            window1_callback();
            break;
    }
}

void window2_passive_motion(int x, int y){
    if(ADD_KEY){//if in the add key mode
        //if the cursor lies within the vicinity of a
        //line joining 2 adjacent keys, draw guide line
        position.x=-1.0f+(2.0f*float(x)/float(window2_width));
        position.y=-1.0f+(2.0f-2.0f*float(y)/float(window2_height));
        //cout<<position.x<<position.y<<"\n";
        unsigned left,right;
        if((position.x>=-0.75f)&&(position.x<=0.85f)&&(position.y>=-0.3f)&&(position.y<=0.9f)){
            //find the two nearest nbrs
            for(unsigned u=0;u<knots.size();u++){
                float posx=(1.6f/256.0f)*knots[u]-0.75f;
                if(posx>position.x){
                    left=u-1;right=u;
                    break;
                }
            }
            //cout<<left<<right<<"\n";
            //find the distance of the point from the line joining the 2
            float P1,P2,Q1,Q2;
            P1=(1.6f/256.0f)*knots[left]-0.75f;
            P2=(1.6f/256.0f)*knots[right]-0.75f;
            Q1=knot_color[left].w*1.2-0.3;
            Q2=knot_color[right].w*1.2-0.3;
            float A=(Q2-Q1)/(Q1*P2-Q2*P1);
            float B=(P1-P2)/(Q1*P2-Q2*P1);
            float dis=(abs((A*position.x)+(B*position.y)+1))/sqrt(A*A+B*B);
            if(dis<0.015f){
                //find the distance of the point from the line
                KEY_VICINITY=true;
                passive_position.x=position.x;
                passive_position.y=Q1+((Q2-Q1)/(P2-P1))*(position.x-P1);
                glutPostRedisplay();
            }
            else {
                KEY_VICINITY=false;
                glutPostRedisplay();
            }
        }
    }
}



//
//  essentials.h
//  VolRen_shader
//
//  Created by Uddipan Mukherjee on 8/22/14.
//  Copyright (c) 2014 Uddipan Mukherjee. All rights reserved.
//

#ifndef VolRen_shader_essentials_h
#define VolRen_shader_essentials_h

#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>
#include <GLUI/glui.h>

#include "Vec2.h"
#include "Vec3.h"


#define RANK 6
#define BLOCK 8


using namespace std;
typedef unsigned char uchar;


typedef struct
{
    float x,y;
}float2;

typedef struct
{
    float x,y,z,w;
}float4;

// reconstruct structure
struct compressed_tensor
{
    float A[BLOCK][RANK],B[BLOCK][RANK],C[BLOCK][RANK];
    float scale[RANK][RANK][RANK];//scale factor tensor
    
};

#endif

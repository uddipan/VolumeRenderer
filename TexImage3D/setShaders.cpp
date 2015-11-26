//
//  setShaders.cpp
//  VolRen_shader
//
//  Created by Uddipan Mukherjee on 8/22/14.
//  Copyright (c) 2014 Uddipan Mukherjee. All rights reserved.
//

#include "setShaders.h"
#define printOpenGLError() printOglError(__FILE__, __LINE__)


GLuint v,f,f2,p;

void setShaders() {
    
	char *vs = NULL,*fs = NULL,*fs2 = NULL;
    
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	f2 = glCreateShader(GL_FRAGMENT_SHADER);
    
	vs = textFileRead("/GLSLVolRen/TexImage3D/TexImage3D/minimal.vert");
	fs = textFileRead("/GLSLVolRen/TexImage3D/TexImage3D/minimal.frag");
    
	const char * vv = vs;
	const char * ff = fs;
    
	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);
    
	free(vs);free(fs);
    
	glCompileShader(v);
	glCompileShader(f);
    
	printShaderInfoLog(v);
	printShaderInfoLog(f);
	printShaderInfoLog(f2);
    
	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);
    
	glLinkProgram(p);
	printProgramInfoLog(p);
    
	glUseProgram(p);
    
}


void printShaderInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;
    
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;
    
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}


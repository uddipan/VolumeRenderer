//
//  Windowbump.cpp
//  MatchHungarHierarch
//
//  Created by uddipan  on 5/24/14.
//  Copyright (c) 2014 University of California, Irvine. All rights reserved.
//

#include "Windowbump.h"

extern int WindowH, WindowW;


int counter = 1; /* This supports animation sequences */


void WindowDump(void)
{
    int i,j;
    FILE *fptr;
    char fname[255];
    unsigned char *image=new unsigned char[3*WindowW*WindowH];
    
    /* Allocate our buffer for the image */
    if (image == NULL) {
        fprintf(stderr,"Failed to allocate memory for image\n");
        return ;
    }
    
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    
    /* Open the file */

    sprintf(fname,"/GLSLVolRen/TexImage3D/TexImage3D/Images%d.raw",counter);
    if ((fptr = fopen(fname,"wb")) == NULL) {
        fprintf(stderr,"Failed to open file for window dump\n");
        return;
    }

    /* Copy the image into our buffer */
    glReadBuffer(GL_BACK_LEFT);
    glReadPixels(0,0,WindowW,WindowH,GL_RGB,GL_UNSIGNED_BYTE,image);
    fwrite (image , sizeof(unsigned char), 3*WindowW*WindowH, fptr);
    /* Write the raw file */
    /* fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm */
//    for (j=WindowH-1;j>=0;j--) {
//        for (i=0;i<WindowW;i++) {
//            fputc(image[3*j*WindowW+3*i+0],fptr);
//            fputc(image[3*j*WindowW+3*i+1],fptr);
//            fputc(image[3*j*WindowW+3*i+2],fptr);
//        }
//    }
    fclose(fptr);
    

    
    /* Clean up */
    counter++;
    delete[] image;
    return ;
}

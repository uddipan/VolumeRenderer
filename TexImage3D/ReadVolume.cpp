//
//  ReadVolume.cpp
//  Shader_Volren
//
//  Created by Uddipan Mukherjee on 8/25/14.
//  Copyright (c) 2014 Uddipan Mukherjee. All rights reserved.
//

#include "ReadVolume.h"

// 1 = normal scalar volume
// 2 = stanford dataset
#define USE_SCALAR_VOL 1//0

#if USE_SCALAR_VOL == 1
string VolumeName="/GLSLVolRen/TexImage3D/TexImage3D/data/Nucleon";
#elif USE_SCALAR_VOL == 2
string VolumeName="/GLSLVolRen/TexImage3D/TexImage3D/data/bunny/";
#elif USE_SCALAR_VOL == 3
string VolumeName="/GLSLVolRen/TexImage3D/TexImage3D/data/Engine6";
#else
string VolumeName="/GLSLVolRen/TexImage3D/TexImage3D/data/EngineReconstructed16";
#endif

int Xdim,Ydim,Zdim;//3D spec of the volume
uchar *volume;//scalar volume
float *volumeFloat;
short *volume16int;
extern void ReconstructTucker (compressed_tensor &T, uchar * volume);

vector<int> histogram(64,0);
int highest_bin=0;//tally of the highest bin

void setup_volume(){
    
#if USE_SCALAR_VOL < 2
    string dat = VolumeName + ".dat";
    string raw = VolumeName + ".raw";
    
    //first read the .dat file to obtain the data dimensions
    string st;
    vector<string> info;//store the data from the dat file
    ifstream infile(dat.c_str());
    if (!infile) {
        cerr << "Can't open input file " << endl;
        exit(1);
    }
    while(!infile.eof()){
        infile>>st;
        info.push_back(st);
    }
    infile.close();
    Xdim=atoi(info[5].c_str());Ydim=atoi(info[6].c_str());Zdim=atoi(info[7].c_str());
    
    size_t size = Xdim*Ydim*Zdim;
    volume=(uchar*) malloc (size);
#endif
    
#if USE_SCALAR_VOL == 1
    
    //now read the raw file for data
    FILE *pFile= fopen ( raw.c_str() , "rb" );
    if (!pFile) {
        fprintf(stderr, "Error opening file '%s'\n",pFile);
        exit(1);
    }
    
    size_t result = fread (volume,1,size,pFile);

    fclose (pFile);
    
#elif USE_SCALAR_VOL == 2
    
     //stanford data set
    char* volumeShort;
    Xdim = 512; Ydim = 512; Zdim = 360;
    
    volume=(uchar*) malloc (Xdim * Ydim * Zdim);
    volume16int=(short*) malloc (Xdim * Ydim * Zdim);

    int offset = 0;
    string raw;
    FILE *pFile=NULL;
    for (int i=1; i<=Zdim; i++)
    {
        // read the individual slices
        if (i < 10)
        {
            raw = VolumeName + char(i + 48);
        }
        else if (i < 100)
        {
            raw = VolumeName + char(i/10 + 48) + char(i%10 + 48);
        }
        else
        {
            raw = VolumeName + char(i/100 + 48) + char((i%100)/10 + 48) + char(i%10 + 48);
        }
        
        ifstream myfile;
        long size;
        ifstream file (raw, ios::in|ios::binary|ios::ate);
        size = file.tellg();
        file.seekg (0, ios::beg);
        volumeShort = new char [size];
        file.read (volumeShort, size);
        file.close();

        for (int i=0; i<Xdim*Ydim; i++)
        {
            short s = ((volumeShort[2*i] << 8) | volumeShort [2*i+1]) ;
            volume[i + offset] = uchar(s);//(volumeShort[2*i+1]);
            volume16int[i + offset] = s;
        }
        offset += Xdim*Ydim;
    }
#elif USE_SCALAR_VOL == 3
    string VolumeName="/GLSLVolRen/Tucker_ALS_reconstruction/Tucker_ALS_reconstruction/engine_block_8_rank__tucker.txt";
    int rank = 6;
    int dimx = 256, dimy = 256, dimz = 112;
    Xdim = dimx; Ydim = dimy, Zdim = dimz;
    
    // Read file
    ifstream infile(VolumeName.c_str());
    if (!infile) {
        cerr << "Can't open input file " << endl;
        exit(1);
    }
    
    string block,model,corename;
    int x,y,z;
    volume =(uchar*)malloc(dimx*dimy*dimz);
    //volume = new uchar[dimx*dimy*dimz];
    uchar* blockvol = (uchar*)malloc(512);
    
    while(!infile.eof()){
        infile >> model;
        for (int i=0; i<dimx; i+=8)
        {
            for(int j=0; j<dimy; j+=8)
            {
                for(int k=0; k<dimz; k+=8)
                {
                    infile >> block;
                    infile >> x >> y >> z;
                    //cout <<x << " " << y <<" "<< z <<"\n";
                    compressed_tensor Tsub;
                    float fl;
                    for(int m=0; m<8; m++)
                    {
                        for(int l=0; l<rank; l++)
                        {
                            infile >> fl;
                            Tsub.A[m][l] = fl;
                        }
                    }
                    for(int m=0; m<8; m++)
                    {
                        for(int l=0; l<rank; l++)
                        {
                            infile >> fl;
                            Tsub.B[m][l] = fl;
                        }
                    }
                    for(int m=0; m<8; m++)
                    {
                        for(int l=0; l<rank; l++)
                        {
                            infile >> fl;
                            Tsub.C[m][l] = fl;
                        }
                    }
                    
                    // core
                    infile >> corename;
                    for(int m=0; m<rank; m++)
                    {
                        for(int l=0; l<rank; l++)
                        {
                            for(int h=0; h<rank; h++)
                            {
                                infile >> fl;
                                Tsub.scale[m][l][h] = fl;
                            }
                        }
                    }
                    
                    // reconstruct
                    //uchar * blockvol=(uchar*) malloc (512);
                    ReconstructTucker(Tsub, blockvol);
                    int idx=0;
                    for(int m=0; m<8;m++)
                    {
                        for(int l=0; l<8; l++)
                        {
                            for(int h=0; h<8; h++)
                            {
                                int index = (h+k)*(dimx*dimy) + (j+l)*dimx + (m+i);
                                volume[index]= blockvol[idx++];
                                //cout << index << "," << float(volume[index]) <<"\n";
                            }
                        }
                    }
                    //delete[] blockvol;
                    
                }
            }
        }
    }
    infile.close();

   

#else
    
    //floatvol
    volumeFloat = new float[size];
    ifstream myfile (raw);
    if (myfile.is_open())
    {
        for (int i=0; i<Xdim*Ydim*Zdim; i++)
        {
            myfile >> volumeFloat[i];
            if (volumeFloat[i] < 0)
            {
                volumeFloat[i] = 0;
            }
            volume[i] = uchar(round(volumeFloat[i]));
        }
        myfile.close();
    }
#endif
    
    // set up the histogram for display in the Tx fn window
    for (int i=0; i<Xdim*Ydim*Zdim; i++)
    {
        histogram[volume[i]/4]++;
    }
    
    for(int i=0;i<64;i++){
        if(highest_bin<histogram[i])
            highest_bin=histogram[i];
    }
    
    free(volume);
}
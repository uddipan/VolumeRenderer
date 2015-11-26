//
//  Tucker_reconstruction.cpp
//  Tucker_ALS_reconstruction
//
//  Created by Uddipan Mukherjee on 9/28/15.
//  Copyright (c) 2015 Uddipan Mukherjee. All rights reserved.
//

#include "essentials.h"

// Routine for reconstructing a tensor using Tucker decomposition


void ReconstructTucker (compressed_tensor &T, uchar * volume)
{
    int idx;
    
    // take one column from each matrix
    for (int i = 0; i < RANK; i++)
    {
        for (int j = 0; j < RANK; j++)
        {
            for (int k = 0; k < RANK; k++)
            {
                
                for (int p=0; p<BLOCK; p++)
                {
                    for (int q=0; q<BLOCK; q++)
                    {
                        for (int r=0; r<BLOCK; r++)
                        {
                            idx = r + q*BLOCK + p*BLOCK*BLOCK;
                            volume[idx] += uchar(T.A[i][p] * T.B[j][q] *T.C[k][r] * T.scale[i][j][k]);
                        }
                    }
                }
                
            }
        }
    }
}
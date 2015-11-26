#define GL_GLEXT_PROTOTYPES
#include<stdio.h>
#include <cutil_inline.h>
#include <cutil_math.h>
#include<iostream>
#include<cuda_gl_interop.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuPrintf.cu"
#include<vector>

typedef unsigned int  uint;
typedef unsigned char uchar;
using namespace std;

typedef struct {
	float4 m[3];
} float3x4;

__constant__ float3x4 c_invViewMatrix;  // inverse view matrix


struct HIT{
	float n,f;
};

//struct for defining a ray
struct RAY{
	float3 origin,direction;
};

cudaArray *d_volumeArray = 0;
cudaArray* d_transferFuncArray;
cudaArray* posarray;



texture<uchar,  3, cudaReadModeNormalizedFloat> tex2[2];

texture<uchar,  3, cudaReadModeNormalizedFloat> tex;         // 3D texture for the volume
//texture<float4, 1, cudaReadModeElementType> transferTex; // 1D transfer function texture for color
texture<float4, 1, cudaReadModeElementType> transferTex; // 1D transfer function texture for color
texture<float, 1, cudaReadModeElementType> key_pos; // 1D transfer function texture for key pos

/*----------------------------------------------------------------------------
copy volume data into 3D texture memory and 
set up the transfer function texture memory
-----------------------------------------------------------------------------*/
void initCuda(uchar *volume, cudaExtent volumeSize){
	// create 3D array
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<uchar>();
	cutilSafeCall( cudaMalloc3DArray(&d_volumeArray, &channelDesc, volumeSize) );

	// copy data to 3D array
	cudaMemcpy3DParms copyParams = {0};
	copyParams.srcPtr   = make_cudaPitchedPtr((void*)volume, volumeSize.width*sizeof(uchar), volumeSize.width, volumeSize.height);
	copyParams.dstArray = d_volumeArray;
	copyParams.extent   = volumeSize;
	copyParams.kind     = cudaMemcpyHostToDevice;
	cutilSafeCall( cudaMemcpy3D(&copyParams) );  

	// set texture parameters
	tex.normalized = true;                      // access with normalized texture coordinates
	tex.filterMode = cudaFilterModeLinear;      // linear interpolation
	tex.addressMode[0] = cudaAddressModeClamp;  // wrap texture coordinates
	tex.addressMode[1] = cudaAddressModeClamp;

	// bind array to 3D texture
	cutilSafeCall(cudaBindTextureToArray(tex, d_volumeArray, channelDesc));
}


//the color is put on a texture memory
void initCuda_color(vector<float4> a){
	float4 *transferFunc;
	transferFunc=(float4*)malloc(a.size()*4*sizeof(float));
	copy( a.begin(), a.end(), transferFunc);


	cudaChannelFormatDesc channelDesc2 = cudaCreateChannelDesc<float4>();
	cutilSafeCall(cudaMallocArray( &d_transferFuncArray, &channelDesc2, a.size(), 1)); 
	cutilSafeCall(cudaMemcpyToArray( d_transferFuncArray, 0, 0, transferFunc, a.size()*4*sizeof(float), cudaMemcpyHostToDevice));



	transferTex.filterMode = cudaFilterModeLinear;
	transferTex.normalized = true;    // access with normalized texture coordinates
	transferTex.addressMode[0] = cudaAddressModeClamp;   // wrap texture coordinates

	// Bind the array to the texture
	cutilSafeCall( cudaBindTextureToArray( transferTex, d_transferFuncArray, channelDesc2));
	delete transferFunc;
}


/*------------------------------------------------------------------------------------------------
set the key positions in the global memory
------------------------------------------------------------------------------------------------*/
void set_key_position(vector<float> keys){
	float *knots=(float*)malloc(keys.size()*sizeof(float));
	copy(keys.begin(),keys.end(),knots);


	cudaChannelFormatDesc channelDesc3 = cudaCreateChannelDesc<float>();
	cutilSafeCall(cudaMallocArray( &posarray, &channelDesc3, keys.size(), 1)); 
	cutilSafeCall(cudaMemcpyToArray( posarray, 0, 0, knots, keys.size()*sizeof(float), cudaMemcpyHostToDevice));

	key_pos.filterMode = cudaFilterModeLinear;
	key_pos.normalized = true;    // access with normalized texture coordinates
	key_pos.addressMode[0] = cudaAddressModeClamp;   // wrap texture coordinates

	// Bind the array to the texture
	cutilSafeCall( cudaBindTextureToArray( key_pos, posarray, channelDesc3));
	delete knots;
}
/*------------------------------------------------------------------------------------------------
interpolation mode
------------------------------------------------------------------------------------------------*/
void setTextureFilterMode(bool bLinearFilter){
	tex.filterMode = bLinearFilter ? cudaFilterModeLinear : cudaFilterModePoint;
}
/*------------------------------------------------------------------------------------------------
free up all CUDA buffers
------------------------------------------------------------------------------------------------*/
void freeCudaBuffers()
{
	cutilSafeCall(cudaFreeArray(d_volumeArray));
	cutilSafeCall(cudaFreeArray(d_transferFuncArray));
	cutilSafeCall(cudaFreeArray(posarray));
}

/*------------------------------------------------------------------------------------------------
free up color buffers
------------------------------------------------------------------------------------------------*/
void freecolorbuffer(){
	cutilSafeCall(cudaFreeArray(d_transferFuncArray));
	cutilSafeCall(cudaFreeArray(posarray));
}
/*------------------------------------------------------------------------------------------------
copy the model view matrix from OpenGL into the device memory.
This is required while defining the origin and direction of the ray
-------------------------------------------------------------------------------------------------*/
void copyInvViewMatrix(float *invViewMatrix, size_t sizeofMatrix)
{
	cutilSafeCall( cudaMemcpyToSymbol(c_invViewMatrix, invViewMatrix, sizeofMatrix) );
}

/*------------------------------------------------------------------------------------------------
transformation of a vector by amatrix
-------------------------------------------------------------------------------------------------*/
__device__ float4 mul(const float3x4 &M, const float4 &v){
	float4 r;
	r.x = dot(v, M.m[0]);
	r.y = dot(v, M.m[1]);
	r.z = dot(v, M.m[2]);
	r.w = 1.0f;
	return r;
}
__device__ float3 mul(const float3x4 &M, const float3 &v){
	float3 r;
	r.x = dot(v, make_float3(M.m[0]));
	r.y = dot(v, make_float3(M.m[1]));
	r.z = dot(v, make_float3(M.m[2]));
	return r;
}
/*------------------------------------------------------------------------------------------------
assign RGBA values
-------------------------------------------------------------------------------------------------*/
__device__ uint rgbaFloatToInt(float4 rgba){
	rgba.x = __saturatef(rgba.x);   // clamp to [0.0, 1.0]
	rgba.y = __saturatef(rgba.y);
	rgba.z = __saturatef(rgba.z);
	rgba.w = __saturatef(rgba.w);
	return (uint(rgba.w*255)<<24) | (uint(rgba.z*255)<<16) | (uint(rgba.y*255)<<8) | uint(rgba.x*255);
}
/*------------------------------------------------------------------------------------------------
RAY BOX INTERSECTION
-------------------------------------------------------------------------------------------------*/
__device__ HIT find_hit(RAY R,float3 boxmin,float3 boxmax){
	HIT H;H.n=0;H.f=0;
	float N,F ;//=-1000.0f,F=-near;//initialize the hit
	N=-100;F=100;
	if(R.direction.x==0){//ray parallel to X plane
		if((R.origin.x<boxmin.x)||(R.origin.x>boxmax.x))
			return H;
	}
	else{
		//compute the intersection distance of the planes
		float T1=(boxmin.x-R.origin.x)/R.direction.x;
		float T2=(boxmax.x-R.origin.x)/R.direction.x;
		if(T1>T2){//swap
			T1=T1+T2;T2=T1-T2;T1=T1-T2;
		}
		if(T1>N)N=T1;
		if(T2<F)F=T2;
	}

	//check with Y planes
	if(R.direction.y==0){//ray parallel to X plane
		if((R.origin.y<boxmin.y)||(R.origin.y>boxmax.y))
			return H;
	}
	else{
		//compute the intersection distance of the planes
		float T1=(boxmin.y-R.origin.y)/R.direction.y;
		float T2=(boxmax.y-R.origin.y)/R.direction.y;
		if(T1>T2){//swap
			T1=T1+T2;T2=T1-T2;T1=T1-T2;
		}
		if(T1>N)N=T1;
		if(T2<F)F=T2;
	}

	//check with Z planes
	if(R.direction.z==0){//ray parallel to X plane
		if((R.origin.z<boxmin.z)||(R.origin.z>boxmax.z))
			return H;
	}
	else{
		//compute the intersection distance of the planes
		float T1=(boxmin.z-R.origin.z)/R.direction.z;
		float T2=(boxmax.z-R.origin.z)/R.direction.z;

		if(T1>T2){//swap
			T1=T1+T2;T2=T1-T2;T1=T1-T2;
		}
		if(T1>N)N=T1;
		if(T2<F)F=T2;
	}

	if(N>F)return H;//box is missed
	if(F<0)return H;//box behind the ray

	H.n=N;H.f=F;
	return H;

}

/*------------------------------------------------------------------------------------------------
MAIN KERNEL
-------------------------------------------------------------------------------------------------*/
__global__ void kernel(uint *d_output,uint width, uint height,float density,float brightness,float projection){
	int no_of_samples=500;//no of samples between near and far hit
	float3 boxMin = make_float3(-1.0f, -1.0f, -1.0f);//the bbox of the volume
	float3 boxMax = make_float3(1.0f, 1.0f, 1.0f);

	int x=threadIdx.x+blockIdx.x*blockDim.x;
	int y=threadIdx.y+blockIdx.y*blockDim.y;
	if ((x >= width) || (y >= height)) return;

	float u = (x / (float) width)*2.0f-1.0f;
	float v = (y / (float) height)*2.0f-1.0f;
	RAY ray;
	if(projection==0){
		//	perspective projection
		ray.origin=make_float3(mul(c_invViewMatrix,make_float4(0.0f, 0.0f, 0.0f, 1.0f)));
		ray.direction = normalize(make_float3(u, v, -2.0f));
		ray.direction = mul(c_invViewMatrix, ray.direction);
	}
	else{
		//orthographic projection used here
		ray.origin=make_float3(mul(c_invViewMatrix,make_float4(u*projection,v*projection, 0.0f, 1.0f)));
		ray.direction = normalize(make_float3(0.0f,0.0f,- 2.0f));
		ray.direction = mul(c_invViewMatrix, ray.direction);
	}

	HIT H;
	H=find_hit(ray,boxMin,boxMax);
	//if there is no hit
	if((H.n==0)&&(H.f==0)) return;
	if (H.n < 0.0f) H.n = 0.0f;     // clamp to near plane
	//now ray marching thru the volume from back to front



	float4 sum=make_float4(0.0f);
	float t=H.f,tstep=0.01;
	for(int i=0;i<no_of_samples;i++){
		//position along the ray
		float3 pos=ray.origin+ray.direction*t;
		pos = pos*0.5f+0.5f;    // map position to [0, 1] coordinates
		// read from 3D texture
		float sample = tex3D(tex, pos.x, pos.y, pos.z);
		// lookup in transfer function texture
		sample=tex1D(key_pos,sample);
		float4 col = tex1D(transferTex, sample/256.0f);
		// accumulate result
		sum = lerp(sum, col, col.w*density);
		t -= tstep;
		if (t < H.n) break;
	}

	sum *= brightness;

	if ((x < width) && (y < height)) {
		// write output color
		uint i = y*width + x;
		d_output[i] = rgbaFloatToInt(sum);
	}
}


void render_kernel(dim3 grids, dim3 threads, uint *d_output, uint width,uint height, float density, 
	float brightness,float projection){
		cudaPrintfInit();
		kernel<<<grids,threads>>>(d_output,width,height,density,brightness,projection);
		cudaPrintfDisplay(stdout, true);
		cudaPrintfEnd();

}
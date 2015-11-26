

uniform sampler3D texVol;
uniform sampler1D texTrans;
uniform float winH, winW;
uniform float theta, phi, scale;


struct Ray{
    vec3 origin, direction;
};

struct Hit{
    float n,f;
};

Ray ShiftOrigin(Ray r){
    r.direction=vec3(0.0,0.0,1.0);
    vec3 x=r.origin,v=x;
    x.x=-(2.0*scale)+v.x*scale*4.0/winW;
    x.y=-(2.0*scale)+v.y*scale*4.0/winH;
    x.z=-4.0;
    
    r.origin=x;
    r.origin.y=cos(theta)*x.y-sin(theta)*x.z;
    r.origin.z=sin(theta)*x.y+cos(theta)*x.z;
    
    x=r.origin;
    r.origin.x=cos(phi)*x.x+sin(phi)*x.z;
    r.origin.z=-sin(phi)*x.x+cos(phi)*x.z;
        
    //the direction for orthographic projection will be
    //that of the center ray, looking at (0,0,0)
    r.direction=vec3(4.0*cos(theta)*sin(phi), -4.0*sin(theta), 4.0*cos(theta)*cos(phi));
    
    return r;
}


Hit find_hit(Ray R){
    vec3 boxmin=vec3(-1.0,-1.0,-1.0);
    vec3 boxmax=vec3(1.0,1.0,1.0);
	Hit H;H.n=0.0;H.f=0.0;
	float N,F ;//initialize the hit
	N=-100.0;F=100.0;
	if(R.direction.x==0.0){//ray parallel to X plane
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
	if(R.direction.y==0.0){//ray parallel to X plane
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
	if(R.direction.z==0.0){//ray parallel to X plane
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
	if(F<0.0)return H;//box behind the ray
    
	H.n=N;H.f=F;
	return H;
    
}

void main()
{
    
    Ray ray;
    ray.origin=gl_FragCoord.xyz;
    ray=ShiftOrigin(ray);
    
    Hit H=find_hit(ray);
    
    if((H.n==0.0)&&(H.f==0.0)){
        gl_FragColor=vec4(0.0,0.0,0.0,0.0);
        return;
    }
	if (H.n < 0.0) H.n = 0.0;     // clamp to near plane
    
	//now ray marching thru the volume from back to front
    vec4 sum=vec4(0.0,0.0,0.0,0.0);
    vec4 InsCol;
    float t=H.f,tstep=0.01,density=0.05;

    for(int i=0;i<200;i++){
        //position along the ray
		vec3 pos=ray.origin+ray.direction*t;
		pos = pos*0.5+0.5;    // map position to [0, 1] coordinates
		// read from 3D texture
        InsCol=texture1D(texTrans,texture3D(texVol,pos).x);
		// accumulate result
		sum=mix(sum,InsCol,InsCol.w*density);
		t -= tstep;
		if (t < H.n) break;
    }
    
    gl_FragColor=sum*2.0;

    
}

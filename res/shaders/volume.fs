#version 330 core

uniform vec3 u_background_color;
uniform float u_absorption;

uniform vec3 u_local_camera_position;
//uniform vec3 u_camera_position;

uniform mat4 u_inverse_model;

uniform float u_step_len;
uniform int u_volume_type;
uniform float u_noise;

// entering from volume.vs
in vec3 v_world_position;
in vec3 v_camera_direction;

out vec3 FragColor;

// proposed function to compute the intersection between two vectors
vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;

    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    return vec2(tNear, tFar);
};

// proposed function to compute the noise -> discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 
vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

const float F3 =  0.3333333;
const float G3 =  0.1666667;
float snoise(vec3 p) {

	vec3 s = floor(p + dot(p, vec3(F3)));
	vec3 x = p - s + dot(s, vec3(G3));
	 
	vec3 e = step(vec3(0.0), x - x.yzx);
	vec3 i1 = e*(1.0 - e.zxy);
	vec3 i2 = 1.0 - e.zxy*(1.0 - e);
	 	
	vec3 x1 = x - i1 + G3;
	vec3 x2 = x - i2 + 2.0*G3;
	vec3 x3 = x - 1.0 + 3.0*G3;
	 
	vec4 w, d;
	 
	w.x = dot(x, x);
	w.y = dot(x1, x1);
	w.z = dot(x2, x2);
	w.w = dot(x3, x3);
	 
	w = max(0.6 - w, 0.0);
	 
	d.x = dot(random3(s), x);
	d.y = dot(random3(s + i1), x1);
	d.z = dot(random3(s + i2), x2);
	d.w = dot(random3(s + 1.0), x3);
	 
	w *= w;
	w *= w;
	d *= w;
	 
	return dot(d, vec4(52.0));
}
 
float snoiseFractal(vec3 m) {
	return   0.5333333* snoise(m)
				+0.2666667* snoise(2.0*m)
				+0.1333333* snoise(4.0*m)
				+0.0666667* snoise(8.0*m);
}

void main()
{
	// ray (pos & dir) 
    vec3 rayOrigin = u_local_camera_position; 
    vec3 fragmentPosition = (u_inverse_model * vec4(v_world_position, 1.0)).xyz;
    vec3 rayDirection = normalize(fragmentPosition - rayOrigin);
	
    vec3 boxMin = vec3(-1.0);
    vec3 boxMax = vec3(1.0);

	// compute intersection with the volume
	vec2 intersectedRay = intersectAABB(rayOrigin, rayDirection, boxMin, boxMax);

	// recover each value from the intersectedRay
	float ta = max(intersectedRay.x, 0.0); // entrada al volum (where is the background)
    float tb = intersectedRay.y;           // sortida del volum (near the viewer)

    //int numStep = 100; 
    //float step = (tb - ta)*u_step_len; // delta value
    //float step = u_step_len;
    float deltaT = u_step_len;
    int numStep = int(ceil((tb - ta) / deltaT));

    float tau = 0; 
    float t = ta + (deltaT/2);

    float opticalThinkness = 0; 
    float absorption = 0;
    for(int i=0; i<numStep; i++){
        // (1) HETEROGENEOUS CASE
        if(u_volume_type == 1){ 
            vec3 samplePosition = rayOrigin + t * rayDirection;
            // noisy version of samplePos
            vec3 modSamplePosition = samplePosition * u_noise; 
            // limit the noise value between [0, 1]
            float density = clamp(snoiseFractal(modSamplePosition), 0.0, 1.0); 
            // sample absCoeff at each step -> we add the noise to introduce varience to absCoeff
            absorption = u_absorption * density;
        }
        // (2) HOMOGENEOUS CASE
        else{
            absorption = u_absorption;
        }
        tau += absorption * deltaT;
       
        t += deltaT; 
    }
	// compute the transmittance
    float transmittance = exp(-tau);
	
    // compute and set the final pixel color
	vec3 finalColor = u_background_color * transmittance;

	FragColor = finalColor;// vec4(finalColor, 1.0);
}
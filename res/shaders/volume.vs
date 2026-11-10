#version 330 core

in vec3 a_vertex;
in vec3 a_normal;
in vec4 a_color;
in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_viewprojection;
uniform vec3 u_camera_position;

//this will store the color for the pixel shader
out vec3 v_position;
out vec3 v_world_position;
out vec3 v_normal;
out vec4 v_color;
out vec2 v_uv;

// add this out, as we will use it in volume.fs -> direction from camera to each vertex
out vec3 v_camera_direction;

void main()
{	
	//calcule the normal in camera space (the NormalMatrix is like ViewMatrix but without traslation)
	v_normal = (u_model * vec4( a_normal, 0.0) ).xyz;
	
	//calcule the vertex in object space
	v_position = a_vertex;
	v_world_position = (u_model * vec4( v_position, 1.0) ).xyz;

	// Task1 calculate camera direction for this vertex!! 
    v_camera_direction = normalize(v_world_position - u_camera_position);
	
	//store the color in the varying var to use it from the pixel shader
	v_color = a_color;

	//store the texture coordinates
	v_uv = a_uv;

	//calcule the position of the vertex using the matrices
	gl_Position = u_viewprojection * vec4( v_world_position, 1.0 );
}
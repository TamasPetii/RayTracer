#version 460 core

//Input Data
layout(location = 0) in vec3 vert_position;

//Output Data
out vec3 frag_position;

void main(){
	frag_position = vert_position;
	gl_Position = vec4(vert_position, 1);
}
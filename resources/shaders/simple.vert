#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_Texture_Coor;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;


out vec3 pass_Normal, pass_Position;
out vec4 four_pass_position;
out mat4 pass_View, pass_Model;
out vec2 pass_Texture_Coor;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	pass_Normal = (NormalMatrix * vec4(in_Normal, 1.0)).xyz;
	//Transform the rel. position matrix into our view and model space
	four_pass_position = (ModelMatrix * vec4(in_Position, 1.0f));
	pass_Position = (ModelMatrix * vec4(in_Position, 1.0f)).xyz;
	//pass_Position = pass_Position + vec3(0.f, -3.f, 0.f);
	pass_View = ViewMatrix;
	pass_Model = ModelMatrix;
	pass_Texture_Coor = in_Texture_Coor;
}

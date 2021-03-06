#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertexPosition_modelspace;

layout(set = 0, binding = 0) uniform MVPUbo
{
	mat4 MVP;
};

layout(set = 0, binding = 1) uniform modelTransformUbo
{
	mat4 modelTransform;
};

layout(location = 0) out vec3 WorldPos0;

void main()
{   
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    gl_Position.y = -gl_Position.y;
    
    WorldPos0 = (modelTransform * vec4(vertexPosition_modelspace, 1.0)).xyz;
}
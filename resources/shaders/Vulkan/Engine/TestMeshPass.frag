#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 WorldPos0;

layout (location = 0) out vec3 WorldPosOut;

void main()
{
    WorldPosOut = WorldPos0;	
} 
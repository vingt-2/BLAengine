#version 330 core

// Ouput data
layout(location = 0) out vec4 color;

uniform sampler2D colorTexture;

in vec2 UV;

void main(){
	color = texture2D(colorTexture, UV);
}
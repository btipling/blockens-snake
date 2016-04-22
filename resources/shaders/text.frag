#version 410 core

smooth in vec2 texcoord;
uniform sampler2D textureMap;

out vec4 color;

void main(void) {
  color = texture(textureMap, texcoord);
}
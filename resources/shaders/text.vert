#version 410 core

layout (location = 0) in vec4 coord;
out vec2 texcoord;

void main(void) {
  gl_Position = coord;
  texcoord = coord.zw;
}
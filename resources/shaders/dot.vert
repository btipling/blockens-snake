#version 410 core

layout(location = 0) in vec4 vPosition;
void main(void) {
    gl_Position = vPosition;
}
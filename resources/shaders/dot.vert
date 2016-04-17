#version 410 core

layout(location = 0) in vec4 vPosition;
flat out int instance_id;
void main(void) {
    gl_Position = vPosition * vec4((gl_InstanceID + 1) * .09, 0.9, 1.0, 1);
    instance_id = gl_InstanceID;
}
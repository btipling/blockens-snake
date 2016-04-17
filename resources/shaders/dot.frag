#version 410 core
flat in int instance_id;
out vec4 color;
void main(void) {
    if (instance_id == 0) {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    } else if (instance_id == 1) {
        color = vec4(1.0, 0.0, 1.0, 1.0);
    } else {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
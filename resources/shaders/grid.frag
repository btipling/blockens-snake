#version 410 core
flat in vec4 vert_color;
out vec4 color;
flat in int do_discard;


void main(void) {
    if (do_discard == 1) {
        discard;
    }
    color = vert_color;
}

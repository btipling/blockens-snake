#version 410 core
flat in int instance_id;

uniform GridData {
    int num_columns;
    vec4 grid_color;
};

out vec4 color;

void main(void) {
    color = grid_color;
}
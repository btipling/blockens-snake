#version 410 core
flat in int instance_id;

uniform GridData {
    int num_columns;
    int num_rows;
    vec4 grid_colors[4];
};

out vec4 color;

void main(void) {
    color = grid_colors[0];
}

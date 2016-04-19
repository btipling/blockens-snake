#version 410 core

layout (location = 0) in vec4 v_position;
uniform bool is_block_vertex;

uniform GridData {
    vec4 grid_colors[4];
    int position_values[25 * 25]; // Max 50x50 grid.
};
int num_columns = 25;
int num_rows = 25;
flat out vec4 vert_color;
flat out int do_discard;

void main(void) {
    vert_color = grid_colors[0];
    do_discard = 0;
    if (is_block_vertex) {
        if (position_values[gl_InstanceID] > 0) {
            int color_index = position_values[gl_InstanceID];
            vert_color = grid_colors[color_index];
        } else {
            do_discard = 1;
        }
    }
    float x_offset = (gl_InstanceID % num_columns)*2 - (num_columns - 1);
    float y_offset = (gl_InstanceID / num_rows)*2 - (num_rows - 1);
    vec4 position = v_position + vec4(x_offset, y_offset * -1, 0.0f, 0.0f);
    position = position / vec4(num_columns, num_rows, 1.0f, 1.0f);
    gl_Position = position;
}

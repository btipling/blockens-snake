#version 410 core

layout(location = 0) in vec4 v_position;

uniform GridData {
    int num_columns;
    int num_rows;
    vec4 grid_colors[4];
};

flat out int instance_id;

void main(void) {
    float x_offset = (gl_InstanceID % num_columns)*2 - (num_columns - 1);
    float y_offset = (gl_InstanceID / num_columns)*2 - (num_columns - 1);
    vec4 position = v_position - vec4(x_offset, y_offset, 0.0f, 0.0f);
    position = position / vec4(num_columns, num_rows, 1.0f, 1.0f);
    gl_Position = position;
    instance_id = gl_InstanceID;
}
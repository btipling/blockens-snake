#version 410 core

layout(location = 0) in vec4 v_position;

uniform GridData {
    int num_columns;
    vec4 grid_color;
};

flat out int instance_id;

void main(void) {
    gl_Position = v_position * vec4((gl_InstanceID + num_columns) * .09, 0.9, 1.0, 1);
    instance_id = gl_InstanceID;
}
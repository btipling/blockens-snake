#version 410 core

layout (location = 0) in vec4 v_position;
uniform bool is_block_vertex;

const int num_columns = 25;
const int num_rows = 24;
uniform GridData {
    int num_blocken_blocks;
    vec4 grid_colors[4];
    ivec2 position_values[num_columns * num_rows];
};
flat out vec4 vert_color;
flat out int do_discard;

int CountDown = 0;
int BlockType = 1;

int NoBlock = 0;
int BlockenBlock = 1;

int block_transparency_start = 5;

int Alpha = 3;
float min_alpha = 0.30f;

void main(void) {

    // Start by assuming we are in the grid, set grid line color.

    vert_color = grid_colors[0];
    vert_color[Alpha] = 0.125; // Faint grid lines.
    do_discard = 0;

    if (is_block_vertex) {

        // We are drawing blocks, not the grid.

        vert_color = grid_colors[2];
        if (position_values[gl_InstanceID][BlockType] > 0) {

            // We are going to draw a block for this vertex.

            int color_index = position_values[gl_InstanceID][BlockType];
            vert_color = grid_colors[color_index];

            if (
                // ...we have enough blocks to start adding transparencies...
                num_blocken_blocks >= block_transparency_start &&
                // ...and if this is a user controlled BlockenBlock...
                position_values[gl_InstanceID][BlockType] == BlockenBlock &&
                // ...and it's nearing the tail end of the block set...
                position_values[gl_InstanceID][CountDown] <= block_transparency_start
            ) {

                // ...let's addd some transparency...
                float res = float(position_values[gl_InstanceID][CountDown])/float(block_transparency_start);

                // ...but only so much, we still need to see the blocks.
                if (res < min_alpha) {
                    res = min_alpha;
                }
                vert_color[Alpha] = res;
            }

        } else {

            // Empty block. Don't draw this vertex.

            do_discard = 1;

        }
    }

    // Figure out where in the grid we are.
//    const int total_rows = num_rows + 1;
    float x_offset = (gl_InstanceID % num_columns)*2 - (num_columns - 1);
    float y_offset = (gl_InstanceID / num_columns)*2 - (num_columns - 1);
    vec4 position = v_position + vec4(x_offset, y_offset * -1, 0.0f, 0.0f);
    position = position / vec4(num_columns, num_columns, 1.0f, 1.0f);

    gl_Position = position;
}

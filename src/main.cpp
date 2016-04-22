#include <blocken.h>


// FreeType.
FT_Library ft;
FT_Face face;
FT_GlyphSlot g;


// Declare colors.

GLfloat blue_green[4];
GLfloat yellow_green[4];
GLfloat light_brown[4];
GLfloat dark_brown[4];
GLfloat brown[4];
GLfloat white[4];
GLfloat black[4];

GLfloat grid_colors[4][4];
GLfloat line_color[4];
GLfloat bg_color[4];
GLfloat blocken_block_color[4];
GLfloat grow_block_color[4];
GLfloat speed_block_color[4];

// Declare buffers.

GLuint vao[3];
GLuint buffers[3];
GLuint ubo;
GLuint tex;
void *ubo_buffer;


GLuint uboIndex;
GLint uboSize;
enum { NumBlockenBlocks, GridColor, PositionValues, NumUniforms };

const char *names[NumUniforms] = {
        "num_blocken_blocks",
        "grid_colors",
        "position_values",
};

GLuint ubo_indexes[NumUniforms];
GLint ubo_sizes[NumUniforms];
GLint ubo_offset[NumUniforms];
GLint ubo_types[NumUniforms];
GLint ubo_strides[NumUniforms];

// Declare game variables.

const GLint num_columns = 25;
const GLint num_rows = 24;
const GLint max_positions = num_columns * num_rows;
GLint position_values[max_positions][2];

const int startCountDown = 1;
int currentCountDown = startCountDown;

enum { CountDown, BlockType };
enum { NoBlock, BlockenBlock, GrowBlock, SpeedBlock };

bool win_focused = true;
bool game_on = true;

enum { MoveLeft, MoveRight, MoveUp, MoveDown };
enum { R, G, B, A };
int current_movement = MoveLeft;

// In seconds.
double base_tick_interval = 0.20;
double speed_increase = -0.01;
double max_speed = 0.09;
double cur_tick_interval = base_tick_interval;
/*
 * The grid is made up of values 0 to n that starts at top left goes num_columns across and n @ num_columns + 1
 * starts next row. Determining n's column is n % num_columns and n's row is n / num_columns. Going from x,y (column,
 * row) to n would be y * num_columns + x = n;
 *
 * Remaining shader logic:
 *
 * If is_block_vertex == true, vertex shader checks position_values. If the value at index 0 is 0 we
 * discard the vertex. The frag shader will use this too, but to query color. If false frag shader picks
 * grid line color at 0.
 *
 * if position_value is anivec2
 *
 *   at index 0 is a count down of draw iterations before the block disappears
 *   at index 1 is the block type which corresponds to a value in grid_colors.
 *
 * A block is made up of two triangles facing each other to form a square.
 * Non blocks are lines that draw a square, a better grid would just to have lines that draw across the
 * entire drawing area horizontally and vertically.
 */


int main() {
    GLFWwindow* window;

    if (!glfwInit()) {
        out("Failed to init glfw.");
        exit(EXIT_FAILURE);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(640, 640, "blockens", NULL, NULL);
    if (!window) {
        std::cout << "Failed to init glfw window.\n";
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetWindowSizeCallback(window, window_resize_callback);
    std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    GLuint grid_program;
    GLuint text_program;

    init_inputs(window);
    init_colors();
    init_positions();
    init_freetype();

    grid_program = compile_grid_program();
    text_program = compile_text_program();

    init_buffers(grid_program);

    double lastTime = glfwGetTime();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        if (win_focused && game_on) {
            double now = glfwGetTime();
            double delta = now - lastTime;
            if (delta > cur_tick_interval) {
                do_movement();
                lastTime = now;
            }
            render_app(grid_program, window);
            render_app_text(window, text_program);
            glfwSwapBuffers(window);
        }
        glfwPollEvents();
    }

    glfwTerminate();
    glDeleteProgram(grid_program);
    return 0;
}


void init_buffers(GLuint grid_program) {
    glGenVertexArrays(3, vao);
    glGenBuffers(3, buffers);
    glGenBuffers(1, &ubo);

    uboIndex = glGetUniformBlockIndex(grid_program, "GridData");
    glGetActiveUniformBlockiv(grid_program, uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
    glUniformBlockBinding(grid_program, uboIndex, 0);

    glGetUniformIndices(grid_program, NumUniforms, names, ubo_indexes);
    glGetActiveUniformsiv(grid_program, NumUniforms, ubo_indexes, GL_UNIFORM_OFFSET, ubo_offset);
    glGetActiveUniformsiv(grid_program, NumUniforms, ubo_indexes, GL_UNIFORM_SIZE, ubo_sizes);
    glGetActiveUniformsiv(grid_program, NumUniforms, ubo_indexes, GL_UNIFORM_TYPE, ubo_types);
    glGetActiveUniformsiv(grid_program, NumUniforms, ubo_indexes, GL_UNIFORM_ARRAY_STRIDE, ubo_strides);

    set_color(grid_colors[NoBlock], line_color);
    set_color(grid_colors[BlockenBlock], blocken_block_color);
    set_color(grid_colors[GrowBlock], grow_block_color);
    set_color(grid_colors[SpeedBlock], speed_block_color);

    ubo_buffer = malloc(static_cast<size_t>(uboSize));
    if (ubo_buffer == NULL) {
        std::cout << "Unable to allocate uniform ubo_buffer.\n";
        exit(EXIT_FAILURE);
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


void init_freetype() {
    if(FT_Init_FreeType(&ft)) {
        out("Could not init freetype library.");
        exit(EXIT_FAILURE);
    }


    if(FT_New_Face(ft, "../resources/fonts/FreeSans.ttf", 0, &face)) {
        out("Could not open font");
        exit(EXIT_FAILURE);
    }
    FT_Set_Pixel_Sizes(face, 0, 48);
    g = face->glyph;
}


void init_inputs(GLFWwindow *window) {
    glfwSetKeyCallback(window, key_callback);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_W:
        case GLFW_KEY_UP:
        case GLFW_KEY_S:
        case GLFW_KEY_DOWN:
        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:
        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT:
            if (action == GLFW_REPEAT) {
                cur_tick_interval = .025;
            } else if (action == GLFW_RELEASE){
                cur_tick_interval = base_tick_interval;
            }
            switch (key) {
                case GLFW_KEY_W:
                case GLFW_KEY_UP:
                    // For each movement we check that if we're longer than 1 we can't move in opposite direction.
                    if (currentCountDown > 1 && current_movement == MoveDown) {
                        out("Can't move up when going down!");
                        return;
                    }
                    current_movement = MoveUp;
                    break;
                case GLFW_KEY_S:
                case GLFW_KEY_DOWN:
                    if (currentCountDown > 1 && current_movement == MoveUp) {
                        out("Can't move down when going up!");
                        return;
                    }
                    current_movement = MoveDown;
                    break;
                case GLFW_KEY_A:
                case GLFW_KEY_LEFT:
                    if (currentCountDown > 1 && current_movement == MoveRight) {
                        out("Can't move left when going right!");
                        return;
                    }
                    current_movement = MoveLeft;
                    break;
                case GLFW_KEY_D:
                case GLFW_KEY_RIGHT:
                default: // Otherwise complaints about no default case or unreachable code.
                    if (currentCountDown > 1 && current_movement == MoveLeft) {
                        out("Can't move right when going left!");
                        return;
                    }
                    current_movement = MoveRight;
                    break;
            }
            break;
        default:
            // Ignore;
            break;
    }

}


void init_colors() {
    rgba_to_color(53, 208, 173, 1, blue_green);
    rgba_to_color(220, 240, 143, 1, yellow_green);
    rgba_to_color(227, 203, 156, 1, light_brown);
    rgba_to_color(210, 158, 79, 1, brown);
    rgba_to_color(191, 89, 34, 1, dark_brown);
    rgba_to_color(255, 255, 255, 1, white);
    rgba_to_color(51, 51, 51, 1, black);

    set_color(bg_color, white);
    set_color(line_color, blue_green);
    set_color(blocken_block_color, light_brown);
    set_color(grow_block_color, dark_brown);
    set_color(speed_block_color, yellow_green);
}


void init_positions() {
    for (int i = 0; i < max_positions; i++) {
        position_values[i][BlockType] = NoBlock;
    }
    srand(time(NULL));
    int n = rand_n();
    position_values[n][CountDown] = currentCountDown;
    position_values[n][BlockType] = BlockenBlock;
    n = rand_n();
    position_values[n][BlockType] = GrowBlock;
}


void n_to_xy(int n, int *x, int *y) {
    *x = n % num_columns;
    *y = n / num_columns;
}


int xy_to_n(int x, int y) {
    return y * num_columns + x;
}


void do_movement() {
    int move_n = -1;
    for (int i = 0; i < max_positions; i++) {
        if (position_values[i][CountDown] == currentCountDown) {
            int x;
            int y;
            n_to_xy(i, &x, &y);

            switch (current_movement) {
                case MoveLeft:
                    x--;
                    break;
                case MoveRight:
                    x++;
                    break;
                case MoveDown:
                    y++;
                    break;
                case MoveUp:
                    y--;
                    break;
                default:
                    out("Movement problem detected.");
                    break;
            }
            if (x >= num_columns) {
                x = 0;
                y++;
            }
            if (x < 0) {
                x = num_columns - 1;
                y--;
            }
            if (y < 0) {
                y = num_rows - 1;
            }
            if (y >= num_rows) {
                y = 0;
            }
            move_n = xy_to_n(x, y);
        }
        if (position_values[i][CountDown] > 0) {
            position_values[i][CountDown]--;
            if (position_values[i][CountDown] == 0) {
                position_values[i][BlockType] = NoBlock;
            }
        }
    }
    if (position_values[move_n][BlockType] == BlockenBlock) {
        out("Game over, you hit a blocken block");
        game_on = false;
        return;
    } else if (position_values[move_n][BlockType] == GrowBlock || position_values[move_n][BlockType] == SpeedBlock) {
        if (position_values[move_n][BlockType] == GrowBlock) {
            currentCountDown++;
        } else {
            base_tick_interval += speed_increase;
        }
        if (base_tick_interval > max_speed && rand() % 4 == 3)  {
            position_values[rand_n()][BlockType] = SpeedBlock;
        } else {
            position_values[rand_n()][BlockType] = GrowBlock;
        }
    }
    position_values[move_n][BlockType] = BlockenBlock;
    position_values[move_n][CountDown] = currentCountDown;
    cur_tick_interval = base_tick_interval;
}


int rand_n() {
    int n;
    int max_tries = max_positions * max_positions;
    int tries = 0;
    do {
        n = rand() % max_positions - 1;
        tries++;
    } while (position_values[n][BlockType] != NoBlock && tries < max_tries);
    if (tries == max_tries) {
        out("Game over, we couldn't find a place to put a grow or speed block");
        game_on = false;
    }
    return n;
}


void window_focus_callback(GLFWwindow* window, int focused) {
    if (focused) {
        win_focused = true;
    } else {
        win_focused = false;
    }
}


void window_resize_callback (GLFWwindow *window, int width, int height) {
    int max_dim = width > height ? width : height;
    glfwSetWindowSize(window, max_dim, max_dim);
}



void set_color(GLfloat to_color[4], GLfloat fro_color[4]) {
    memcpy(to_color, fro_color, sizeof(GLfloat) * 4);
}


void setup_grid_vertices() {
    const int n = 8;
    enum Attrib_IDS { vPosition = 0 };
    GLfloat vertices[n][4] = {

            { -1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },

            { -1.0f, -1.0f, 1.0f, 1.0f },
            { 1.0f, -1.0f, 1.0f, 1.0f },

            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, -1.0f, 1.0f, 1.0f },

            { -1.0f, 1.0f, 1.0f, 1.0f },
            { -1.0f, -1.0f, 1.0f, 1.0f },
    };

    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
}


void setup_block_vertices() {
    const int n = 6;
    enum Attrib_IDS { vPosition = 0 };
    GLfloat vertices[n][4] = {
            { -1.0f, -1.0f, 1.0f, 1.0f },
            { -1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },

            { -1.0f, -1.0f, 1.0f, 1.0f },
            { 1.0f, -1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
    };

    glBindVertexArray(vao[1]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
}

void render_app_text(GLFWwindow *window, GLuint text_program) {
    enum Attrib_IDS { texcoord = 0 };

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float sx = 2.0f/width;
    float sy = 2.0f/height;

    glUseProgram(text_program);
//    GLfloat vertices[6][4] = {
//            { -1.0f, -1.0f, 1.0f, 1.0f },
//            { -1.0f, 1.0f, 1.0f, 1.0f },
//            { 1.0f, 1.0f, 1.0f, 1.0f },
//
//            { -1.0f, -1.0f, 1.0f, 1.0f },
//            { 1.0f, -1.0f, 1.0f, 1.0f },
//            { 1.0f, 1.0f, 1.0f, 1.0f },
//    };

    glBindVertexArray(vao[2]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    glVertexAttribPointer(texcoord, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(texcoord);


    render_text("X", 0.0f,   0.1f,  sx, sy);
}

void render_text(const char *text, float x, float y, float sx, float sy) {
    const char *p;

    for(p = text; *p; p++) {

        if(FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
            continue;
        }

//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            g->bitmap.width,
//            g->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            g->bitmap.buffer
//        );

        float x2 = x + g->bitmap_left * sx;
        float y2 = -y - g->bitmap_top * sy;
        float w = g->bitmap.width * sx;
        float h = g->bitmap.rows * sy;

        GLfloat box[4][4] = {
                {x2,     -y2    , 0, 0},
                {x2 + w, -y2    , 1, 0},
                {x2,     -y2 - h, 0, 1},
                {x2 + w, -y2 - h, 1, 1},
        };
        std::cout << "w: " << w << " h: " << h << " x2: " << x2 << " y2: " << -y2 << " x: " << x << " y: " << y << "\n";
        glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
        glPointSize(40.0f);
        glDrawArrays(GL_POINTS, 0, 4);

        x += (g->advance.x/64) * sx;
        y += (g->advance.y/64) * sy;
    }
}

void setup_uniform() {

    *(int *)ADDRESS(ubo_buffer, ubo_offset[NumBlockenBlocks]) = currentCountDown;

    GLint color_offset = ubo_offset[GridColor];
    for (int colors_i = 0; colors_i < 4; colors_i++) {
        ((float *)ADDRESS(ubo_buffer, color_offset))[R] = grid_colors[colors_i][R];
        ((float *)ADDRESS(ubo_buffer, color_offset))[G] = grid_colors[colors_i][G];
        ((float *)ADDRESS(ubo_buffer, color_offset))[B] = grid_colors[colors_i][B];
        ((float *)ADDRESS(ubo_buffer, color_offset))[A] = grid_colors[colors_i][A];
        color_offset += ubo_strides[GridColor];
    }

    GLint pos_offset = ubo_offset[PositionValues];
    for (int pos_i = 0; pos_i < max_positions; pos_i++) {
        ((GLint *)ADDRESS(ubo_buffer, pos_offset))[CountDown] = position_values[pos_i][CountDown];
        ((GLint *)ADDRESS(ubo_buffer, pos_offset))[BlockType] = position_values[pos_i][BlockType];
        pos_offset += ubo_strides[PositionValues];
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, ubo_buffer, GL_STATIC_DRAW);
}


void render_app(GLuint grid_program, GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 1, 0, 1);
    glClearBufferfv(GL_COLOR, 0, bg_color);

    GLint is_block_index = glGetUniformLocation(grid_program, "is_block_vertex");
    const GLuint numVertices = 8;
    setup_uniform();
    glUseProgram(grid_program);

    glUniform1i(is_block_index, GL_TRUE);
    setup_block_vertices();
    glDrawArraysInstanced(GL_TRIANGLES, 0, numVertices, num_columns * num_rows);

    glUniform1i(is_block_index, GL_FALSE);
    setup_grid_vertices();
    glDrawArraysInstanced(GL_LINES, 0, numVertices, num_columns * num_rows);
}

GLuint compile_shader(std::string filename, GLenum type) {
    GLuint shader;
    std::string shader_source_string = get_shader(filename);
    const GLchar * shader_source = shader_source_string.c_str();  shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    GLint success;
    GLint log_size;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);

    if (success != GL_TRUE) {
        std::cout << filename << " shader didn't compile.  " << log_size << " \n";
        char* log = new char[log_size];
        glGetShaderInfoLog(shader, log_size, NULL, log);
        std::cout << filename << " shader error log: \n" << log << " \n";
        exit(EXIT_FAILURE);
    } else {
        std::cout << filename <<  " shader did compile. " << log_size << " \n";
    }

    return shader;
};

GLuint compile_text_program() {
    return compile_shaders("text.vert", "text.frag");
}

GLuint compile_grid_program() {
    return compile_shaders("grid.vert", "grid.frag");
}

GLuint compile_shaders(std::string vert_shader_name, std::string frag_shader_name) {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    vertex_shader = compile_shader(vert_shader_name, GL_VERTEX_SHADER);
    fragment_shader = compile_shader(frag_shader_name, GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}


std::string get_working_path() {
    char temp[MAXPATHLEN];
    return ( getcwd(temp, MAXPATHLEN) ? std::string( temp ) : std::string("") );
}


std::string get_shader(std::string path) {
    std::cout << "Current cwd is " << get_working_path() << " !\n";
    std::string fullpath = "../resources/shaders/" + path;
    std::cout << fullpath << "\n";
    std::ifstream vertexShaderFile(fullpath);
    std::ostringstream vertexBuffer;
    vertexBuffer << vertexShaderFile.rdbuf();
    return vertexBuffer.str();
}


void rgba_to_color(int r, int g, int b, int a, GLfloat color[4]) {
    color[0] = r/255.0f;
    color[1] = g/255.0f;
    color[2] = b/255.0f;
    color[3] = a;
}

void out(const char* msg) {
    std::cout << msg << "\n";
}

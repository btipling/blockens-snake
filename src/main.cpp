#include <blocken.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/param.h>

GLfloat blue_green[4];
GLfloat yellow_green[4];
GLfloat light_brown[4];
GLfloat dark_brown[4];
GLfloat brown[4];
GLfloat white[4];
GLfloat black[4];

GLfloat line_color[4];
GLfloat bg_color[4];
GLfloat block_color[4];
GLfloat grow_block_color[4];
GLfloat speed_block_color[4];

const GLint max_positions = 25 * 25;
GLint position_values[max_positions];

bool win_focused = true;

/*
 * The grid is made up of values 0 to n that starts at top left goes num_columns across and n @ num_columns + 1
 * starts next row. Determining n's column is n % num_columns and n's row is n / num_rows. Going from x,y (column,
 * row) to n would be y * num_columns + x = n;
 *
 * Remaining shader logic:
 *
 * If is_block_vertex == true, vertex shader checks position_values. If the balue for gl_InstanceID is -1 we
 * discard the vertex. The frag shader will use this too, but to query color. If false frag shader picks
 * grid line color at 0.
 *
 * inside position_values are values -1 to sizeof(grid_colors)
 *
 *   i index of what color to use from grid_color.
 *
 * A block is made up of two triangles facing each other to form a square.
 * Non blocks are lines that draw a square, a better grid would just to have lines that draw across the
 * entire drawing area horizontally and vertically.
 */

int main() {
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cout << "Failed to init glfw.\n";
        exit(EXIT_FAILURE);
    }

    glfwDefaultWindowHints();
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

    std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    GLuint rendering_program;

    init_colors();
    init_positions();
    rendering_program = compile_shaders();

    while (!glfwWindowShouldClose(window)) {
        if (win_focused) {
            render_app(rendering_program, window);
        }
        glfwPollEvents();
    }

    glfwTerminate();
    glDeleteProgram(rendering_program);
    return 0;
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
    set_color(block_color, light_brown);
    set_color(grow_block_color, dark_brown);
    set_color(speed_block_color, yellow_green);
}

void init_positions() {
    for (int i = 0; i < max_positions; i++) {
        position_values[i] = -1;
    }
    position_values[0] = 1;
}


void window_focus_callback(GLFWwindow* window, int focused) {
    if (focused) {
        win_focused = true;
    } else {
        win_focused = false;
    }
}


void set_color(GLfloat to_color[4], GLfloat fro_color[4]) {
    memcpy(to_color, fro_color, sizeof(GLfloat) * 4);
}


void setup_grid_vertices() {
    GLuint vertex_array_object[1];
    GLuint buffers[1];
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

    glGenVertexArrays(1, vertex_array_object);
    glBindVertexArray(vertex_array_object[0]);
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
}


void setup_block_vertices() {
    GLuint vertex_array_object[1];
    GLuint buffers[1];
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

    glGenVertexArrays(1, vertex_array_object);
    glBindVertexArray(vertex_array_object[0]);
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
}

void setup_uniform(GLuint rendering_program, GLint num_columns, GLint num_rows) {

    GLuint uboIndex;
    GLint uboSize;
    GLuint ubo;
    enum { GridColor, PositionValues, NumUniforms };

    const char *names[NumUniforms] = {
        "grid_colors",
        "position_values",
    };

    GLuint indexes[NumUniforms];
    GLint size[NumUniforms];
    GLint offset[NumUniforms];
    GLint type[NumUniforms];
    GLint strides[NumUniforms];

    uboIndex = glGetUniformBlockIndex(rendering_program, "GridData");
    glGetActiveUniformBlockiv(rendering_program, uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
    glUniformBlockBinding(rendering_program, uboIndex, 0);

    glGetUniformIndices(rendering_program, NumUniforms, names, indexes);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_OFFSET, offset);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_SIZE, size);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_TYPE, type);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_ARRAY_STRIDE, strides);

    GLfloat grid_colors[4][4];
    set_color(grid_colors[0], line_color);
    set_color(grid_colors[1], block_color);
    set_color(grid_colors[2], grow_block_color);
    set_color(grid_colors[3], speed_block_color);

    void *buffer;

    buffer = malloc(static_cast<size_t>(uboSize));
    if (buffer == NULL) {
        std::cout << "Unable to allocate unform buffer.\n";
        exit(EXIT_FAILURE);
    }

    GLint color_offset = offset[GridColor];
    for (int colors_i = 0; colors_i < 4; colors_i++) {
        ((float *)ADDRESS(buffer, color_offset))[0] = grid_colors[colors_i][0];
        ((float *)ADDRESS(buffer, color_offset))[1] = grid_colors[colors_i][1];
        ((float *)ADDRESS(buffer, color_offset))[2] = grid_colors[colors_i][2];
        ((float *)ADDRESS(buffer, color_offset))[3] = grid_colors[colors_i][3];
        color_offset += strides[GridColor];
    }

    GLint pos_offset = offset[PositionValues];
    for (int pos_i = 0; pos_i < max_positions; pos_i++) {
        *(GLint *)ADDRESS(buffer, pos_offset) = position_values[pos_i];
        pos_offset += strides[PositionValues];
    }

    glGenBuffers(1, &ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, buffer, GL_STATIC_DRAW);
}

void render_app(GLuint rendering_program, GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 1, 0, 1);
    glClearBufferfv(GL_COLOR, 0, bg_color);

    GLint is_block_index = glGetUniformLocation(rendering_program, "is_block_vertex");
    const GLuint numVertices = 8;
    GLint num_columns = 25;
    GLint num_rows = 25;
    setup_uniform(rendering_program, num_columns, num_rows);
    glUseProgram(rendering_program);

    glUniform1i(is_block_index, GL_TRUE);
    setup_block_vertices();
    glDrawArraysInstanced(GL_TRIANGLES, 0, numVertices, num_columns * num_rows);

    glUniform1i(is_block_index, GL_FALSE);
    setup_grid_vertices();
    glDrawArraysInstanced(GL_LINES, 0, numVertices, num_columns * num_rows);

    glfwSwapBuffers(window);
}


GLuint compile_shaders(void) {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    std::string vertex_shader_source_string = get_shader("grid.vert");
    const GLchar * vertex_shader_source = vertex_shader_source_string.c_str();

    std::string fragment_shader_source_string = get_shader("grid.frag");
    const GLchar * fragment_shader_source = fragment_shader_source_string.c_str();

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLint success;
    GLint log_size;

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_size);

    if (success != GL_TRUE) {
        std::cout << "Vertex shader didn't compile homes.  " << log_size << " \n";
        char* log = new char[log_size];
        glGetShaderInfoLog(vertex_shader, log_size, NULL, log);
        std::cout << "Vertex shader error log: \n" << log << " \n";
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Vertex shader did compile homes. " << log_size << " \n";
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_size);

    if (success != GL_TRUE) {
        std::cout << "Frag shader didn't compile homes. " << log_size << " \n";
        char* log = new char[log_size];
        glGetShaderInfoLog(fragment_shader, log_size, NULL, log);
        std::cout << "Frag shader error log: \n" << log << " \n";
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Frag shader did compile homes. " << log_size << " \n";
    }

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
    color[3] = a/255.0f;
}

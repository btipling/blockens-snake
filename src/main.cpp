
#include <blocken.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/param.h>
#include <math.h>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    GLuint rendering_program;

    rendering_program = compile_shaders();

    while (!glfwWindowShouldClose(window)) {

        app_render(rendering_program, window);
    }

    glfwTerminate();
    glDeleteProgram(rendering_program);
    return 0;
}

void setup_vertices(GLuint numVertices) {
    GLuint vertex_array_object[1];
    GLuint buffers[1];
    const int n = 8;
    enum Attrib_IDS { vPosition = 0 };
    GLfloat vertices[n][4] = {

            { -1.0f, 1.0f, 0.50f, 1.0f },
            { 1.0f, 1.0f, 0.50f, 1.0f },

            { -1.0f, -1.0f, 0.50f, 1.0f },
            { 1.0f, -1.0f, 0.50f, 1.0f },

            { 1.0f, 1.0f, 0.50f, 1.0f },
            { 1.0f, -1.0f, 0.50f, 1.0f },


            { -1.0f, 1.0f, 0.50f, 1.0f },
            { -1.0f, -1.0f, 0.50f, 1.0f },
    };

    glGenVertexArrays(1, vertex_array_object);
    glBindVertexArray(vertex_array_object[0]);
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
}

int column_size = 0;
int direction = 1;

void setup_uniform(GLuint rendering_program) {

    GLuint uboIndex;
    GLint uboSize;
    GLuint ubo;
    enum { NumColumns, GridColor, NumUniforms };

    const char *names[NumUniforms] = {
            "num_columns",
            "grid_color",
    };

    GLuint indexes[NumUniforms];
    GLint size[NumUniforms];
    GLint offset[NumUniforms];
    GLint type[NumUniforms];

    uboIndex = glGetUniformBlockIndex(rendering_program, "GridData");
    glGetActiveUniformBlockiv(rendering_program, uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
    glUniformBlockBinding(rendering_program, uboIndex, 0);

    glGetUniformIndices(rendering_program, NumUniforms, names, indexes);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_OFFSET, offset);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_SIZE, size);
    glGetActiveUniformsiv(rendering_program, NumUniforms, indexes, GL_UNIFORM_TYPE, type);


    if (column_size >= 10) {
        direction = -1;
    } else if (column_size == 0) {
        direction = 1;
    }

    if (direction == 1) {
        column_size++;
    } else {
        column_size--;
    }
    GLint num_columns = column_size;
    GLfloat grid_color[] = { 1.0f, 0.0f, 1.0f, 1.0f };

    void *buffer;

    buffer = malloc(static_cast<size_t>(uboSize));
    if (buffer == NULL) {
        std::cout << "Unable to allocate unform buffer.\n";
        exit(EXIT_FAILURE);
    }

    memcpy(ADDRESS(buffer, offset[NumColumns]), &num_columns, sizeof(GLint));
    memcpy(ADDRESS(buffer, offset[GridColor]), &grid_color, sizeof(GLfloat) * 4);

    glGenBuffers(1, &ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, buffer, GL_STATIC_DRAW);
}

void app_render(GLuint rendering_program, GLFWwindow* window) {
    const GLuint numVertices = 8;
    setup_vertices(numVertices);
    setup_uniform(rendering_program);
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 1, 0, 1);
    static const GLfloat bg[] = { 0.0, 0.0, 0.0, 1.0 };
    glClearBufferfv(GL_COLOR, 0, bg);

    glUseProgram(rendering_program);
    glDrawArraysInstanced(GL_LINES, 0, numVertices, 3);


    glfwSwapBuffers(window);
    glfwPollEvents();
}

GLuint compile_shaders(void) {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    std::string vertex_shader_source_string = get_shader("dot.vert");
    const GLchar * vertex_shader_source = vertex_shader_source_string.c_str();

    std::string fragment_shader_source_string = get_shader("dot.frag");
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

std::string get_working_path()  {
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
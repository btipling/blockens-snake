
#include <blocken.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/param.h>

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
    GLuint vertex_array_object[1];
    GLuint buffers[1];
    enum Attrib_IDS { vPosition = 0 };
    const GLuint numVertices = 8;
    GLfloat vertices[numVertices][4] = {

        { -0.90f, 0.90f, 0.50f, 1.0f },
        { 0.90f, 0.90f, 0.50f, 1.0f },

        { -0.90f, -0.90f, 0.50f, 1.0f },
        { 0.90f, -0.90f, 0.50f, 1.0f },

        { 0.90f, 0.90f, 0.50f, 1.0f },
        { 0.90f, -0.90f, 0.50f, 1.0f },


        { -0.90f, 0.90f, 0.50f, 1.0f },
        { -0.90f, -0.90f, 0.50f, 1.0f },
    };

    rendering_program = compile_shaders();
    glGenVertexArrays(1, vertex_array_object);
    glBindVertexArray(vertex_array_object[0]);
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vPosition);
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 1, 0, 1);
        static const GLfloat bg[] = { 0.0, 0.0, 0.0, 1.0 };
        glClearBufferfv(GL_COLOR, 0, bg);

        glUseProgram(rendering_program);
//        glPointSize(40.0f);

        glDrawArrays(GL_LINES, 0, numVertices);

        glfwSwapBuffers(window);
        glfwPollEvents();
        sleep(1);
    }

    glfwTerminate();
    glDeleteProgram(rendering_program);
    return 0;
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
    } else {
        std::cout << "Vertex shader did compile homes. " << log_size << " \n";
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_size);

    if (success != GL_TRUE) {
        std::cout << "Frag shader didn't compile homes. " << log_size << " \n";
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
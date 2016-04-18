
#define GLFW_INCLUDE_GLCOREARB = 1

#include <GLFW/glfw3.h>
#include <iostream>
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define ADDRESS(p,o) ((void *)(((char *)p)+(o)))

std::string get_shader(std::string path);
GLuint compile_shaders(void);
void rgba_to_color(int r, int g, int b, int a, GLfloat color[4]);
void render_app(GLuint rendering_program, GLFWwindow *window);
void setup_uniform(GLuint rendering_program, GLuint num_columns, GLint num_rows);
void setup_vertices();
void init_colors();
void set_color(GLfloat[4], GLfloat[4]);
void window_focus_callback(GLFWwindow* window, int focused);


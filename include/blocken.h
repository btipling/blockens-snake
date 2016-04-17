
#define GLFW_INCLUDE_GLCOREARB = 1

#include <GLFW/glfw3.h>
#include <iostream>
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define ADDRESS(p,o) ((void *)(((char *)p)+(o)))

std::string get_shader(std::string path);
GLuint compile_shaders(void);
void app_render(GLuint rendering_program, GLFWwindow *window);
void setup_uniform(GLuint);


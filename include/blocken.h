
#define GLFW_INCLUDE_GLCOREARB = 1

#include <GLFW/glfw3.h>
#include <iostream>

std::string get_shader(std::string path);
GLuint compile_shaders(void);

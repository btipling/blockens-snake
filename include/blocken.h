
#define GLFW_INCLUDE_GLCOREARB = 1

#include <GLFW/glfw3.h>
#include <iostream>
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

std::string get_shader(std::string path);
GLuint compile_shaders(void);

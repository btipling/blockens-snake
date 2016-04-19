
#define GLFW_INCLUDE_GLCOREARB = 1

#include <GLFW/glfw3.h>
#include <iostream>
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define ADDRESS(p,o) ((void *)(((char *)p)+(o)))

std::string get_shader(std::string path);
GLuint compile_shaders();
void rgba_to_color(int, int, int, int, GLfloat[4]);
void render_app(GLuint, GLFWwindow*);
void setup_uniform(GLuint);
void setup_grid_vertices();
void setup_block_vertices();
void init_colors();
void do_movement();
void init_positions();
void set_color(GLfloat[4], GLfloat[4]);
void window_focus_callback(GLFWwindow*, int);
void n_to_xy(int, int*, int*);
int xy_to_n(int, int);


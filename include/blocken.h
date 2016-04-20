
#define GLFW_INCLUDE_GLCOREARB = 1

#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/param.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>

// Helper macros.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define ADDRESS(p,o) ((void *)(((char *)p)+(o)))

// Init game.
void init_colors();
void init_inputs(GLFWwindow*);
void init_positions();
void init_buffers(GLuint);

// Setup writing data to GPU for each draw.
void setup_uniform();
void setup_grid_vertices();
void setup_block_vertices();

// Rendering.
std::string get_shader(std::string path);
GLuint compile_shaders();
void render_app(GLuint, GLFWwindow*);

// Movement.
void n_to_xy(int, int*, int*);
int xy_to_n(int, int);
int rand_n();
void do_movement();

// Colors.
void set_color(GLfloat[4], GLfloat[4]);
void rgba_to_color(int, int, int, int, GLfloat[4]);

// GLFW callbacks.
void window_focus_callback(GLFWwindow*, int);
void window_resize_callback(GLFWwindow *, int, int);
void key_callback(GLFWwindow*, int, int, int, int);

// Debug.
void out(const char *msg);


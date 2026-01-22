#include <iostream>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


// CPU buffer
struct Buffer{
    size_t width, height;
    uint32_t* data;
};

// Sprite
struct Sprite{
    size_t width, height;
    uint8_t* data;
};

/* =====================
     SHADERS
   ===================== */
const char* vertex_shader_src = 
"#version 330 core\n"
"noperspective out vec2 TexCoord;\n"
"void main() {\n"
"    TexCoord.x = (gl_VertexID == 2) ? 2.0 : 0.0;\n"
"    TexCoord.y = (gl_VertexID == 1) ? 2.0 : 0.0;\n"
"    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
"}\n";

const char* fragment_shader_src = 
"#version 330 core\n"
"uniform sampler2D buffer;\n"
"noperspecitve in vec2 TexCoord;\n"
"out vec3 outColor;\n"
"void main() {\n"
"    outColor = texture(buffer, TexCoord).rgb;\n"
"}\n";

void error_callback(int, const char*);
void validate_shader(GLuint, const char*);
bool validate_program(GLuint);
uint32_t rgb_to_uint32(uint8_t, uint8_t, uint8_t);
void buffer_clear(Buffer*, uint32_t);
void buffer_sprite_draw(Buffer*, const Sprite&, size_t, size_t, uint32_t);


int main(){
    // Setter error callback
    glfwSetErrorCallback(error_callback);

    // Initialiserer GLFW
    if (!glfwInit()){
        std::cerr << "Failed to initialize GLFW\n"; 
        return -1;
    }
    
    // Spør om OPENGL 3.3 Core profil
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Lager Vinduet
    GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    if (!window){
        std::cerr << "Failed to create window...\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    // Putter vinduet til OPENGL konteksten
    glfwMakeContextCurrent(window);

    // Initialiserer GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Lager CPU bufferen
    const size_t buffer_width  = 320;
    const size_t buffer_height = 240; 
    
    Buffer buffer;
    buffer.width  = buffer_width;
    buffer.height = buffer_height;
    buffer.data   = new uint32_t[buffer_width * buffer_height];

    uint32_t clear_color = rgb_to_uint32(0, 128, 0);

    // Alien Sprite
    Sprite alien;
    alien.width = 11;
    alien.height = 8;
    alien.data = new uint8_t[11 * 8]{
        0,0,1,0,0,0,0,0,1,0,0,
        0,0,0,1,0,0,0,1,0,0,0,
        0,0,1,1,1,1,1,1,1,0,0,
        0,1,1,0,1,1,1,0,1,1,0,
        1,1,1,1,1,1,1,1,1,1,1,
        1,0,1,1,1,1,1,1,1,0,1,
        1,0,1,0,0,0,0,0,1,0,1,
        0,0,0,1,1,0,1,1,0,0,0};
    

    //OpenGl objekter
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint program = glCreateProgram();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader_src, nullptr);
    glCompileShader(vs);
    validate_shader(vs, "vertex");
    glAttachShader(program, vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader_src, nullptr);
    glCompileShader(fs);
    validate_shader(fs, "fragment");
    glAttachShader(program, fs);

    glLinkProgram(program);
    validate_program(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(program);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB8,
        buffer.width, buffer.height, 0,
        GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
        buffer.data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(glGetUniformLocation(program, "buffer"), 0);

    glDisable(GL_DEPTH_TEST);


    // Spill løkken 
    while (!glfwWindowShouldClose(window)){
        buffer_clear(&buffer, clear_color);

        buffer_sprite_draw(
            &buffer,
            alien,
            112, 128,
            rgb_to_uint32(128, 0, 0)
        );

        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            buffer.width, buffer.height,
            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
            buffer.data
        );

        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete[] buffer.data;
    delete[] alien.data;

    std::cout << "Exiting...\n";
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

void validate_shader(GLuint shader, const char* name = ""){
    char buffer[512];
    GLsizei length = 0;
    glGetShaderInfoLog(shader, 512, &length, buffer);
    
    if (length > 0)
        std::cout << "Shader Error: (" << name << "): " << buffer << std::endl;
}

bool validate_program(GLuint program){
    char buffer[512];
    GLsizei length = 0;
    glGetProgramInfoLog(program, 512, &length, buffer);

    if (length > 0){
        std::cout << "Program link error: " << buffer << std::endl;
        return 0;
    }
    
    return 1;
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b){
    return (r << 24) | (g << 16) | (b << 8) | 255;
}

void buffer_clear(Buffer* buffer, uint32_t color){
    for (size_t i{0}; i < buffer->width * buffer->height; i++){
        buffer->data[i] = color;
    }
}

void buffer_sprite_draw(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t color){
    for (size_t xi{0}; xi < sprite.width; ++xi){
        for (size_t yi{0}; yi < sprite.height; ++yi){
            size_t sx = x + xi;
            size_t sy = sprite.height - 1 + y - yi;

            if (sprite.data[yi * sprite.width + xi] &&
                sx < buffer->width &&
                sy < buffer->height)
            {
                buffer->data[sy * buffer->width + sx] = color;
            }
        }
    }
}

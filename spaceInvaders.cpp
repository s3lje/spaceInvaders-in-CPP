#include <iostream>
#include <cstdint>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


// CPU buffer
struct Buffer {
    size_t width, height;
    uint32_t* data;
};

// Sprite
struct Sprite {
    size_t width, height;
    uint8_t* data;
};

struct SpriteAnimation {
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite** frames;
};

struct Alien {
    size_t x, y;
    uint8_t type;
};

struct Player {
    size_t x, y;
    size_t life;
};

struct Game {
    size_t width, height; 
    size_t num_aliens;
    Alien* aliens;
    Player player;
};

bool game_running = 1; 
int move_dir      = 0;

/* =====================
     SHADERS
   ===================== */
const char* vertex_shader_src =
    "\n"
    "#version 330\n"
    "\n"
    "noperspective out vec2 TexCoord;\n"
    "\n"
    "void main(void){\n"
    "\n"
    "    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
    "    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
    "    \n"
    "    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
    "}\n";

const char* fragment_shader_src =
    "\n"
    "#version 330\n"
    "\n"
    "uniform sampler2D buffer;\n"
    "noperspective in vec2 TexCoord;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void){\n"
    "    outColor = texture(buffer, TexCoord).rgb;\n"
    "}\n";

void error_callback(int, const char*);
void key_callback(GLFWwindow*, int, int, int, int);
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
    glfwSetKeyCallback(window, key_callback);

    // Putter vinduet til OPENGL konteksten
    glfwMakeContextCurrent(window);
    
    glfwSwapInterval(1);

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
    const size_t buffer_width  = 224;
    const size_t buffer_height = 256; 
    
    uint32_t clear_color = rgb_to_uint32(0, 0, 0);
    Buffer buffer;
    buffer.width  = buffer_width;
    buffer.height = buffer_height;
    buffer.data   = new uint32_t[buffer_width * buffer_height];
    buffer_clear(&buffer, clear_color);

    // Alien Sprite
    Sprite alien_sprite;
    alien_sprite.width = 11;
    alien_sprite.height = 8;
    alien_sprite.data = new uint8_t[11 * 8]{
        0,0,1,0,0,0,0,0,1,0,0,
        0,0,0,1,0,0,0,1,0,0,0,
        0,0,1,1,1,1,1,1,1,0,0,
        0,1,1,0,1,1,1,0,1,1,0,
        1,1,1,1,1,1,1,1,1,1,1,
        1,0,1,1,1,1,1,1,1,0,1,
        1,0,1,0,0,0,0,0,1,0,1,
        0,0,0,1,1,0,1,1,0,0,0};

    Sprite alien_sprite1;
    alien_sprite1.width = 11;
    alien_sprite1.height = 7;
    alien_sprite1.data = new uint8_t[11 * 8]{
    0,0,1,0,0,0,0,0,1,0,0,
    1,0,0,1,0,0,0,1,0,0,1, 
    1,0,1,1,1,1,1,1,1,0,1, 
    1,1,1,0,1,1,1,0,1,1,1, 
    1,1,1,1,1,1,1,1,1,1,1, 
    0,1,1,1,1,1,1,1,1,1,0, 
    0,0,1,0,0,0,0,0,1,0,0, 
    0,1,0,0,0,0,0,0,0,1,0};
    
    SpriteAnimation* alien_animation = new SpriteAnimation;
    alien_animation->loop = true;
    alien_animation->num_frames = 2;
    alien_animation->frame_duration = 10;
    alien_animation->time = 0;

    alien_animation->frames = new Sprite*[2];
    alien_animation->frames[0] = &alien_sprite;
    alien_animation->frames[1] = &alien_sprite1;


    // Player Sprite
    Sprite player_sprite;
    player_sprite.width = 11;
    player_sprite.height = 7;
    player_sprite.data = new uint8_t[11 * 7]{
        0,0,0,0,0,1,0,0,0,0,0,
        0,0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,1,1,1,0,0,0,0,
        0,1,1,1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1};


    // Initialiser Game strukten
    Game game;
    game.width = buffer_width;
    game.height = buffer_height;
    game.num_aliens = 55;
    game.aliens = new Alien[game.num_aliens];

    game.player.x = 112 - 5;
    game.player.y = 32;
    game.player.life = 3; 

    for (size_t yi{0}; yi < 5; ++yi){
        for (size_t xi{0}; xi < 11; ++xi){
            game.aliens[yi * 11 + xi].x = 16 * xi + 20;
            game.aliens[yi * 11 + xi].y = 17 * yi + 128;
        }
    }


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
    while (!glfwWindowShouldClose(window) && game_running){
        buffer_clear(&buffer, clear_color);
        
        // Draw Alien Sprite
        for (size_t ai = 0; ai < game.num_aliens; ++ai){
            const Alien& alien = game.aliens[ai];
            size_t current_frame = alien_animation->time / alien_animation->frame_duration;
            const Sprite& sprite = *alien_animation->frames[current_frame]; 
            buffer_sprite_draw(&buffer, sprite,
                    alien.x, alien.y, rgb_to_uint32(255, 0, 0));
        }

        buffer_sprite_draw(&buffer, player_sprite,
                game.player.x, game.player.y, rgb_to_uint32(255, 255, 255));
        

        // Update Alien Animation 
        ++alien_animation->time;
        if (alien_animation->time == alien_animation->num_frames *
                alien_animation->frame_duration){
            if (alien_animation->loop) alien_animation->time = 0;
            else {
                delete alien_animation;
                alien_animation = nullptr;
            }
        }

        // Bevegelses logikk
        int player_move_dir = 2 * move_dir;
        if (player_move_dir != 0){
            if (game.player.x + player_sprite.width + player_move_dir >= game.width){
                game.player.x = game.width - player_sprite.width;
            } else if ((int)game.player.x + player_move_dir <= 0){
                game.player.x = 0; 
            } else
                game.player.x += player_move_dir; 
        }

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
    delete[] alien_sprite.data;

    std::cout << "Exiting...\n";
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    switch (key){
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS) game_running = false;
            break;
        case GLFW_KEY_RIGHT:
            if (action == GLFW_PRESS) move_dir += 1;
            else if (action == GLFW_RELEASE) move_dir -= 1;
            break;
        case GLFW_KEY_LEFT:
            if (action == GLFW_PRESS) move_dir -= 1;
            else if (action == GLFW_RELEASE) move_dir += 1;
            break;
        default:
            break;
    }
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

#include <iostream>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void error_callback(int, const char*);


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

    // printer opengl versjon
    int major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "Using OpenGL " << major << "." << minor << std::endl;

    // Spill løkken 
    glClearColor(1.0, 0.0, 0.0, 1.0);
    while (!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "Exiting...\n";
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}


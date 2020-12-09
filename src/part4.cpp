// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/texture.hpp>

std::vector<GLuint> vertex_vector;
std::vector<GLuint> num_indicator;

// defining a struct
// purpose - to same multiple models and
// objects and their information in one place with
// proper organization and neatness :)
// tutorials from learnopengl.com were used to contruct this stuct
// https://learnopengl.com/Model-Loading/Mesh
struct ModelObjects{
    std::vector<glm::vec3> MV;
    std::vector<glm::vec2> MU;
    std::vector<glm::vec3> MN;
    Model M;
    glm::mat4 MM;
    GLuint vid;
};

int main( void )
{
    
    
    
    /**********************************/
    /*** APPLICATION INITIALIZATION ***/
    /**********************************/
    
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "CMPT 485", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    // Ensure we can capture the escape key being pressed below
//    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);
    
    // Initialize mouse callbacks
    initializeMouseCallbacks();
    
    
    /**********************************/
    /***** OPENGL INITIALIZATION ******/
    /**********************************/
    
    
    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    
    
    
    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );
    // Use our shader
    glUseProgram(programID);
    
    // Get a handle for our "MVP" uniform
    GLuint ViewProjectionMatrixID = glGetUniformLocation(programID, "VP");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    
    // Initialize GLFW control callbacks
    initializeMouseCallbacks();
    
    // Projection and Model matrices are fixed
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    
    /**********************************/
    /**** LOAD MODEL INFORMATION ******/
    /**********************************/
    
    std::vector<Model> models;
    std::vector<ModelObjects> model_objects;
    
    loadModels("default.models", models);
    
    //buffer initialization
    GLuint tex_id;
    GLuint vertex_id;
    GLuint vertex_buffer;
    GLuint normalsbuffer;

    
    for (int i = 0; i<models.size(); i++){
        Model model = models[i]; //each model
        
        glm::mat4 I = glm::mat4(1.0f); // Identity Matrix
        glm::mat4 Ms = glm::scale(I, glm::vec3(model.sx, model.sy, model.sz));
        glm::mat4 Mr = glm::rotate(I, model.ra,glm::vec3(model.rx, model.ry, model.rz));
        glm::mat4 Mt = glm::translate(I, glm::vec3(model.tx, model.ty, model.tz));
        glm::mat4 Mminust = glm::translate(I, glm::vec3(-model.tx, -model.ty, -model.tz));
        glm::mat4 ModelMatrix = Mt * Mr * Ms;
        
        // Read our .obj file
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<glm::ivec3> vertex_indices;
        std::vector<glm::ivec3> uv_indices;
        std::vector<glm::ivec3> normal_indices;
        
        bool loadSuccess = loadOBJ(model.objFilename.c_str(), vertices, uvs, normals);
        if (!loadSuccess) {
            return -1;
        }
        
        //initialzing a the struct we constructed in the very beginning
        ModelObjects OG = {vertices, uvs, normals, model, ModelMatrix, vertex_id};
        model_objects.push_back(OG);
        //store the size every iteration
        GLsizei UV_size_vertex = uvs.size();

        
        // need to keep track of vbo sizes for drawing later
        GLsizei numVertices = vertices.size(); // should be same as numNormals
        num_indicator.push_back(numVertices);
        GLsizei numVertexIndices = vertex_indices.size();
        
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        
        //read .bmp file
        // assistant tutorials for reading bmp files were observed from below
        //
        tex_id = loadBMP_custom(model.textureFilename.c_str());
        //bind texture
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glGenBuffers(1, &tex_id);
        glBindBuffer(GL_ARRAY_BUFFER, tex_id);
        glBufferData(GL_ARRAY_BUFFER, UV_size_vertex * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
        
        //fragment shader sampler
        glUniform1i(glGetUniformLocation(programID, "t_sampler"), 0);
        GLuint vTexCoord = glGetAttribLocation( programID, "vTexCoord" );
        
        glGenVertexArrays(1, &vertex_id);
        glBindVertexArray(vertex_id);
        vertex_vector.push_back(vertex_id);
        
        /********************************************/
        /*** ASSOCIATE DATA WITH SHADER VARIABLES ***/
        /********************************************/
        
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(
                              0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : normals
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
                              1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : VtexCoord
        glEnableVertexAttribArray(vTexCoord);
        glBindBuffer(GL_ARRAY_BUFFER, tex_id);
        glVertexAttribPointer(
                              2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                              2,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        /**********************************/
        /*** UNBIND VERTEX-ARRAY OBJECT ***/
        /**********************************/
        
        glBindVertexArray(0);
        
    }
    
    do{
        
        // get updated View matrix from keyboard and mouse input
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 VP = ProjectionMatrix * ViewMatrix;
        
        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(ViewProjectionMatrixID, 1, GL_FALSE, &VP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model_objects[0].MM[0][0]);
        
        
        
        /**********************************/
        /************** DRAW  *************/
        /**********************************/
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //iteration through the model buffer
        for (int i = 0; i < models.size(); i++){
            // Bind VAO
            glBindVertexArray(vertex_vector[i]);
            glBindTexture(GL_TEXTURE_2D, i+1);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            
            // Set our Model transform matrix
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model_objects[i].MM[0][0]);
            
            glDrawArrays(GL_TRIANGLES, 0, num_indicator[i]);
            
            // Unbind VAO
            glBindVertexArray(0);
            
        }
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &normalsbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &vertex_id);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
    return 0;
}


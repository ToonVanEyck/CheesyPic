#include "render_thread.h"

#include <turbojpeg.h>

static unsigned long readJpg(char *name, char **data){
    FILE *file;
	unsigned long size;
    file = fopen(name, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", name);
        return 1;
    }
    
    //Get file length
    fseek(file, 0, SEEK_END);
    size=ftell(file);
    fseek(file, 0, SEEK_SET);
    //Allocate memory
    *data=(char *)malloc(size+1);
    if (!*data)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return 1;
    }
    fread(*data, size, 1, file);
    fclose(file);
    return size;
}
 
static int renderRunning;

static const struct
{
    float x, y;
    float r, g, b;
    float s, t;
} vertices[] =
{
    {-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // Top-left
    { 1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}, // Top-right
    { 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f}, // Bottom-right
    {-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f}  // Bottom-left
};
#define NUM_TEXTURES 2
static const char* vertex_shader_text =
"#version 140\n"
"in vec3 vCol;\n"
"in vec2 vPos;\n"
"in vec2 texcoord;\n"
"out vec3 color;\n"
"out vec2 Texcoord;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(vPos, 0.0, 1.0);\n"
"    Texcoord = texcoord;\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 140\n"
"in vec3 color;\n"
"in vec2 Texcoord;\n"
"out vec4 outColor;\n"
"uniform sampler2D tex_preview;\n"
"uniform sampler2D texPuppy;\n"
"void main()\n"
"{\n"
"   outColor = mix(texture(texPuppy, Texcoord), texture(tex_preview, Texcoord), 0.8);//* vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stdout, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void start_render_thread(jpeg_buffer_t *shared_buffer)
{
    printf("%sStarted render thread!\n",PR);
    signal(SIGINT, stop_render_thread);
    GLuint textures[NUM_TEXTURES];
    GLuint program, fragment_shader, vertex_shader, ebo, vbo;
    GLFWwindow* window;
    init_render_thread(&window,textures,&program,&fragment_shader,&vertex_shader,&ebo, &vbo);
    renderRunning = 1;
    run_render_thread(shared_buffer, &window,program);
    cleanup_render_thread(&window,textures,&program,&fragment_shader,&vertex_shader,&ebo, &vbo);
    printf("%sFinished render thread!\n",PR);
}
void stop_render_thread(int dummy)
{
    renderRunning = 0;
}

int init_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program,GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo)
{
    GLint vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    *window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!*window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(*window, key_callback);

    glfwMakeContextCurrent(*window);
    gladLoadGL();
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity
    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };
    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(elements), elements, GL_STATIC_DRAW);


    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    *vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(*vertex_shader);

    *fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(*fragment_shader);

    *program = glCreateProgram();
    glAttachShader(*program, *vertex_shader);
    glAttachShader(*program, *fragment_shader);
    glBindFragDataLocation(*program, 0, "outColor");
    glLinkProgram(*program);

    vpos_location = glGetAttribLocation(*program, "vPos");
    vcol_location = glGetAttribLocation(*program, "vCol");
    GLint texAttrib = glGetAttribLocation(*program, "texcoord");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*) (sizeof(float) * 5));
    glUseProgram(*program);
    glGenTextures(NUM_TEXTURES, textures);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
        // size = readJpg("test.jpg",&jpeg);
        // decodeJpg(jpeg,size,&image,&width,&height);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(*program, "texPuppy"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // int width, height;
    // unsigned char* image;
    // char* jpeg;
    // unsigned long size;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
        // size = readJpg("capture_preview.jpg",&jpeg);
        // decodeJpg(jpeg,size,&image,&width,&height);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(*program, "tex_preview"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void cleanup_render_thread(GLFWwindow **window, GLuint *textures,GLuint *program,GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo)
{
    glDeleteTextures(NUM_TEXTURES, textures);
    glDeleteProgram(*program);
    glDeleteShader(*fragment_shader);
    glDeleteShader(*vertex_shader);
    glDeleteBuffers(1, ebo);
    glDeleteBuffers(1, vbo);
    glfwDestroyWindow(*window);
    glfwTerminate();
}

void run_render_thread(jpeg_buffer_t *shared_buffer, GLFWwindow **window, GLuint program)
{
    while (!glfwWindowShouldClose(*window) && renderRunning)
    {
        sem_wait(&shared_buffer[0].sem_render);
        if(shared_buffer[0].state == render){
            int width, height;
            glfwGetFramebufferSize(*window, &width, &height);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);

            // unsigned char* image;
            // char* jpeg;
            // unsigned long size;
            // shared_buffer[0].size = readJpg("capture_preview.jpg",&jpeg);
            // int jpegSubsamp;
            // tjhandle _jpegDecompressor = tjInitDecompress();
            // tjDecompressHeader2(_jpegDecompressor, 
            //                     (unsigned char*) jpeg,
            //                     shared_buffer[0].size, 
            //                     &shared_buffer[0].width, 
            //                     &shared_buffer[0].height, 
            //                     &jpegSubsamp);         
            // tjDecompress2(  _jpegDecompressor, 
            //                 (unsigned char*) jpeg, 
            //                 shared_buffer[0].size, 
            //                 shared_buffer[0].uncompressed_data,
            //                 shared_buffer[0].width, 
            //                 0,
            //                 shared_buffer[0].height, 
            //                 TJPF_RGB, TJFLAG_FASTDCT);
            //                 shared_buffer[0].state = render;
            // tjDestroy(_jpegDecompressor);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, shared_buffer[0].width, 
                        shared_buffer[0].height, 0, GL_RGB, GL_UNSIGNED_BYTE, 
                        shared_buffer[0].uncompressed_data);

            glUniform1i(glGetUniformLocation(program, "tex_preview"), 1);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glfwSwapBuffers(*window);
            shared_buffer[0].state = capture;
        }
        glfwPollEvents();
    }

}
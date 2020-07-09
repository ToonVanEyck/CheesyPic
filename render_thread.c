#include "render_thread.h"

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
"uniform  mat4 resize;\n"
"uniform  mat2 preview_mirror;\n"
"uniform  mat2 reveal_mirror;\n"
"out vec3 color;\n"
"out vec2 tex_preview_coord;\n"
"out vec2 tex_overlay_coord;\n"
"void main()\n"
"{\n"
"    gl_Position = resize * vec4(vPos, 0.0, 1.0);\n"
"    tex_preview_coord = texcoord * preview_mirror;\n"
"    tex_overlay_coord = texcoord * reveal_mirror;\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 140\n"
"in vec3 color;\n"
"in vec2 tex_preview_coord;\n"
"in vec2 tex_overlay_coord;\n"
"out vec4 outColor;\n"
"uniform sampler2D tex_preview;\n"
"uniform sampler2D tex_overlay;\n"
"void main()\n"
"{\n"
"   outColor = mix(texture(tex_preview, tex_preview_coord),texture(tex_overlay, tex_overlay_coord), texture(tex_overlay, tex_overlay_coord).a);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stdout, "Error: %s\n", description);
}
 
static char button_pushed;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }else if (action == GLFW_PRESS){
        button_pushed = *glfwGetKeyName(key,scancode);
    }
}

void start_render_thread(shared_memory_t *shared_memory)
{
    printf("%sStarted render thread!\n",PR);
    signal(SIGINT, stop_render_thread);
    GLuint textures[NUM_TEXTURES];
    GLuint program, resize_mat, preview_mirror_mat, reveal_mirror_mat, fragment_shader, vertex_shader, ebo, vbo;
    GLFWwindow* window;
    init_render_thread(&window, textures, &program, &resize_mat, &preview_mirror_mat, &reveal_mirror_mat, &fragment_shader, &vertex_shader, &ebo, &vbo);
    renderRunning = 1;
    run_render_thread(shared_memory, &window,program, resize_mat, preview_mirror_mat, reveal_mirror_mat);
    cleanup_render_thread(&window, textures, &program, &resize_mat, &preview_mirror_mat, &reveal_mirror_mat, &fragment_shader, &vertex_shader, &ebo, &vbo);
    printf("%sFinished render thread!\n",PR);
}
void stop_render_thread(int dummy)
{
    renderRunning = 0;
}

int init_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program, GLuint *resize_mat, GLuint *preview_mirror_mat, GLuint *reveal_mirror_mat, GLuint *fragment_shader, GLuint *vertex_shader, GLuint *ebo, GLuint *vbo)
{
    GLint vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    *window = glfwCreateWindow(600, 400, "CheesyPic Photobooth by ToonVanEyck", NULL/*glfwGetPrimaryMonitor()*/, NULL);
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

    *resize_mat = glGetUniformLocation(*program, "resize");
    *preview_mirror_mat = glGetUniformLocation(*program, "preview_mirror");
    *reveal_mirror_mat = glGetUniformLocation(*program, "reveal_mirror");
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

    // int width, height;
    // unsigned char* image;
    // char* jpeg;
    // unsigned long size;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
        // size = readJpg("test.jpg",&jpeg);
        // decodeJpg(jpeg,size,&image,&width,&height);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(*program, "tex_overlay"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
        // size = readJpg("capture_preview.jpg",&jpeg);
        // decodeJpg(jpeg,size,&image,&width,&height);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(*program, "tex_preview"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void cleanup_render_thread(GLFWwindow **window, GLuint *textures,GLuint *program ,GLuint *resize_mat, GLuint *preview_mirror_mat ,GLuint *reveal_mirror_mat, GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo)
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

void run_render_thread(shared_memory_t *shared_memory, GLFWwindow **window, GLuint program, GLuint resize_mat, GLuint preview_mirror_mat ,GLuint reveal_mirror_mat)
{
    const struct timespec sem_timespec = {0,100000};
    mat4x4 m;
    while (!glfwWindowShouldClose(*window) && renderRunning)
    {
        switch(button_pushed){
            case 'a':
                shared_memory->photobooth_active ^= 1;
                break;
            case 'c':
                if(shared_memory->logic_state == log_idle && shared_memory->photobooth_active){
                    shared_memory->logic_state = log_triggred; 
                }break;
            case '+':
                shared_memory->photobooth_active = 0;
                if(shared_memory->logic_state == LAST_STATE){shared_memory->logic_state = FIRST_STATE;}
                else {shared_memory->logic_state++;}
                break;
            case '-':
                shared_memory->photobooth_active = 0;
                if(shared_memory->logic_state == FIRST_STATE){shared_memory->logic_state = LAST_STATE;}
                else{shared_memory->logic_state--;}
                break;
            case 'l':
                shared_memory->preview_mirror ^= 1;
                break;
            case 'm':
                shared_memory->reveal_mirror ^= 1;
                break;
            case 'f':
                shared_memory->fastmode ^= 1;
                break;
            default:
                break;
        }
      
        int width, height;
        glfwGetFramebufferSize(*window, &width, &height);

        //setup transformation matrix so previeuw is always shown in correct aspect ratio
        mat4x4_identity(m);
        float square_hight = (float)PREVIEW_WIDTH / (float)PREVIEW_HEIGHT * (float)height;
        m[0][0] = (square_hight < width)?((float)PREVIEW_WIDTH*(float)height)/((float)PREVIEW_HEIGHT*(float)width):1.0;
        m[1][1] = (square_hight >= width)?((float)PREVIEW_HEIGHT*(float)width)/((float)PREVIEW_WIDTH*(float)height):1.0;
        glViewport(0, 0, width, height);
        
        vec2 mirror_preview[2]={{1,0},{0,1}};
        vec2 mirror_reveal[2]={{1,0},{0,1}};
        mirror_preview[0][0] = (shared_memory->preview_mirror==1)?(-1):(1);

        if(sem_timedwait(&shared_memory->sem_render,&sem_timespec) == 0){
            if(shared_memory->logic_state != log_reveal  || !shared_memory->photobooth_active){
                //  -------- PREVIEW LOGIC --------
                for(int i = 0;i<NUM_JPEG_BUFFERS;i++){
                    if(shared_memory->preview_buffer[i].pre_state == pre_render){
                        glActiveTexture(GL_TEXTURE1);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shared_memory->preview_buffer[i].width, 
                                    shared_memory->preview_buffer[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                                    shared_memory->preview_buffer[i].raw_data);
                        glUniform1i(glGetUniformLocation(program, "tex_preview"), 1);
                        shared_memory->preview_buffer[i].pre_state = pre_capture;
                        break;
                    }
                }
                glActiveTexture(GL_TEXTURE0);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shared_memory->overlay_buffer.width, 
                    shared_memory->overlay_buffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                    shared_memory->overlay_buffer.raw_data);
                glUniform1i(glGetUniformLocation(program, "tex_overlay"), 0);

                glUniformMatrix2fv(preview_mirror_mat, 1, GL_FALSE, (const GLfloat*) mirror_preview);
                glUniformMatrix2fv(reveal_mirror_mat, 1, GL_FALSE, (const GLfloat*) mirror_reveal);
                glUniformMatrix4fv(resize_mat, 1, GL_FALSE, (const GLfloat*) m);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glfwSwapBuffers(*window);
            }else{
                // -------- CAPTURE LOGIC --------
                mirror_reveal[0][0] = (shared_memory->reveal_mirror==1)?(-1):(1);
                glActiveTexture(GL_TEXTURE0);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shared_memory->capture_buffer.width, 
                            shared_memory->capture_buffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                            shared_memory->capture_buffer.raw_data);
                glUniform1i(glGetUniformLocation(program, "tex_overlay"), 0);
                glUniformMatrix2fv(preview_mirror_mat, 1, GL_FALSE, (const GLfloat*) mirror_preview);
                glUniformMatrix2fv(reveal_mirror_mat, 1, GL_FALSE, (const GLfloat*) mirror_reveal);
                glUniformMatrix4fv(resize_mat, 1, GL_FALSE, (const GLfloat*) m);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glfwSwapBuffers(*window);
            }
        }
        if(button_pushed != 0) button_pushed = 0;
        sem_post(&shared_memory->sem_logic);
        glfwPollEvents();
    }

}
#include "render_thread.h"

#define GLAD_GLES2_IMPLEMENTATION
#include <glad/gles2.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int init_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program,GLuint *resize_mat, GLuint *mirror_liveview_mat ,GLuint *mirror_preview_mat, GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo);
void cleanup_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program,GLuint *resize_mat, GLuint *mirror_liveview_mat ,GLuint *mirror_preview_mat, GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo);
void run_render_thread(shared_memory_t *shared_memory, GLFWwindow **window, GLuint program, GLuint resize_mat ,GLuint mirror_liveview_mat ,GLuint mirror_preview_mat);


static unsigned long readJpg(char *name, char **data){
    FILE *file;
	unsigned long size;
    file = fopen(name, "rb");
    if (!file)
    {
        LOG("Unable to open file %s", name);
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
        LOG("Memory error!");
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
"#version 100\n"
"precision mediump float;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"attribute vec2 texcoord;\n"
"uniform  mat4 resize;\n"
"uniform  mat2 mirror_liveview;\n"
"uniform  mat2 mirror_preview;\n"
"varying vec3 color;\n"
"varying vec2 tex_preview_coord;\n"
"varying vec2 tex_overlay_coord;\n"
"void main()\n"
"{\n"
"    gl_Position = resize * vec4(vPos, 0.0, 1.0);\n"
"    tex_preview_coord = texcoord * mirror_liveview;\n"
"    tex_overlay_coord = texcoord * mirror_preview;\n"
"    color = vCol;\n"
"}\n";
 
// "out vec4 outColor;\n"
static const char* fragment_shader_text =
"#version 100\n"
"precision mediump float;\n"
"varying vec3 color;\n"
"varying vec2 tex_preview_coord;\n"
"varying vec2 tex_overlay_coord;\n"
"uniform sampler2D tex_preview;\n"
"uniform sampler2D tex_overlay;\n"
"void main()\n"
"{\n"
"   gl_FragColor = mix(texture2D(tex_preview, tex_preview_coord),texture2D(tex_overlay, tex_overlay_coord), texture2D(tex_overlay, tex_overlay_coord).a);\n"
"}\n";


static void error_callback(int error, const char* description)
{
    LOG("Error: %s\n", description);
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

static void window_focus_callback(GLFWwindow* window, int focused)
{
    if (focused)
    {
        // The window gained input focus
    }
    else
    {
        // glfwFocusWindow(window);
    }
}

void start_render_thread(shared_memory_t *shared_memory)
{
    LOG("Started render thread!\n");
    signal(SIGINT, stop_render_thread);
    GLuint textures[NUM_TEXTURES];
    GLuint program, resize_mat, mirror_liveview_mat, mirror_preview_mat, fragment_shader, vertex_shader, ebo, vbo;
    GLFWwindow* window;
    init_render_thread(&window, textures, &program, &resize_mat, &mirror_liveview_mat, &mirror_preview_mat, &fragment_shader, &vertex_shader, &ebo, &vbo);

    printf("OpenGL info:\n"
	       "\tVendor   = \"%s\"\n"
	       "\tRenderer = \"%s\"\n"
	       "\tVersion  = \"%s\"\n",
	       glGetString(GL_VENDOR),
	       glGetString(GL_RENDERER),
	       glGetString(GL_VERSION));

    renderRunning = 1;
    run_render_thread(shared_memory, &window,program, resize_mat, mirror_liveview_mat, mirror_preview_mat);
    cleanup_render_thread(&window, textures, &program, &resize_mat, &mirror_liveview_mat, &mirror_preview_mat, &fragment_shader, &vertex_shader, &ebo, &vbo);
    LOG("Finished render thread!\n");
}
void stop_render_thread(int dummy)
{
    renderRunning = 0;
}


GLFWmonitor* primaryMonitor;
struct windowParams{int xpos; int ypos; int width; int height;};
struct windowParams windowParams;

int init_render_thread(GLFWwindow **window, GLuint *textures, GLuint *program, GLuint *resize_mat, GLuint *mirror_liveview_mat, GLuint *mirror_preview_mat, GLuint *fragment_shader, GLuint *vertex_shader, GLuint *ebo, GLuint *vbo)
{
    GLint vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) exit(EXIT_FAILURE);

    // glfwWindowHint(GLFW_FLOATING , GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);


    primaryMonitor = glfwGetPrimaryMonitor();
    *window = glfwCreateWindow(600, 400, "CheesyPic Photobooth by ToonVanEyck", WINDOW?NULL:primaryMonitor, NULL);
    if (!*window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glfwSetKeyCallback(*window, key_callback);
    glfwSetWindowFocusCallback(*window, window_focus_callback);

    glfwMakeContextCurrent(*window);

    gladLoadGLES2(glfwGetProcAddress);
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
    // glBindFragDataLocation(*program, 0, "outColor");
    glLinkProgram(*program);
    *resize_mat = glGetUniformLocation(*program, "resize");
    *mirror_liveview_mat = glGetUniformLocation(*program, "mirror_liveview");
    *mirror_preview_mat = glGetUniformLocation(*program, "mirror_preview");
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

void cleanup_render_thread(GLFWwindow **window, GLuint *textures,GLuint *program ,GLuint *resize_mat, GLuint *mirror_liveview_mat ,GLuint *mirror_preview_mat, GLuint *fragment_shader,GLuint *vertex_shader, GLuint *ebo, GLuint *vbo)
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

void run_render_thread(shared_memory_t *shared_memory, GLFWwindow **window, GLuint program, GLuint resize_mat, GLuint mirror_liveview_mat ,GLuint mirror_preview_mat)
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
                shared_memory->mirror_liveview ^= 1;
                break;
            case 'm':
                shared_memory->mirror_preview ^= 1;
                break;
            case 'f':
                shared_memory->fastmode ^= 1;
                break;
            case 'p':
                shared_memory->toggle_printer ^= 1;
                break;
            case 'w': // toggle windowed / windowless
                if(glfwGetWindowMonitor(*window)){
                    glfwSetWindowMonitor(*window,NULL,windowParams.xpos,windowParams.ypos,windowParams.width,windowParams.height,GLFW_DONT_CARE);
                }else{
                    glfwGetWindowPos(*window,&windowParams.xpos,&windowParams.ypos);
                    glfwGetWindowSize(*window,&windowParams.width,&windowParams.height);
                    int width, height;
                    glfwGetMonitorWorkarea(primaryMonitor,NULL,NULL,&width,&height);
                    glfwSetWindowMonitor(*window,primaryMonitor,0,0,width,height,GLFW_DONT_CARE);
                }
                break;
            case 's':
                glfwSetWindowShouldClose(*window, GLFW_TRUE);
                shared_memory->exit_slow = 1;
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
        mirror_preview[0][0] = (shared_memory->mirror_liveview==1)?(-1):(1);

        if(sem_timedwait(&shared_memory->sem_render,&sem_timespec) == 0){
            if((shared_memory->logic_state != log_reveal && shared_memory->logic_state != log_procces) || !shared_memory->photobooth_active){
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
                glUniformMatrix2fv(mirror_liveview_mat, 1, GL_FALSE, (const GLfloat*) mirror_preview);
                glUniformMatrix2fv(mirror_preview_mat, 1, GL_FALSE, (const GLfloat*) mirror_reveal);
                glUniformMatrix4fv(resize_mat, 1, GL_FALSE, (const GLfloat*) m);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                glfwSwapBuffers(*window);
            }else{
                // -------- CAPTURE LOGIC --------
                mirror_reveal[0][0] = (shared_memory->mirror_preview==1)?(-1):(1);
                glActiveTexture(GL_TEXTURE0);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shared_memory->capture_buffer.width, 
                            shared_memory->capture_buffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                            shared_memory->capture_buffer.raw_data);
                glUniform1i(glGetUniformLocation(program, "tex_overlay"), 0);
                glUniformMatrix2fv(mirror_liveview_mat, 1, GL_FALSE, (const GLfloat*) mirror_preview);
                glUniformMatrix2fv(mirror_preview_mat, 1, GL_FALSE, (const GLfloat*) mirror_reveal);
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
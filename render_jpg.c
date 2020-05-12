#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>

#include <turbojpeg.h>

#include <glad/glad.h>
//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// #include "linmath.h"
 
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <unistd.h>
 
static double current_time(void)
{
   struct timeval tv;
   gettimeofday(&tv, NULL );
   return (double) tv.tv_sec + tv.tv_usec / 1000000.0;
}

static const struct
{
    float x, y;
    float r, g, b;
    float s, t;
} vertices[] =
{
    {-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // Top-left
    { 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}, // Top-right
    { 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f}, // Bottom-right
    {-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f}  // Bottom-left
};
 
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
"uniform sampler2D tex_overlay;\n"
"void main()\n"
"{\n"
"   outColor = mix(texture(tex_overlay, Texcoord), texture(tex_preview, Texcoord), 0.8);//* vec4(color, 1.0);\n"
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
    printf("size %ld\n",size);
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
 
static void decodeJpg(char *data, unsigned long size, unsigned char **buffer, int *_width, int *_height){
    int jpegSubsamp;
    tjhandle _jpegDecompressor = tjInitDecompress();
    tjDecompressHeader2(_jpegDecompressor, (unsigned char*) data, size, _width, _height, &jpegSubsamp);
    //unsigned char buffer[_width*_height*3]; //!< will contain the decompressed image
    *buffer=(unsigned char *)malloc(*_width**_height*3);
    tjDecompress2(_jpegDecompressor, (unsigned char*) data, size, *buffer, *_width, 0/*pitch*/, *_height, TJPF_RGB, TJFLAG_FASTDCT);
    tjDestroy(_jpegDecompressor);
}

static void startCamera(GPContext **context, Camera **camera){
    *context = gp_context_new();
    CameraList *cameraList;
	gp_list_new (&cameraList);
	    int count = gp_camera_autodetect(cameraList, *context);
    if (count <= 0)
    {
        if (count == 0) {
            printf("No camera detected\n");
        } else {
            printf("GP ERROR :%d\n",count);
        }
        gp_context_unref(*context);
        exit(1);
    }else{
		printf("cameras detected\n");
		const char *modelName = NULL, *portName = NULL;
		gp_list_get_name  (cameraList, 0, &modelName);
		gp_list_get_value (cameraList, 0, &portName);
		printf("found model: %s @ %s\n",modelName, portName);




		/* Prepare connection to the camera. */
		CameraAbilitiesList *al = NULL;
		CameraAbilities a;
		gp_abilities_list_new (&al);
		gp_abilities_list_load (al, NULL);
		int i = gp_abilities_list_lookup_model (al, modelName);
		if (i < 0)
			printf("Could not find model: '%s'.\n",gp_result_as_string (i));
		gp_abilities_list_get_abilities (al, i, &a);
		
		
		GPPortInfoList *il = NULL;
		GPPortInfo info;
		gp_port_info_list_new (&il);
		gp_port_info_list_load (il);
		i = gp_port_info_list_lookup_path (il, portName);
		if (i < 0)
			printf("Could not find port: '%s'.\n",gp_result_as_string (i));
		gp_port_info_list_get_info (il, i, &info);

		/* Capture an image, download it and delete it. */
		printf("Initializing camera...\n");
		CameraFilePath path;
		const char *data;
        const char *mime_type;
		unsigned long size;

		i = gp_camera_new (camera);
		printf("%d %s\n",i,a.model);
		gp_camera_set_abilities (*camera, a);
		gp_camera_set_port_info (*camera, info);

        gp_list_free(cameraList);
		gp_abilities_list_free (al);
		gp_port_info_list_free (il);
    }
}

int main(int argc, char *argv[])
{
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    //gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    gladLoadGL();
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity
    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(elements), elements, GL_STATIC_DRAW);


    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindFragDataLocation(program, 0, "outColor");
    glLinkProgram(program);

    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
    GLint texAttrib = glGetAttribLocation(program, "texcoord");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*) (sizeof(float) * 2));

    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*) (sizeof(float) * 5));
    glUseProgram(program);
    GLuint textures[2];
    glGenTextures(2, textures);

    
    int width, height;
    unsigned char* image;
    char* jpeg;
    unsigned long size;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
        size = readJpg("test.jpg",&jpeg);
        decodeJpg(jpeg,size,&image,&width,&height);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(program, "tex_overlay"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(jpeg);
    free(image);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
        
        size = readJpg("capture_preview.jpg",&jpeg);
        decodeJpg(jpeg,size,&image,&width,&height);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glUniform1i(glGetUniformLocation(program, "tex_preview"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(image);


    GPContext *context;
    Camera *camera;
    startCamera(&context,&camera);

    //gp_file_free(file);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        const char *data;
        const char *mime_type;
        //unsigned long size;
        CameraFile *file = NULL;
        gp_file_new (&file);
        gp_camera_capture_preview (camera, file, context);
        //gp_file_get_mime_type(file,&mime_type);
        gp_file_get_data_and_size (file, &data, &size);
        decodeJpg((char*)data,size,&image,&width,&height);

        //decodeJpg(jpeg,size,(unsigned char **)&image,&width,&height);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glUniform1i(glGetUniformLocation(program, "tex_preview"), 1);
        free(image);
        gp_file_free(file);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        static int frames = 0;
        static double tRate0 = -1.0;
        double t = current_time();
        frames++;
        if (tRate0 < 0.0)
            tRate0 = t;
        if (t - tRate0 >= 5.0) {
            GLfloat seconds = t - tRate0;
            GLfloat fps = frames / seconds;
            printf("%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds,fps);
            tRate0 = t;
            frames = 0;
        }
    }

    glDeleteTextures(2, textures);

    glDeleteProgram(program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vertex_buffer);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
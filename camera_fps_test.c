#include <gphoto2/gphoto2-abilities-list.h>
#include <gphoto2/gphoto2-list.h>
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>

#include <turbojpeg.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

static double current_time(void)
{
   struct timeval tv;
   gettimeofday(&tv, NULL );
   return (double) tv.tv_sec + tv.tv_usec / 1000000.0;
}
 
int main(int argc, char *argv[])
{
    signal(SIGINT, intHandler);
    if(argc > 1){
        printf("Captureing and decoding!\n");
    }else{
        printf("Captureing only!\n");
    }
	GPContext *context = gp_context_new();
	CameraList *cameraList = NULL;
	gp_list_new (&cameraList);
	    int count = gp_camera_autodetect(cameraList, context);
    if (count <= 0)
    {
        if (count == 0) {
            printf("No camera detected\n");
        } else {
            printf("GP ERROR :%d\n",count);
        }
        gp_context_unref(context);
    }else{

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
		CameraFilePath path;
		
		Camera *camera = NULL;
		const char *data;
        const char *mime_type;
		unsigned long size;

		i = gp_camera_new (&camera);
		gp_camera_set_abilities (camera, a);
		gp_camera_set_port_info (camera, info);

        printf("Calculating fps over 5s period.\n");
        while (keepRunning)
        {
            CameraFile *file = NULL;
            if( gp_file_new (&file) < 0 ){
                printf("failed: gp_file_new\n");
                break;
            }
            if( gp_camera_capture_preview (camera, file, context) < 0 ){
                printf("failed: gp_camera_capture_preview\n");
                break;
            }
            if( gp_file_get_mime_type(file,&mime_type) < 0 ){
                printf("failed: gp_file_get_mime_type\n");
                break;
            }
            if( gp_file_get_data_and_size (file, &data, &size) < 0 ){
                printf("failed: gp_file_get_data_and_size\n");
                break;
            }
            if( gp_file_free(file) < 0 ){
                printf("failed: gp_file_free\n");
                break;
            }

            if(argc > 1){
                int jpegSubsamp, _width, _height;
                tjhandle _jpegDecompressor = tjInitDecompress();
                tjDecompressHeader2(_jpegDecompressor, (unsigned char*) data, size, &_width, &_height, &jpegSubsamp);
                unsigned char buffer[_width*_height*3]; //!< will contain the decompressed image
                tjDecompress2(_jpegDecompressor, (unsigned char*) data, size, buffer, _width, 0/*pitch*/, _height, TJPF_RGB, TJFLAG_FASTDCT);
                tjDestroy(_jpegDecompressor);
            }

            static int frames = 0;
            static double tRate0 = -1.0;
            double t = current_time();
            frames++;
            if (tRate0 < 0.0)
                tRate0 = t;
            if (t - tRate0 >= 5.0) {
                float seconds = t - tRate0;
                float fps = frames / seconds;
                printf("%d frames in %3.1f seconds = %6.3f FPS\n", frames, seconds,fps);
                tRate0 = t;
                frames = 0;
            }
        }

        gp_list_free(cameraList);
        
        gp_camera_exit(camera,context);
		gp_context_unref(context);
		gp_abilities_list_free (al);
		gp_port_info_list_free (il);
	}
}
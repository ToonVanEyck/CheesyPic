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

    const char *name = "../test.jpg";
    FILE *file;
	char *data;
	unsigned long size;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		return;
	}
	
	//Get file length
	fseek(file, 0, SEEK_END);
	size=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	data=(char *)malloc(size+1);
	if (!data)
	{
		fprintf(stderr, "Memory error!");
                                fclose(file);
		return;
	}
	fread(data, size, 1, file);
	fclose(file);
    while (keepRunning){
        int jpegSubsamp, _width, _height;
        tjhandle _jpegDecompressor = tjInitDecompress();
        tjDecompressHeader2(_jpegDecompressor, (unsigned char*) data, size, &_width, &_height, &jpegSubsamp);
        unsigned char buffer[_width*_height*3]; //!< will contain the decompressed image
        tjDecompress2(_jpegDecompressor, (unsigned char*) data, size, buffer, _width, 0/*pitch*/, _height, TJPF_RGB, TJFLAG_FASTDCT);
        tjDestroy(_jpegDecompressor);

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
    free(data);
    return 0;
}
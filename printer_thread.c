#include "printer_thread.h"

static int printerRunning;

void *start_printer_thread(void *shared_memory)
{
    printf("%sStarted printer thread!\n",PP);
    init_printer_thread();
    printerRunning = 1;
    run_printer_thread(shared_memory);
    printf("%sFinished printer thread!\n",PP);
}

void stop_printer_thread()
{
    printerRunning = 0;
}

int init_printer_thread()
{

}

typedef struct{
    char status[16];
    unsigned int media_max;
    unsigned int media_remaining;
    unsigned int lifetime_prints;
}printer_info_t;

#define MAX_LEN 1024

int get_printer_driver_name(char **name)
{
    char buffer[1024];
    FILE *lpoptions = popen("lpoptions", "r");
    if(lpoptions == NULL){
        fprintf(stderr,"failed to read lpoptions\n");
        return 1;
    }
    fread (buffer, 1, 1024, lpoptions);
    pclose(lpoptions);

    regex_t regex;
    regmatch_t match[2];
    if(regcomp(&regex, "gutenprint53\\+usb:\\/\\/([^\\/]*)", REG_EXTENDED)){
        fprintf(stderr,"failed to compile regex\n");
        return 1;
    }
    int ret = regexec(&regex, buffer, 2, match, 0);
    if(!ret){
        int len = match[1].rm_eo-match[1].rm_so;
        *name = malloc(len+1);
        if(*name == NULL){
            regfree(&regex);
            fprintf(stderr,"failed to allocate memory for printer driver name\n");
            return 1;
        }
        (*name)[len]=0;
        memcpy(*name,buffer+match[1].rm_so,len);
    }else if(ret == REG_NOMATCH){
        regfree(&regex);
        return 1;
    }else{
        regerror(ret, &regex, buffer, sizeof(buffer));
        fprintf(stderr, "Regex match failed: %s\n", buffer);
        return 1;
    }

    regfree(&regex);
    return 0;
}

int get_printer_stats_from_json(char *driver_name)
{
    char buffer[1024]= {0};
    sprintf(buffer,"BACKEND_STATS_ONLY=2 BACKEND=%s /usr/lib/cups/backend/gutenprint53+usb -s 2>&1",driver_name);
    FILE *gp = popen(buffer, "r");
    if (gp == NULL){
        fprintf(stderr,"failed to get printer stats\n");
        return 1;
    }
    memset(buffer,0,1024);
    fread (buffer, 1, MAX_LEN, gp);
    pclose(gp);
    printf("-------------------------------\n%s\n",buffer);
    return 0;
}

int get_printer_stats(char *driver_name)
{
    if(!strcmp(driver_name,"mitsubishi-9550dw")){
        printf("printer not yet supported\n");
        return 1;
    }
    if(!strcmp(driver_name,"mitsubishi-d70dw")){
        return get_printer_stats_from_json(driver_name);
    }
}

void run_printer_thread(shared_memory_t *shared_memory)
{
    int ret = 0;
    FILE *fp;
    int length;

    printer_info_t upper_deck,lower_deck;
    memset(&upper_deck,0,sizeof(printer_info_t));
    memset(&lower_deck,0,sizeof(printer_info_t));

    char *pdn;
    get_printer_driver_name(&pdn);
    printf("driver: %s\n",pdn);

    get_printer_stats(pdn);

    free(pdn);

    //     fp = popen("BACKEND_STATS_ONLY=1 BACKEND=mitsu70x /usr/lib/cups/backend/gutenprint53+usb -s", "r");
    //     if (fp == NULL){
    //         fprintf(stderr,"failed to get printer stats\n");
    //     }else{
    //         fread (buffer, 1, MAX_LEN, fp);
    //         pclose(fp);
    //     }

    //     regex_t regex;


    //     printf("Upper Deck:\n");
    //     printf("    Status:           %s\n",upper_deck.status);
    //     printf("    prints remaining: %d / %d\n",upper_deck.media_remaining,upper_deck.media_max);
    //     printf("    lifetime prints : %d\n",upper_deck.lifetime_prints);
    //     printf("Lower Deck:\n");
    //     printf("    Status:           %s\n",lower_deck.status);
    //     printf("    prints remaining: %d / %d\n",lower_deck.media_remaining,lower_deck.media_max);
    //     printf("    lifetime prints : %d\n",lower_deck.lifetime_prints);
    // //}
}

void main(int argc, char *argv[])
{
    run_printer_thread(NULL);
}
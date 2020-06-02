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


typedef enum{
    Idle,
    Printing,
}printing_status_t;

typedef struct{
    char name[16];
    printing_status_t status;
    unsigned int media_max;
    unsigned int media_remaining;
    unsigned int lifetime_prints;
}deck_info_t;

typedef struct{
    unsigned int nuf_of_decks;
    deck_info_t *deck;
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

int get_printer_stats_from_json(char *driver_name, printer_info_t *printer_info)
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

    json_object *printer_stats = json_tokener_parse(buffer);
    json_object *deck_list;
    json_object_object_get_ex(printer_stats,"decks",&deck_list);
    int deck_cnt = 0;
    json_object_object_foreach(deck_list, deck_name, deck) {
        
        strncpy(printer_info->deck[deck_cnt].name,deck_name,15);
        json_object *temp;
        if(json_object_object_get_ex(deck,"status",&temp)){
            printer_info->deck[deck_cnt].status = strcmp(json_object_get_string(temp),"Idle")?Printing:Idle;
        }
        if(json_object_object_get_ex(deck,"medialevelnow",&temp)){
            printer_info->deck[deck_cnt].media_remaining = json_object_get_int(temp);
        }
        if(json_object_object_get_ex(deck,"medialevelmax",&temp)){
            printer_info->deck[deck_cnt].media_max = json_object_get_int(temp);
        }
        if(json_object_object_get_ex(deck,"counters",&temp)){
            json_object *temp_temp;
            if(json_object_object_get_ex(temp,"lifetime",&temp_temp)){
                printer_info->deck[deck_cnt].lifetime_prints = json_object_get_int(temp_temp);
            }
        }
        if(++deck_cnt == printer_info->nuf_of_decks) break;
    }
    return 0;
}

int get_printer_stats(char *driver_name, printer_info_t *printer_info)
{
    if(!strcmp(driver_name,"mitsubishi-9550dw")){
        printf("printer not yet supported\n");
        return 1;
    }
    if(!strcmp(driver_name,"mitsubishi-d70dw")){
        return get_printer_stats_from_json(driver_name, printer_info);
    }
}

void run_printer_thread(shared_memory_t *shared_memory)
{
    int ret = 0;
    FILE *fp;
    int length;

    printer_info_t printer_info;
    printer_info.nuf_of_decks = 2;
    printer_info.deck = malloc(printer_info.nuf_of_decks * sizeof(deck_info_t));
    memset(printer_info.deck,0,printer_info.nuf_of_decks * sizeof(deck_info_t));

    char *pdn;
    get_printer_driver_name(&pdn);
    printf("driver: %s\n",pdn);
    get_printer_stats(pdn,&printer_info);
    for(int i = 0; i<printer_info.nuf_of_decks;i++){
        printf("%s Deck:\n",printer_info.deck[i].name);
        printf("    Status:           %s\n",printer_info.deck[i].status?"Printing":"Idle");
        printf("    prints remaining: %d / %d\n",printer_info.deck[i].media_remaining,printer_info.deck[i].media_max);
        printf("    lifetime prints : %d\n",printer_info.deck[i].lifetime_prints);
    }

    free(pdn);

    //     fp = popen("BACKEND_STATS_ONLY=1 BACKEND=mitsu70x /usr/lib/cups/backend/gutenprint53+usb -s", "r");
    //     if (fp == NULL){
    //         fprintf(stderr,"failed to get printer stats\n");
    //     }else{
    //         fread (buffer, 1, MAX_LEN, fp);
    //         pclose(fp);
    //     }

    //     regex_t regex;
}

void main(int argc, char *argv[])
{
    run_printer_thread(NULL);
}
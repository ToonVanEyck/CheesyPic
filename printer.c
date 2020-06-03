#include "printer.h"

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
    if(feof(gp)){
        if(strstr(buffer,"Permission denied")){
            fprintf(stderr,"Please make sure user has permission to execute (555 or 755) \"/usr/lib/cups/backend/gutenprint53+usb\"\n");
            return 1;
        }else if(strstr(buffer,"No matching printers found")){
            printer_info->connected = 0;
            return 1;
        }else{
            printer_info->connected = 1;
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
        }
    }else{
        fprintf(stderr,"error reading backend\n");
        return 1;
    }
    pclose(gp);
    
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

int print_file(const char *file)
{
    char buffer[1024]= {0};
    sprintf(buffer,"lp %s",file);
    FILE *lp = popen(buffer, "r");
    if (lp == NULL){
        fprintf(stderr,"failed to get printer stats\n");
        return 1;
    }
    pclose(lp);
    return 0;
}

int is_printing_finished(char *driver_name, printer_info_t *printer_info)
{
    static time_t prev_time = 0;
    static printing_status_t curr_status = Idle;
    time_t now;
    time(&now);
    if(difftime(now,prev_time) > 1.25){
        prev_time = now;
        if(!get_printer_stats(driver_name,printer_info)){
            if(curr_status == Idle){
                int busy = 1;
                for(int i = 0 ; i < printer_info->nuf_of_decks ; i++){
                    busy &= (printer_info->deck[i].status == Idle);
                }
                if(busy) return 0;
                curr_status = Printing;
            }
            if(curr_status == Printing){
                int busy=0;
                for(int i = 0 ; i < printer_info->nuf_of_decks ; i++){
                    busy |= (printer_info->deck[i].status == Printing);
                }
                if(busy) return 0;
                curr_status = Idle;
            }
            return 1;
        }
    }
    return 0; 
}

// void main(int argc, char *argv[])
// {
//     printer_info_t printer_info;
//     printer_info.nuf_of_decks = 2;
//     printer_info.deck = malloc(printer_info.nuf_of_decks * sizeof(deck_info_t));
//     memset(printer_info.deck,0,printer_info.nuf_of_decks * sizeof(deck_info_t));

//     char *pdn;
//     get_printer_driver_name(&pdn);
//     printf("driver: %s\n",pdn);
//     get_printer_stats(pdn,&printer_info);
//     if(printer_info.connected){
//         print_file("print_me.jpg");
//         while(!is_printing_finished(pdn,&printer_info));
//     }
//     printf("print finished\n");
//     free(pdn);
// }
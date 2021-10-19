#include "config.h"

int read_config(config_t *config)
{
    memset(config,0,sizeof(config_t));
    GKeyFile *keyfile;
    GError *error = NULL;

    keyfile = g_key_file_new ();
    if (!g_key_file_load_from_file (keyfile, CONFIG_FILE, G_KEY_FILE_NONE, &error)){
        LOG("%s\n",error->message);
        return 1;
    }
    wordexp_t p;

    ///// [design] /////
    char *design_dir = g_key_file_get_string(keyfile,"design","design_directory",NULL);
    char *design_file;
    int use_latest_design = g_key_file_get_boolean(keyfile,"design","use_latest_design",&error);
    if(design_dir && wordexp(design_dir, &p, WRDE_UNDEF) == 0 && p.we_wordc == 1){
        char *design_path;
        if(use_latest_design){
            get_recent_file_in_dir(&design_file,p.we_wordv[0],".design.svg");
        }else{
            design_file = g_key_file_get_string(keyfile,"design","default_design",NULL);
        }
        if(design_file == NULL)
        {
            return 1;
        }
        asprintf(&design_path,"%s%s",p.we_wordv[0],design_file);
        if(load_design_from_file(&config->design,design_path)){
            return 1;
        }
        free(design_path);
    }
    wordfree(&p);


    ///// [theme] /////
    char *theme_dir = g_key_file_get_string(keyfile,"theme","theme_directory",NULL);
    char *theme_file;
    int use_latest_theme = g_key_file_get_boolean(keyfile,"theme","use_latest_theme",&error);
    if(theme_dir && theme_file && wordexp(theme_dir, &p, WRDE_UNDEF) == 0 && p.we_wordc == 1){
        char *theme_path;
        if(use_latest_theme){
            get_recent_file_in_dir(&theme_file,p.we_wordv[0],".theme.svg");
        }else{
            theme_file = g_key_file_get_string(keyfile,"theme","default_theme",NULL);
        }
        if(theme_file == NULL)
        {
            return 1;
        }
        asprintf(&theme_path,"%s%s",p.we_wordv[0],theme_file);
        if(load_theme_from_file(&config->theme,theme_path)){
            return 1;
        }
        free(theme_path);
    }
    wordfree(&p);


    ///// [save] /////
    config->save_photos = g_key_file_get_boolean(keyfile,"save","save_photos",NULL);
    char *save_dir = g_key_file_get_string(keyfile,"save","save_directory",NULL);
     if(save_dir && wordexp(save_dir, &p, WRDE_UNDEF) == 0 && p.we_wordc == 1){
        char *save_dir_with_design;
        char *design_file_extention_token = strstr(design_file,".design.svg");
        if(design_file_extention_token != NULL) *design_file_extention_token = 0;
        asprintf(&save_dir_with_design,"%s%s/",p.we_wordv[0],design_file);
        asprintf(&config->save_path_and_prefix,"%s%s/%s_",p.we_wordv[0],design_file,design_file);
        printf("%s\n",save_dir_with_design);
        printf("%s\n",config->save_path_and_prefix);
        mkdir(save_dir_with_design,0777);
        if(design_file_extention_token != NULL) *design_file_extention_token = '.';
        free(save_dir_with_design);
    }
    wordfree(&p);

    free(design_dir);
    free(design_file);
    free(theme_dir);
    free(theme_file);
    free(save_dir);

    ///// [addons] /////
    char *ups_addon_path = g_key_file_get_string(keyfile,"addons","ups_addon_path",NULL);
    if(ups_addon_path && wordexp(ups_addon_path, &p, WRDE_UNDEF) == 0 && p.we_wordc == 1){
        asprintf(&config->ups_addon_path,"%s",p.we_wordv[0]);
    }
    wordfree(&p);
    free(ups_addon_path);

    ///// [advanced] /////
    double countdown_time = g_key_file_get_double(keyfile,"advanced","countdown_time",&error) + 0.5e-9;
    config->countdown_time.it_value.tv_sec = (long) countdown_time;
    config->countdown_time.it_value.tv_usec = (countdown_time - config->countdown_time.it_value.tv_sec ) * 1000000L;
    double preview_time = g_key_file_get_double(keyfile,"advanced","preview_time",&error) + 0.5e-9;;
    config->preview_time.it_value.tv_sec = (long) preview_time;
    config->preview_time.it_value.tv_usec = (preview_time - config->preview_time.it_value.tv_sec ) * 1000000L;
    config->mirror_liveview = g_key_file_get_boolean(keyfile,"advanced","mirror_liveview",NULL);
    config->mirror_preview = g_key_file_get_boolean(keyfile,"advanced","mirror_preview",NULL);
    config->printing_enabled = !g_key_file_get_boolean(keyfile,"advanced","disable_printing",NULL);
    config->windowless_mode = g_key_file_get_boolean(keyfile,"advanced","windowless_mode",NULL);

    g_key_file_free (keyfile);

    if(countdown_time == 0.5e-9) return 1;
    if(preview_time == 0.5e-9) return 1;
    return 0;
}

void free_config(config_t *config){
    if(config->printer_driver_name != NULL)   free(config->printer_driver_name);
    if(config->save_path_and_prefix != NULL)  free(config->save_path_and_prefix);
    if(config->ups_addon_path != NULL)        free(config->ups_addon_path);
    free_design(&config->design);
    free_theme(&config->theme);
}

int get_recent_file_in_dir(char **file, char *directory, char *file_extention)
{
    *file = NULL;
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if(d)
    {
        time_t most_recent = 0;
        while ((dir = readdir(d)) != NULL)
        {
            if(dir->d_type != DT_REG)continue;
            char file_path_buf[512]={0};
            struct stat file_stats = {0};
            strncpy(file_path_buf,directory,255);
            strncat(file_path_buf,dir->d_name,255);
            stat(file_path_buf,&file_stats);
            if( strstr(file_path_buf,file_extention) && file_stats.st_mtim.tv_sec > most_recent){
                most_recent = file_stats.st_mtim.tv_sec;
                if(*file != NULL) free(*file);
                asprintf(file,"%s",basename(file_path_buf));
            }          
        }
        closedir(d);
        if(file[0]==0){
            LOG("ERROR: \"%s\" does not contain a file ending in %s\n",directory,file_extention);
            return 1;
        }
    }else{
        LOG("ERROR: \"%s\" is not a valid directory.\n",directory);
        return 1;
    }
    return 0;
}

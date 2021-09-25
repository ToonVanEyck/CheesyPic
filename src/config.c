#include "config.h"

int read_config(cp_config_t *cp_config)
{
    GKeyFile *keyfile;
    GError *error = NULL;

    keyfile = g_key_file_new ();
    if (!g_key_file_load_from_file (keyfile, CONFIG_FILE, G_KEY_FILE_NONE, &error)){
        LOG("%s\n",error->message);
        return 1;
    }

    wordexp_t p;

    char *design_dir = g_key_file_get_string(keyfile,"design","design_directory",NULL);
    char *design_file = g_key_file_get_string(keyfile,"design","default_design",NULL);
    int use_latest_design = g_key_file_get_boolean(keyfile,"design","use_latest_design",&error);

    LOG("%s %s %d\n",design_dir,design_file,use_latest_design);

    if(design_dir && design_file && wordexp(design_dir, &p, WRDE_UNDEF) == 0 && p.we_wordc == 1){
        char *design_path;
        if(use_latest_design){
            get_recent_file_in_dir(&design_path,p.we_wordv[0],".design.svg");
        }else{
            asprintf(&design_path,"%s%s",p.we_wordv[0],design_file);
        }
        load_design_from_file(&cp_config->cp_design,design_path);
        free(design_path);
    }
    free(design_dir);
    free(design_file);
    wordfree(&p);


    // char *theme_dir  = g_key_file_get_string(keyfile,"theme","theme_directory",NULL);
    // char *theme_file  = g_key_file_get_string(keyfile,"theme","default_theme",NULL);
    // unsigned char use_latest_theme = g_key_file_get_boolean(keyfile,"theme","use_latest_theme",NULL);

    // char *capture_dir= g_key_file_get_string(keyfile,"capture","save_directory",NULL);
    // unsigned char save_photos = g_key_file_get_boolean(keyfile,"capture","save_photos",NULL);

    // double cowntdown_time = g_key_file_get_double(keyfile,"avanced","cowntdown_time",NULL);
    // double preview_time = g_key_file_get_double(keyfile,"avanced","preview_time",NULL);
    // unsigned char mirror_liveview = g_key_file_get_boolean(keyfile,"avanced","mirror_liveview",NULL);
    // unsigned char mirror_preview = g_key_file_get_boolean(keyfile,"avanced","mirror_preview",NULL);




    
    g_key_file_free (keyfile);
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
            char path[512]={0};
            struct stat file_stats = {0};
            strncpy(path,directory,255);
            strncat(path,dir->d_name,255);
            stat(path,&file_stats);
            if( strstr(path,file_extention) && file_stats.st_mtim.tv_sec > most_recent){
                most_recent = file_stats.st_mtim.tv_sec;
                if(*file != NULL) free(*file);
                asprintf(file,"%s",path);
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

int main(int argc, char *argv[])
{
    cp_config_t cp_config;
    memset(&cp_config,0,sizeof(cp_config_t));
    read_config(&cp_config); 
}

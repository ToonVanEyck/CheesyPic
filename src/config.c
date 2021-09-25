#include "config.h"

int read_config()
{
    GKeyFile *keyfile;
    GKeyFileFlags flags;
    GError *error = NULL;
    gsize length;
    char *design_path, *theme_path, **env;


    // Create a new GKeyFile object and a bitwise list of flags.
    keyfile = g_key_file_new ();
    flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

    // Load the GKeyFile from keyfile.conf or return.
    if (!g_key_file_load_from_file (keyfile, "../configuration/cheesypic.conf", flags, &error))
    {
        g_error (error->message);
        return -1;
    }
    design_path = g_key_file_get_string(keyfile,"desgin","design_directory",NULL);
    theme_path  = g_key_file_get_string(keyfile,"theme","theme_directory",NULL);

    printf("design_path: %s\n",design_path);
    printf("theme_path : %s\n",theme_path);

    wordexp_t p;
    char **w;

    int rc = wordexp(design_path, &p, WRDE_UNDEF);
    for (int i = 0; i < p.we_wordc; i++)
        printf("%d: %s\n",rc, p.we_wordv[i]);
    wordfree(&p);
    free(design_path);
    free(theme_path);
    g_key_file_free (keyfile);
}

int main(int argc, char *argv[])
{
   read_config(); 
   exit(1);
}
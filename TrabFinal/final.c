#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libtar.h>
#include <bzlib.h>
#include <unistd.h>

// struct dirent
// {
//     ino_t d_ino;             /* inode number */
//     off_t d_off;             /* not an offset */
//     unsigned short d_reclen; /* length of record */
//     unsigned char d_type;    /* type of file;
//               not supported by all filesystem types */
//     char d_name[256];        /* filename */
// };

DIR *opendir(const char *name);

struct dirent *readdir(DIR *dirp);

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

int closedir(DIR *dirp);

int mostra_dir(const char *nome_dir)
{
    DIR *dir_d;
    int finito, n_entradas;
    struct dirent *dir_entry;
    dir_d = opendir(nome_dir);
    if (dir_d == NULL)
    {
        fprintf(stderr, "erro:"
                        "impossível abrir DIR '%s' - %s\n",
                nome_dir, strerror(errno));
        return -1;
    }
    n_entradas = 0;
    finito = 0;

    do
    {
        dir_entry = readdir(dir_d);
        if (dir_entry == NULL)
        {
            if (errno)
            {
                fprintf(stderr, "erro:readdir (entrada %d)\n", n_entradas);
                closedir(dir_d);
                return -1;
            }
            else
            {
                printf("Iteração de DIR '%s' "
                       "terminada (%d entradas)\n",
                       nome_dir, n_entradas);
                finito = 1;
            }
        }
        else
        {
            printf("entrada: '%lu'\n", dir_entry->d_name);
            n_entradas++;
        }
    } while (finito == 0);

    if (closedir(dir_d) == -1)
    {
        fprintf(stderr, "erro: impossível fechar "
                        "DIR '%s' - %s\n",
                nome_dir, strerror(errno));
        return -1;
    }
    printf("DIR '%s': %d entradas\n",
           nome_dir, n_entradas);
    return 0;
}

int main()
{
    char *nome = ".";
    int retorno = mostra_dir(nome);
    printf("Retorno = %d", retorno);
}
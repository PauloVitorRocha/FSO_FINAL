#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
// #include <libtar.h>
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

char read_buffer[4096];
char write_buffer[4096];

DIR *opendir(const char *name);

struct dirent *readdir(DIR *dirp);

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

int closedir(DIR *dirp);

/* Devolve string descritiva da entrada st_mode. */
int devolve_tipo_entrada(mode_t st_mode)
{
    switch (st_mode & S_IFMT)
    {
    case S_IFDIR:
        return 15;
    case S_IFREG:
        return 10;
    default:
        return -5;
    }
    return 0; /* Nunca alcançado */
}
/* Mostra as entradas do diretório 'nome_dir' 
 * Devolve 0 se tudo ok, -1 em caso de erro */

void print_return(int response)
{
    switch (response)
    {
    case BZ_PARAM_ERROR:
        //printf("BZ_PARAM_ERROR\n");
        break;
    case BZ_SEQUENCE_ERROR:
        //printf("BZ_SEQUENCE_ERROR\n");
        break;
    case BZ_RUN_OK:
        //printf("BZ_RUN_OK\n");
        break;
    case BZ_FLUSH_OK:
        //printf("BZ_FLUSH_OK\n");
        break;
    case BZ_FINISH_OK:
        //printf("BZ_FINISH_OK\n");
        break;
    case BZ_STREAM_END:
        //printf("BZ_STREAM_END\n");
        break;
    default:
        //printf("ERRAO\n");
        break;
    }
}
void get_fname(char* destino, const char* path){
    int i=strlen(path)-1;
    char buffer[256];
    int j=0;
    while(i>=0 && path[i]!='/'){
        buffer[j++]=path[i--];
    }
    buffer[j]='\0';
    i=strlen(buffer)-1;
    j=0;
    while(i>=0){
        destino[j++]=buffer[i--];
    }
    destino[j]='\0';
}
void compress_bz2(const char *origem, const char *destino){
    char file_name[256];
    get_fname(file_name, origem);
    char out_path[256];
    strcpy(out_path, destino);
    strcat(out_path, file_name); 
    strcat(out_path, ".bz2");

    FILE *entrada;
    entrada = fopen(origem, "rb");
    FILE *saida;
    saida = fopen(out_path, "wb");

    //Configuração da compactação
    bz_stream strm;
    strm.bzalloc = NULL;
    strm.bzfree = NULL;
    strm.opaque = NULL;
    BZ2_bzCompressInit(&strm, 9, 0, 30);
    int action = BZ_RUN;
    int response = BZ_OK;
    char read_buffer[4096];
    char write_buffer[4096];

    do{
        strm.avail_in = fread(read_buffer, sizeof(char), sizeof(read_buffer), entrada);
        //printf("strm.avail_in = %u\n", strm.avail_in);
        if (feof(entrada) != 0){
            action = BZ_FINISH;
        }
        //printf("action=%d\n", action);
        strm.next_in = read_buffer;
        do
        {
            strm.avail_out = sizeof(write_buffer);
            strm.next_out = write_buffer;
            response = BZ2_bzCompress(&strm, action);
            //print_return(response);
            int written_bytes = sizeof(write_buffer) - strm.avail_out;
            fwrite(write_buffer, written_bytes, sizeof(char), saida);
            //printf("wrt buffer: %s\n", write_buffer);
            //printf("written_bytes: %d\n", written_bytes);
        } while ((strm.avail_out == 0) && (response != BZ_STREAM_END));
    } while (action != BZ_FINISH);
    BZ2_bzCompressEnd(&strm);
    fclose(entrada);
    fclose(saida);

}
int mostra_dir(const char *nome_dir, const char *out_dir)
{
    DIR *dir_d;
    int finito, n_entradas;
    struct dirent *dir_entry;
    char path[PATH_MAX];
    // //printf("PATHMAX = %d\n", PATH_MAX);
    size_t path_len = sizeof(path);
    dir_d = opendir(nome_dir);
    if (dir_d == NULL)
    {
        fprintf(stderr, "erro: impossível abrir"
                        "DIR '%s' - %s\n",
                nome_dir, strerror(errno));
        return -1;
    }
    n_entradas = 0;
    finito = 0;

    do{
        dir_entry = readdir(dir_d);
        if (dir_entry == NULL)
        {
            if (errno)
            {
                fprintf(stderr, "erro: readdir"
                                "(entrada %d)\n",
                        n_entradas);
                closedir(dir_d);
                return -1;
            }
            //printf("Iteração de DIR '%s' terminada "
//  "(%d entradas)\n",
//  nome_dir, n_entradas);
            finito = 1;
        }
        else{
            struct stat stat_buf;
            snprintf(path, path_len, "%s/%s",
                     nome_dir, dir_entry->d_name);
            if (stat(path, &stat_buf) == -1)
            {
                fprintf(stderr, "impossível stat"
                                " '%s':%s\n",
                        dir_entry->d_name,
                        strerror(errno));
                return -1;
            }
            n_entradas++;

            char file_path[256] = "";
            strcat(file_path, nome_dir);
            strcat(file_path, "/");
            strcat(file_path, dir_entry->d_name);

            // SE EH PASTA
            if (devolve_tipo_entrada(stat_buf.st_mode) == 15){
                int res = strcmp(dir_entry->d_name, "..");
                res *= strcmp(dir_entry->d_name, ".");
                // SE EH . ou ..
                if (res == 0)
                {
                    //printf("%s\n", dir_entry->d_name);
                    continue;
                }
                //printf("%s\n", dir_entry->d_name);
                mostra_dir(file_path, out_dir);
            }

            //SE EH ARQUIVO
            else{
                compress_bz2(file_path, out_dir);
            }
        }
    } while (finito == 0);
    if (closedir(dir_d) == -1)
    {
        fprintf(stderr, "erro: impossível fechar"
                        "DIR '%s' - %s\n",
                nome_dir, strerror(errno));
        return -1;
    }
    //printf("DIR '%s': %d entradas\n",
        //    nome_dir, n_entradas);
    return 0;
}

int main(int argc, char *argv[])
{
    char *path = argv[1];
    char *endPath = argv[2];
    char tmp[256];
    char tmp1[256] = "rm -rf ";

    strcpy(tmp, endPath);

    tmp[strlen(tmp)-1] = '.';
    strcat(tmp, "bz2/");
    strcat(tmp1, tmp);
    system(tmp1);
    printf("%s\n", tmp1);
    mkdir(tmp, 0700);

        int retorno = mostra_dir(path, tmp);
    //printf("Retorno = %d\n", retorno);
}
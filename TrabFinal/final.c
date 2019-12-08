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
typedef struct file{
    char source[256];
    char destiny[256];
}arq;

arq files[4096];
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
void compress_bz2(const char *origem, const char *destino){
    char out_path[256];
    strcpy(out_path, destino);
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
        if (feof(entrada) != 0)
        {
            action = BZ_FINISH;
        }
        //printf("action=%d\n", action);
        strm.next_in = read_buffer;
        do{
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
int mostra_dir(const char *nome_dir, const char *out_dir, int num_arq){
    DIR *dir_d;
    int finito, n_entradas;
    struct dirent *dir_entry;
    char path[PATH_MAX];
    //printf("PATHMAX = %d\n", PATH_MAX);
    size_t path_len = sizeof(path);
    dir_d = opendir(nome_dir);
    if (dir_d == NULL){
        fprintf(stderr, "erro: impossível abrir"
                        "DIR '%s' - %s\n",
                nome_dir, strerror(errno));
        exit(0);
    }
    n_entradas = 0;
    finito = 0;

    do{
        // Erro no arquivo, final do diretório
        dir_entry = readdir(dir_d);
        if (dir_entry == NULL){
            if (errno){
                fprintf(stderr, "erro: readdir"
                                "(entrada %d)\n",
                        n_entradas);
                closedir(dir_d);
                exit(0);
            }
            //printf("Iteração de DIR '%s' terminada "
            //  "(%d entradas)\n",
            //  nome_dir, n_entradas);
            finito = 1;
        }
        else{
            // Arquivo encontrado
            struct stat stat_buf;
            snprintf(path, path_len, "%s/%s",
                     nome_dir, dir_entry->d_name);
            if (stat(path, &stat_buf) == -1){
                fprintf(stderr, "impossível stat"
                                " '%s':%s\n",
                        dir_entry->d_name,
                        strerror(errno));
                exit(0);
            }
            n_entradas++;

            // Atualizando nome dos paths
            char source_path[256] = "";
            strcat(source_path, nome_dir);
            strcat(source_path, dir_entry->d_name);
            char destiny_path[256] = "";
            strcat(destiny_path, out_dir);
            strcat(destiny_path, dir_entry->d_name);

            // SE EH PASTA
            if (devolve_tipo_entrada(stat_buf.st_mode) == 15)
            {
                int res = strcmp(dir_entry->d_name, "..");
                res *= strcmp(dir_entry->d_name, ".");
                // printf("FOLDER = %s\n", dir_entry->d_name);
                // SE EH . ou ..
                if (res == 0)
                {
                    continue;
                }
                strcat(destiny_path, "/");
                strcat(source_path, "/");
                mkdir(destiny_path, 0700);
                num_arq=mostra_dir(source_path, destiny_path, num_arq);
            }

            //SE EH ARQUIVO
            else{
                //printf("Compress from %s to %s\n", source_path, destiny_path);
                // printf("FILE = %s\n", dir_entry->d_name);
                strcpy(files[num_arq].source, source_path);
                strcpy(files[num_arq].destiny, destiny_path);
                num_arq++;
                //compress_bz2(source_path, destiny_path);
            }
        }
    } while (finito == 0);
    if (closedir(dir_d) == -1){
        fprintf(stderr, "erro: impossível fechar"
                        "DIR '%s' - %s\n",
                nome_dir, strerror(errno));
        exit(0);
    }
    //printf("DIR '%s': %d entradas\n",
    //    nome_dir, n_entradas);
    return num_arq;
}

int main(int argc, char *argv[])
{
    char *path = argv[1];
    char *endPath = argv[2];
    char endDir[256];
    char rm[256] = "rm -rf ";
    char tar[256] = "tar -cf ";
    char tmp[256] = "";

    strcpy(endDir, endPath);
    strcpy(tmp, endPath);

    // strcat(endDir, ".bz2");
    // printf("endDir = %s\n", endDir);

    tmp[strlen(tmp) - 4] = '\0';
    // printf("TMP = %s\n", tmp);

        //comand TAR
    strcat(tar, endDir);
    strcat(tar, " ");
    strcat(tar, tmp);
    strcat(tar, " --remove-files");
    // printf("TAR = %s\n", tar);

    //Remove se ja existe pasta
    strcat(endDir, "/");
    strcat(path, "/");
    strcat(rm, tmp);
    system(rm);

    // printf("Comando = %s\n", rm);
    mkdir(tmp, 0700);

    strcat(tmp, "/");
    // while(valid==0);
    // sleep(1);
    // printf("endDir = %s\n", endDir);
    int num_arq = mostra_dir(path, tmp, 0);
    int i;
    printf("num_arq:%d\n", num_arq);
    for(i=0; i<num_arq; ++i){
        printf("struct %d: source:%s destiny:%s\n", i, files[i].source, files[i].destiny);
        compress_bz2(files[i].source, files[i].destiny);
    }
    FILE *a = popen(tar, "r");
    pclose(a);

    //printf("Retorno = %d\n", retorno);
}
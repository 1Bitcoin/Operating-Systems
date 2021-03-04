#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 

int dopath(const char *filename, int depth);

int main(int argc, char * argv[])
{
    int code_error = 0;
    if (argc == 2)
        code_error = dopath(argv[1], 0);
    else
    {
        printf("Use ./a.out <base_dir>\n");
        code_error = -1;
    }
    return code_error;
}

int dopath(const char *filename, int depth)
{
    int code_error = 0;

    struct stat statbuf;
    if (lstat(filename, &statbuf) < 0) // получаем структуру с информацией о файле
        return -1;

    for (int i = 0; i < depth; ++i)
        printf("|   ");

    if (S_ISDIR(statbuf.st_mode) == 0) // это не каталог?
    {
        printf("-- %s\n", filename);
        return 0;
    }

    printf("%s\n", filename);

    DIR *dp = opendir(filename); // открывает поток каталога, возв. указатель на этот поток

    if (dp == NULL) // каталог недоступен
        return -3;

    chdir(filename);

    struct dirent *dirp;

    while (!code_error && (dirp = readdir(dp)) != NULL) // NULL - достигли последнюю запись 
                                                        // возвращает указатель на след.запись в директории в структуре
    {
        if (strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, ".."))
            code_error = dopath(dirp->d_name, depth + 1);
    }
    chdir("..");

    if (closedir(dp) < 0)
    {
        perror("Can`t close dir");
        code_error = -4;
    }

    return code_error;
}
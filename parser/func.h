#ifndef _DB_FUNC_H_
#define _DB_FUNC_H_

#include <stdio.h>
#include <dirent.h>

void listDirectories()
{
    DIR *dp = opendir(".");
    if (NULL == dp)
    {
        perror("opendir");
        return ;
    }
    struct dirent * dirp;
    while ((dirp = readdir(dp)) != NULL)
        if (4 == dirp->d_type && strcmp(".", dirp->d_name)
                && strcmp("..", dirp->d_name))
        {
            printf("%s\n", dirp->d_name);
        }
    closedir(dp);
}

void listTables(const char * path)
{
    DIR *dp = opendir(path);
    if (NULL == dp)
    {
        perror("opendir");
        return ;
    }
    struct dirent * dirp;
    while ((dirp = readdir(dp)) != NULL)
        if (8 == dirp->d_type)
        {
            printf("%s\n", dirp->d_name);
        }
    closedir(dp);
}

#endif

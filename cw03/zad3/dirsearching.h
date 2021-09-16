#ifndef DIRSEARCHING_H
#define DIRSEARCHING_H
#define MAX_STRING_LENGTH 1000
#define MAX_FILENAME_LENGTH 1000

int isDirectory( struct dirent * file );
int search_file(char *filename, char * pattern);

#endif //DIRSEARCHING_H

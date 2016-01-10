#ifndef FILEDESC_H
#define FILEDESC_H

void openFile(char* filename, int oflag);

extern int g_fileDesc[100];
extern int g_currFileDesc;

#endif /* FILEDESC_H */

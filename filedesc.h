#ifndef FILEDESC_H
#define FILEDESC_H

void initFileDesc();
void endFileDesc();
int openFile(char* filename, int oflag);

extern int *g_fileDesc;
extern int g_currFileDesc;

#endif /* FILEDESC_H */

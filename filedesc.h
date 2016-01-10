#ifndef FILEDESC_H
#define FILEDESC_H

void openFile(char* filename, int oflag);

static int g_fileDesc[100];
static int g_currFileDesc = 0;

#endif /* FILEDESC_H */

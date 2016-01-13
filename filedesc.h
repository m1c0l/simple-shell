#ifndef FILEDESC_H
#define FILEDESC_H

typedef struct {
  int fd;
  int oflag;
} file;

void initFileDesc();
void endFileDesc();
int openFile(char* filename, int oflag);
file getFile(int index);

#endif /* FILEDESC_H */

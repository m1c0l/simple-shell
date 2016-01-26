#ifndef FILEDESC_H
#define FILEDESC_H

typedef struct {
  int fd;
  int readable;
  int writable;
} file;

void initFileDesc(void);
void endFileDesc(void);


int openFile(char* filename, int oflag);
int closeFile(char* fd_string);

void close_all_files(void);

file getFile(int index);


#endif /* FILEDESC_H */

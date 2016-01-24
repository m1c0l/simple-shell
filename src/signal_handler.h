#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

int getSignalNumber(char *str);

int ignoreSignal(char* optarg);
int useDefaultSignal(char* optarg);

int raiseAbortSignal(void);

#endif
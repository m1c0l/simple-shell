#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

int getSignalNumber(char *str);

int ignoreSignal(char* optarg);
int useDefaultSignal(char* optarg);

void signalHandlerInit(void);
void signalHandler(int signalNumber);
int catchSignal(char* optarg);

int raiseAbortSignal(void);

struct sigaction gSigAction;

#endif
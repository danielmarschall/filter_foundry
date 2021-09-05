extern FilterRecordPtr gpb;

#define PINEWHANDLE      gpb->handleProcs->newProc
#define PIDISPOSEHANDLE  gpb->handleProcs->disposeProc
#define PIGETHANDLESIZE  gpb->handleProcs->getSizeProc
#define PISETHANDLESIZE  gpb->handleProcs->setSizeProc
#define PILOCKHANDLE     gpb->handleProcs->lockProc
#define PIUNLOCKHANDLE   gpb->handleProcs->unlockProc

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

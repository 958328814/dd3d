#ifndef DD3D_H_INCLUDED
#define DD3D_H_INCLUDED

#include <cstdio>
#include <cstdlib>
#include <stdint.h>

#include <map>
#include <set>
#include <list>

extern int ConfigTestMode;
extern int ConfigMinVersion;
extern int ConfigEnableBetaLogin;
extern char Buffer[BUFSIZ];
extern uint8_t MsgBuffer[1024000];
extern volatile bool Terminate;

#endif

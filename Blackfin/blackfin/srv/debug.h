//
// Debug.h
//
#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG

#ifdef DEBUG
#define DEBUG_BUFFER_SIZE   12

int _dbgInfo[DEBUG_BUFFER_SIZE];

extern inline void debug_setDebugInfo(int dbgData, int index)
							{ if (index < DEBUG_BUFFER_SIZE && index >= 0) _dbgInfo[index] = dbgData; }
extern inline int* debug_getDebugInfo()
							{ return &_dbgInfo[0]; }

#endif

#endif


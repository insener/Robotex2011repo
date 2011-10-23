//
// Debug.h
//
#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG

#ifdef DEBUG
extern unsigned int _debugInfo;

extern inline void 		debug_setDebugInfo(unsigned int debug) { _debugInfo = debug; }
extern inline unsigned int debug_getDebugInfo() 			   { return _debugInfo; }

#endif

#endif


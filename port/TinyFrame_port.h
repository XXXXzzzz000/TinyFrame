#ifndef __TINYFRAME_PORT_H__
#define __TINYFRAME_PORT_H__
#include <TinyFrame.h>
#include <rtthread.h>
#ifdef PKG_TINYFRAME_USE_UART1
extern TinyFrame tf1;
#endif

#ifdef PKG_TINYFRAME_USE_UART2
extern TinyFrame tf2;
#endif

#ifdef PKG_TINYFRAME_USE_UART3
extern TinyFrame tf3;
#endif

#endif // !__TINYFRAME_PORT_H__
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
typedef struct
{
    rt_device_t serial;
    struct rt_semaphore rx_sem;
    char uart_name[PKG_TINYFRAME_UART_NAME_MAX_LEN];
    int enable_rx;
    int enable_rx_cb;
    void (*rx_cb)(uint8_t data);
} TinyFramUserData;
int TFEX_OpenReceive(TinyFrame *tf);
int TFEX_CloseReceive(TinyFrame *tf);
int TFEX_OpenReceiveCb(TinyFrame *tf);
int TFEX_CloseReceiveCb(TinyFrame *tf);
int TFEX_SetReceiveCb(TinyFrame *tf, void (*rx_cb)(uint8_t data));
#endif // !__TINYFRAME_PORT_H__
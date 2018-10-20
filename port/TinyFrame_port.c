// #include "../inc/TinyFrame.h"
#include "TinyFrame_port.h"
typedef struct
{
    rt_device_t serial;
    struct rt_semaphore rx_sem;
    char uart_name[PKG_TINYFRAME_UART_NAME_MAX_LEN];
} TinyFramUserData;

#ifdef PKG_TINYFRAME_USE_UART1
TinyFrame tf1;
TinyFramUserData tfu1;
static rt_err_t uart_input1(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&tfu1.rx_sem);

    return RT_EOK;
}
#endif

#ifdef PKG_TINYFRAME_USE_UART2
TinyFrame tf2;
TinyFramUserData tfu2;
static rt_err_t uart_input2(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&tfu2.rx_sem);

    return RT_EOK;
}
#endif

#ifdef PKG_TINYFRAME_USE_UART3
TinyFrame tf3;
TinyFramUserData tfu3;
static rt_err_t uart_input3(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&tfu3.rx_sem);

    return RT_EOK;
}
#endif

void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    TinyFramUserData *tfu = (TinyFramUserData *)tf->userdata;
    if (tfu->serial != RT_NULL)
    {
        rt_device_write(tfu->serial, 0, buff, len);
    }
    else
    {
        RT_ASSERT(0);
    }
}

static void tf_thread_entry(void *parameter)
{
    TinyFrame *tf = (TinyFrame *)parameter;
    TinyFramUserData *tfu = (TinyFramUserData *)tf->userdata;

    while (1)
    {
        char ch;
        while (rt_device_read(tfu->serial, -1, &ch, 1) != 1)
        {
            rt_sem_take(&tfu->rx_sem, RT_WAITING_FOREVER);
        }
        TF_Accept(tf, &ch, 1);
        TF_Tick(tf);
    }
}

static int _tf_uart_init(TinyFrame *tf, TinyFramUserData *tfu, char *uart_name, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size))
{
    rt_device_t serial = RT_NULL;
    static struct rt_semaphore *rx_sem = RT_NULL;

    /* init sem */
    rx_sem = &tfu->rx_sem;
    char buf[PKG_TINYFRAME_UART_NAME_MAX_LEN + 6] = {0};
    char sem_name[] = "_rx_sem";
    memcpy(buf, uart_name, strlen(uart_name) + 1);
    strcat(buf, sem_name);
    rt_sem_init((rt_sem_t)&rx_sem, buf, 0, RT_IPC_FLAG_FIFO);

    /* init uart */
    memcpy(tfu->uart_name, uart_name, strlen(uart_name) + 1);
    tfu->serial = rt_device_find(tfu->uart_name);
    serial = tfu->serial;
    if (serial == RT_NULL)
    {
        RT_ASSERT(0);
    }
    rt_device_open(serial, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);

    rt_device_set_rx_indicate(serial, rx_ind);

    tf->userdata = tfu;
    /* creat thread */
    rt_thread_t thread = rt_thread_create("serial", tf_thread_entry, tf, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    return 0;
}

int tf_uart_init(void)
{
#ifdef PKG_TINYFRAME_USE_UART1
    _tf_uart_init(&tf1, &tfu1, "uart1", uart_input1);
#endif
#ifdef PKG_TINYFRAME_USE_UART2
    _tf_uart_init(&tf2, &tfu2, "uart2", uart_input2);
#endif
#ifdef PKG_TINYFRAME_USE_UART3
    _tf_uart_init(&tf3, &tfu3, "uart3", uart_input3);
#endif
}
INIT_APP_EXPORT(tf_uart_init);

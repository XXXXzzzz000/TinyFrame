#include "uart3_txrx.h"
#include "../utils/utils.h"
/**
 * 发送接收示例
 */
TinyFrame *demo_tf;


/** 一个监听器函数的示例 */
TF_Result myListener(TinyFrame *tf, TF_Msg *msg)
{
    dumpFrameInfo(msg);
    return TF_STAY;
}

TF_Result testIdListener(TinyFrame *tf, TF_Msg *msg)
{
    rt_kprintf("OK - ID Listener triggered for msg!\n");
    dumpFrameInfo(msg);
    return TF_CLOSE;
}
int tf_uart3_txrx(int argc, char **argv)
{
    TF_Msg msg;
    const char *longstr = "Lorem ipsum dolor sit amet.";

    //设置TinyFrame库
    // demo_tf = TF_Init(TF_MASTER); // 1 = master, 0 = slave

    demo_tf = &tf3;
    TF_InitStatic(demo_tf, TF_MASTER);
    // 添加lintener
    TF_AddGenericListener(demo_tf, myListener);

    rt_kprintf("------ Simulate sending a message --------\n");

    TF_ClearMsg(&msg);
    msg.type = 0x22;
    msg.data = (pu8) "Hello TinyFrame";
    msg.len = 16;
    TF_Send(demo_tf, &msg);
    rt_thread_delay(1 * RT_TICK_PER_SECOND);

    msg.type = 0x33;
    msg.data = (pu8)longstr;
    msg.len = (TF_LEN)(strlen(longstr) + 1); // 添加null类型
    TF_Send(demo_tf, &msg);
    rt_thread_delay(1 * RT_TICK_PER_SECOND);

    msg.type = 0x44;
    msg.data = (pu8) "Hello2";
    msg.len = 7;
    TF_Send(demo_tf, &msg);
    rt_thread_delay(1 * RT_TICK_PER_SECOND);

    msg.len = 0;
    msg.type = 0x77;
    TF_Query(demo_tf, &msg, testIdListener, 0);
    rt_thread_delay(1 * RT_TICK_PER_SECOND);
}

MSH_CMD_EXPORT(tf_uart3_txrx, tinyfram uart3 test);

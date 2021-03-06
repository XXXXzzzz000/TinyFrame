//
// Created by MightyPork on 2017/10/15.
//

#include "../demo.h"
#include <stdio.h>
/* 你好监听器 */
TF_Result helloListener(TinyFrame *tf, TF_Msg *msg)
{
    printf("helloListener()\n");
    dumpFrameInfo(msg);
    return TF_STAY;
}
/* rsp监听器 */
TF_Result replyListener(TinyFrame *tf, TF_Msg *msg)
{
    printf("replyListener()\n");
    dumpFrameInfo(msg);
    msg->data = (const uint8_t *)"response to query";
    msg->len = (TF_LEN)strlen((const char *)msg->data);
    TF_Respond(tf, msg);

    //未经请求的回复 - 将不会由已经离开的ID侦听器处理
    msg->data = (const uint8_t *)"SPAM";
    msg->len = 5;
    TF_Respond(tf, msg);

    //无关的消息
    TF_SendSimple(tf, 77, (const uint8_t *)"NAZDAR", 7);
    return TF_STAY;
}

int main(void)
{
    demo_tf = TF_Init(TF_SLAVE);
    TF_AddTypeListener(demo_tf, 1, helloListener);
    TF_AddTypeListener(demo_tf, 2, replyListener);

    demo_init(TF_SLAVE);
    demo_sleep();
}

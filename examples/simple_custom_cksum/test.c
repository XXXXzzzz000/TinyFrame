#include <stdio.h>
#include <string.h>
#include "../../TinyFrame.h"
#include "../utils.h"

/**
 * 自定义校验和示例
 */
TinyFrame *demo_tf;

bool do_corrupt = false;

/**
 * 此功能应在应用程序代码中定义。
 * 它实现了最低层 - 发送字节到 UART（或其他）
 */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    printf("--------------------\n");
    printf("\033[32mTF_WriteImpl - sending frame:\033[0m\n");
    
    uint8_t *xbuff = (uint8_t *)buff;    
    if (do_corrupt) {
      printf("(corrupting to test checksum checking...)\n");
      xbuff[8]++;
    }
    
    dumpFrame(xbuff, len);

    // 将它发回，就好像我们收到了它
    TF_Accept(tf, xbuff, len);
}

/** 一个监听器函数的示例 */
TF_Result myListener(TinyFrame *tf, TF_Msg *msg)
{
    dumpFrameInfo(msg);
    return TF_STAY;
}

TF_Result testIdListener(TinyFrame *tf, TF_Msg *msg)
{
    printf("OK - ID Listener triggered for msg!\n");
    dumpFrameInfo(msg);
    return TF_CLOSE;
}

void main(void)
{
    TF_Msg msg;
    const char *longstr = "Lorem ipsum dolor sit amet.";

        //设置TinyFrame库
    demo_tf = TF_Init(TF_MASTER); // 1 = master, 0 = slave
    TF_AddGenericListener(demo_tf, myListener);

    printf("------ Simulate sending a message --------\n");

    TF_ClearMsg(&msg);
    msg.type = 0x22;
    msg.data = (pu8) "Hello TinyFrame";
    msg.len = 16;
    TF_Send(demo_tf, &msg);
    
    printf("This should fail:\n");
    
    // test checksums are tested
    do_corrupt = true;    
    msg.type = 0x44;
    msg.data = (pu8) "Hello2";
    msg.len = 7;
    TF_Send(demo_tf, &msg);
}


// a made up custom checksum - just to test it's used and works

TF_CKSUM TF_CksumStart(void)
{
    return 0;
}

TF_CKSUM TF_CksumAdd(TF_CKSUM cksum, uint8_t byte)
{
    return cksum ^ byte + 1;
}

TF_CKSUM TF_CksumEnd(TF_CKSUM cksum)
{
    return ~cksum ^ 0xA5;
}

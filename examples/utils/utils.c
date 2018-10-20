//
// Created by MightyPork on 2017/10/15.
//

#include "utils.h"
// #include <stdio.h>
// #include <rtthread.h>

// helper func for testing
void dumpFrame(const uint8_t *buff, size_t len)
{

    size_t i;
    for (i = 0; i < len; i++)
    {
        rt_kprintf("%3u \033[94m%02X\033[0m", buff[i], buff[i]);
        if (buff[i] >= 0x20 && buff[i] < 127)
        {
            rt_kprintf(" %c", buff[i]);
        }
        else
        {
            rt_kprintf(" \033[31m.\033[0m");
        }
        rt_kprintf("\n");
    }
    rt_kprintf("--- end of frame ---\n\n");
}

void dumpFrameInfo(TF_Msg *msg)
{
    rt_kprintf("\033[33mFrame info\n"
               "  type: %02Xh\n"
               "  data: \"%.*s\"\n"
               "   len: %u\n"
               "    id: %Xh\033[0m\n\n",
               msg->type, msg->len, msg->data, msg->len, msg->frame_id);
}

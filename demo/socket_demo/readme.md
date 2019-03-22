# 测试说明
测试文件中 slave作为服务器接受信息,masrter作为客户端发送信息
由代码可知整体测试步骤如下:
```
    master:发送普通消息
    slave:接收普通消息->调用普通hello回调
    
    master:发送普通消息
    slave:接收普通消息->调用普通hello回调

    master:发送query信息并注册rsp回调
    slave:接收query信息->调用reply回调->reply内发送rsp消息1
    master:正常调用rsp回调
    slave:reply内发送rsp消息2
    master:回复消息非法,调用通用回调
    slave:reply内发送普通消息3
    master:正常调用通用回调
  
```
由此可以发现:
1. 发送的rsp会调用相应的rsp回调
2. 非法的rsp不会调用rsp回调但依旧会视作一条普通信息
3. 消息的接收回调根据其类型区分
   ```c
bool TF_AddIdListener(TinyFrame *tf, TF_Msg *msg, TF_Listener cb, TF_TICKS timeout);
bool TF_AddTypeListener(TinyFrame *tf, TF_TYPE frame_type, TF_Listener cb);
bool TF_AddGenericListener(TinyFrame *tf, TF_Listener cb);


   ```
## slave
```
--- STARTING SERVER! ---

Waiting for client...

Client connected
--- RX 14 bytes ---
1. 接收 
    TF_SendSimple(demo_tf, 1, (pu8) "Ahoj", 5);
--- end of frame ---

helloListener()
Frame info
  type: 01h
  data: "Ahoj"
   len: 5
    id: 80h

--- RX 15 bytes ---
2. 接收  
    TF_SendSimple(demo_tf, 1, (pu8) "Hello", 6);
--- end of frame ---

helloListener()
Frame info
  type: 01h
  data: "Hello"
   len: 6
    id: 81h

--- RX 15 bytes ---
3. 接收 
    TF_QuerySimple(demo_tf, 2, (pu8) "Query!", 6, testIdListener, 0);
--- end of frame ---

replyListener()
Frame info
  type: 02h
  data: "Query!"
   len: 6
    id: 82h

TF_WriteImpl - sending frame:
4. 发送回复1
    msg->data = (const uint8_t *)"response to query";
    msg->len = (TF_LEN)strlen((const char *)msg->data);
    TF_Respond(tf, msg);
--- end of frame ---

TF_WriteImpl - sending frame:
5. 发送回复2
    msg->data = (const uint8_t *)"SPAM";
    msg->len = 5;
    TF_Respond(tf, msg);
--- end of frame ---

TF_WriteImpl - sending frame:
6. 发送无关消息
    TF_SendSimple(tf, 77, (const uint8_t *)"NAZDAR", 7);
--- end of frame ---

```

## master
```
Starting MASTER...
Thread started
TF_WriteImpl - sending frame:
1. 发送 
    TF_SendSimple(demo_tf, 1, (pu8) "Ahoj", 5);
--- end of frame ---


--- STARTING CLIENT! ---

 Child Process 
TF_WriteImpl - sending frame:
2. 发送 
    TF_SendSimple(demo_tf, 1, (pu8) "Hello", 6);
--- end of frame ---

TF_WriteImpl - sending frame:
3. 发送并注册回复监听
    TF_QuerySimple(demo_tf, 2, (pu8) "Query!", 6, testIdListener, 0);
--- end of frame ---

--- RX 26 bytes ---
4. 接收回复(调用专用监听)
    msg->data = (const uint8_t *)"response to query";
    msg->len = (TF_LEN)strlen((const char *)msg->data);
    TF_Respond(tf, msg);
--- end of frame ---

testIdListener()
Frame info
  type: 02h
  data: "response to query"
   len: 17
    id: 82h

--- RX 14 bytes ---
5. 接收无效的回复(调用通用监听)
    msg->data = (const uint8_t *)"SPAM";
    msg->len = 5;
    TF_Respond(tf, msg);
--- end of frame ---

testGenericListener()
Frame info
  type: 02h
  data: "SPAM"
   len: 5
    id: 82h

--- RX 16 bytes ---
6. 接收无关信息(调用通用监听)
    TF_SendSimple(tf, 77, (const uint8_t *)"NAZDAR", 7);

--- end of frame ---

testGenericListener()
Frame info
  type: 4Dh
  data: "NAZDAR"
   len: 7
    id: 0h



```
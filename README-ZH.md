# TinyFrame

TinyFrame是一个简单的库，用于构建和分析通过串行接口（例如UART，Telnet，套接字）发送的数据帧。代码被编写为与 `--std=gnu99`大多数兼容`--std=gnu89`。

该库提供了一个高层次的接口，用于在两个对等体之间传递消息。多消息会话，响应监听器，校验和，超时都由库处理。

TinyFrame适用于广泛的应用，包括微控制器之间的通信，作为基于FTDI的PC应用程序的协议或通过UDP数据包进行消息传递的协议。

该库允许您注册监听器（回调函数）以等待（1）任何帧，（2）特定帧类型，或（3）特定的消息ID。这个高级API足以实现大多数通信模式。

TinyFrame是可重入的，并支持创建多个实例，但其结构（字段大小和校验和类型）是相同的。有一个使用互斥体向共享实例添加多线程访问的支持。

TinyFrame还带有（可选）辅助函数，用于构建和分析消息有效载荷，这些都在`utils/`文件夹中提供。

## Ports

TinyFrame已经被移植到多种语言:

- The reference C implementation is in this repo
- Python port - [MightyPork/PonyFrame](https://github.com/MightyPork/PonyFrame)
- Rust port - [cpsdqs/tinyframe-rs](https://github.com/cpsdqs/tinyframe-rs)
- JavaScript port - [cpsdqs/tinyframe-js](https://github.com/cpsdqs/tinyframe-js)

请注意，大多数ports是实验性的，可能会出现各种错误或缺少的功能。测试人员欢迎:)

## Functional overview

TinyFrame的基本功能在这里解释。对于特殊的API函数，建议阅读头文件中的doc注释。

### Structure of a frame

每个帧由一个头部和一个有效载荷组成。这两个部分都可以通过校验和来保护，确保帧格式不正确（例如，长度字段损坏）或者被破坏的有效载荷被拒绝。

帧头包含一个帧ID和一个消息类型。每个新消息都会增加帧ID。ID字段的最高位固定为1和0，避免冲突。

帧ID可以在响应中重新使用以将两个消息结合在一起。类型字段的值是用户定义的。

框架中的所有字段都具有可配置的大小。通过在配置文件中改变字段，如`TF_LEN_BYTES`（1，2或4），库无缝之间切换`uint8_t`， `uint16_t`并`uint32_t`用于与所述领域工作的所有功能。

```
,-----+-----+-----+------+------------+- - - -+-------------,
| SOF | ID  | LEN | TYPE | HEAD_CKSUM | DATA  | DATA_CKSUM  |
| 0-1 | 1-4 | 1-4 | 1-4  | 0-4        | ...   | 0-4         | <- size (bytes)
'-----+-----+-----+------+------------+- - - -+-------------'

SOF ......... start of frame, usually 0x01 (optional, configurable)
ID  ......... the frame ID (MSb is the peer bit)
LEN ......... number of data bytes in the frame
TYPE ........ message type (used to run Type Listeners, pick any values you like)
HEAD_CKSUM .. header checksum

DATA ........ LEN bytes of data
DATA_CKSUM .. data checksum (left out if LEN is 0)
```

### Message listeners

TinyFrame基于消息监听器的概念。监听器是一个回调函数，等待一个特定的消息类型或ID被接收。

有3种监听器类型，按优先级顺序排列：
 
- **ID listeners** - 等待响应
- **Type listeners** - 等待给定类型字段的消息
- **Generic listeners** - 后备

ID 监听器可以在发送消息时自动注册。所有监听器也可以手动注册和删除。

ID监听器用于接收对请求的响应。当注册一个ID监听器时，可以将自定义用户数据附加到监听器回调中。这个data（`void *`）可以是任何类型的应用程序上下文变量。

ID监听器可以被分配超时。当监听器到期时，在删除之前，为了让用户`free()`附加任何数据，使用NULL有效载荷数据触发回调。只有当用户数据不是NULL时才会发生这种情况。

Listener回调函数返回`TF_Result`枚举的值：

- `TF_CLOSE` -  接受消息，删除监听器
- `TF_STAY` - 接受消息，保持监听器
- `TF_RENEW` - 与此相同`TF_STAY`，但ID监听器的超时被更新
- `TF_NEXT` - 不接受消息，保持监听器并将消息传递给下一个能够处理它的监听器。

### Data buffers, multi-part frames

TinyFrame使用两个数据缓冲区：一个小的发送缓冲区和一个较大的接收缓冲区。发送缓冲区用于准备一次发送的字节，或者如果缓冲区不够大，则以循环方式发送。缓冲区只能包含整个帧头，所以例如32个字节应该足够用于短消息。

使用`*_Multipart()`发送功能，还可以将帧头和有效载荷分成多个函数调用，从而允许应用程序即时生成有效载荷。

与发送缓冲区相反，接收缓冲区必须足够大以包含整个帧。这是因为在处理帧之前必须验证最终的校验和。

如果需要大于可能的接收缓冲区大小的帧（例如，在RAM较小的嵌入式系统中），建议在较高级别实现多消息传输机制，并以块为单位发送数据。

## Usage Hints

- 所有TinyFrame函数，typedefs和宏都以`TF_`前缀开始。
- 两个对等体都必须包含具有相同配置参数的库
- 请参阅`TF_Integration.example.c`并`TF_Config.example.c`参考如何配置和集成库。
- 如果可能的话，不要修改库文件。这使得升级变得容易。
- 通过调用开始`TF_Init()`使用`TF_MASTER`或`TF_SLAVE`作为参数。这创建一个句柄。使用`TF_InitStatic()`避免了使用`malloc（）`。
- 如果使用多个实例，则可以使用`tf.userdata`/ `tf.usertag`字段对它们进行标记。
- 实现`TF_WriteImpl()`- 在头文件底部声明为`extern`。这个函数被`TF_Send()`其他人用来写字节到你的UART（或其他物理层）。帧可以全部发送，也可以发送多个帧，具体取决于其大小。
- 如果您想使用超时，请定期调用`TF_Tick()`。通话周期决定了一个刻度的长度。这被用于在分析器陷入不良状态（例如接收到部分帧）时超时解析器，并且也可以超时ID侦听器。
- 绑定类型或通用侦听器使用`TF_AddTypeListener()`或`TF_AddGenericListener()`。
- 通过发送一个消息`TF_Send()`，`TF_Query()`，`TF_SendSimple()`，`TF_QuerySimple()`。查询函数需要一个侦听器回调（函数指针），它将作为ID侦听器添加并等待响应。
- `*_Multipart()`在多个函数调用中使用上述发送函数的变体来生成有效载荷。之后通过调用发送有效载荷，`TF_Multipart_Payload()` 并关闭帧`TF_Multipart_Close()`。
- 如果需要自定义校验和实现，请选择`TF_CKSUM_CUSTOM8`16或32，然后执行三个校验和功能。
- 要回复消息（当你的监听器被调用时），使用`TF_Respond()` 你收到的msg对象，用响应代替`data`指针（和`len`）。
- 在任何时候，您都可以使用手动重置消息解析器`TF_ResetParser()`。它也可以在配置文件中配置超时后自动重置。

### Gotchas to look out for

- 如果有任何用户数据附加到超时的ID侦听器，那么当侦听器超时时，它将被调用NULL `msg->data`来让用户释放用户数据。因此`msg->data`在继续处理消息之前需要检查。
- 如果正在发送多部分帧，则库的Tx部分被锁定以防止并发访问。在尝试发送其他内容之前，帧必须完全发送和关闭。
- 如果使用多线程，不要忘记实现互斥量回调以避免并发访问Tx功能。默认实现并不完全是线程安全的，因为它不能依赖特定于平台的资源，如互斥锁或原子访问。设置`TF_USE_MUTEX`于1在配置文件中。

### Examples

您会在`demo/`文件夹中找到各种示例。每个示例都有它自己的Makefile，阅读它以查看可用的选项。

演示是为Linux编写的，有些使用套接字和`clone()`后台处理。他们尝试在异步Rx和Tx的嵌入式系统中模拟真实的TinyFrame行为。如果你不能运行演示，那么源文件仍然是很好的例子。

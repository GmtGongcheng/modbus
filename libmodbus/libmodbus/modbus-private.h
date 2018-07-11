/*
 * Copyright © 2010-2012 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_PRIVATE_H
#define MODBUS_PRIVATE_H

#ifndef _MSC_VER
# include <stdint.h>
# include <sys/time.h>
#else
# include "stdint.h"
# include <time.h>
typedef int ssize_t;
#endif
#include <sys/types.h>
#include <config.h>

#include "modbus.h"

MODBUS_BEGIN_DECLS

/* It's not really the minimal length (the real one is report slave ID
 * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
 * communications to read many values or write a single one.
 * Maximum between :
 * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
 * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
 */
#define _MIN_REQ_LENGTH 12

#define _REPORT_SLAVE_ID 180

#define _MODBUS_EXCEPTION_RSP_LENGTH 5

/* Timeouts in microsecond (0.5 s) */
#define _RESPONSE_TIMEOUT    500000
#define _BYTE_TIMEOUT        500000

typedef enum {
    _MODBUS_BACKEND_TYPE_RTU=0,
    _MODBUS_BACKEND_TYPE_TCP
} modbus_backend_type_t;

/*
 *  ---------- Request     Indication ----------
 *  | Client | ---------------------->| Server |
 *  ---------- Confirmation  Response ----------
 */
typedef enum {
    /* Request message on the server side */
    MSG_INDICATION,
    /* Request message on the client side */
    MSG_CONFIRMATION
} msg_type_t;

/* This structure reduces the number of params in functions and so
 * optimizes the speed of execution (~ 37%). */
typedef struct _sft {
    int slave;
    int function;
    int t_id;
} sft_t;

typedef struct _modbus_backend {

	/*
	取值包含两种，分别是_MODBUS_BACKEND_TYPE_RTU
	和 _MODBUS_BACKEND_TYPE_TCP,用于指明处理类型
	*/
    unsigned int backend_type;
	
	/*
	消息头长度，在RTU模式下长度为_MODBUS_RTU_HEADER_LENGTH=1,
	即 ADDRESS字段长度；在TCP模式下取值为_MODBUS_TCP_HEADER_LENGTH=7,
	即MBAP字段长度
	*/
    unsigned int header_length;

	/*
	错误校验字段长度，在RTU模式下为_MODBUS_RTU_CHECKSUM_LENGTH = 2;
	在TCP模式下取值为_MODBUS_TCP_CKECKSUM_LENGTH = 0，无错误校验字段。
	*/
    unsigned int checksum_length;

	/*
	ADU最大长度。RTU模式下为MODBUS_RTU_MAX_ADU_LENGTH = 256，即RTU下
	MODBUS ADU = 253 bytes + slave(1 byte) + CRC(2 bytes) = 256 bytes。
	TCP模式下为MODBUS_TCP_MAX_ADU_LENGTH = 256,即TCP下
	MODBUS ADU = 253 bytes + MBAP(7 bytes) = 260 bytes。
	*/
    unsigned int max_adu_length;
    int (*set_slave) (modbus_t *ctx, int slave);  //设置从站设备地址

	/*
	  build_request_basic():此函数构造查询报文的基本通行帧。
	  在RTU模式下，其指向为文件modbus-rtu.c中的函数_modbus_rtu_build_request_basis(),
	  从函数中得知，通信帧的基本长度为_MODBUS_RTU_PRESET_REQ_LENGTH = 6 个字节；
	  在TCP模式下，其指向为文件modbus-tcp.c中的函数_modbus_tcp_build_request_basis(),
	  从函数中得知，通信帧的基本长度为_MODBUS_TCP_PRESET_REQ_LENGTH = 12 个字节。
	  */
    int (*build_request_basis) (modbus_t *ctx, int function, int addr,
                                int nb, uint8_t *req);  

	/*
	此函数功能为构造响应报文的基本通信帧。
    在RTU模式下，其指向为文件modbus-rtu.c中的函数_modbus_rtu_build_response_basis(),
	从函数中得知，通信帧的基本长度为_MODBUS_RTU_PRESET_RSP_LENGTH = 2 个字节；
	在TCP模式下，其指向为文件modbus-tcp.c中的函数_modbus_tcp_build_response_basis(),
	从函数中得知，通信帧的基本长度为_MODBUS_TCP_PRESET_RSP_LENGTH = 8 个字节。
	*/
    int (*build_response_basis) (sft_t *sft, uint8_t *rsp);

	/*
	此函数功能为构造响应报文TID参数。
	即构造响应报文MBAP中的Transaction Identifier 字段，仅在TCP模式下有效。
	*/
    int (*prepare_response_tid) (const uint8_t *req, int *req_length);

	/*
	此函数功能为发送报文前的预处理。
	在RTU模式下为CRC错误校验码的计算；在TCP模式下，设置MBAP中的Length字段的内容。
	*/
    int (*send_msg_pre) (uint8_t *req, int req_length);

	/*
	此函数功能为通过物理层发送报文。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_send();
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_send()。
	*/
    ssize_t (*send) (modbus_t *ctx, const uint8_t *req, int req_length);

	/*
	此函数功能用于接收报文。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_receive();
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_receive()。
	而_modbus_rtu_receive()和_modbus_tcp_receive()此两个函数最终又调用文件
	modbus.c中的函数_modbus_receive_msg()。_modbus_receive_msg()函数一次调用select()
	函数、recv()函数，用于读取通道数据，最后调用函数check_integrity()用以监测数据的完整性。
	*/
    int (*receive) (modbus_t *ctx, uint8_t *req);

	/*
	此函数功能为通过物理层读取报文，recv()函数被 receive()函数调用。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_recv();
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_recv()。
	*/
    ssize_t (*recv) (modbus_t *ctx, uint8_t *rsp, int rsp_length);

	/*
	此函数用于数据完整性检查。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_check_integrity()，在函数
	中通过计算CRC16的值，进行比较从而判断接收的数据是否是完整。
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_check_integrity()。因为在
	TCP模式下不需要进行CRC校验，因此直接返回消息长度。
	*/
    int (*check_integrity) (modbus_t *ctx, uint8_t *msg,
                            const int msg_length);

	/*
	此函数用于确认响应报文的帧头是否一致。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_pre_check_confirmation();
	    主要确认查询报文和响应报文中的从站地址是否一致。
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_pre_check_confirmation()。
	    该函数用于确认查询报文和响应报文的Transaction ID 和 Protocol ID 是否一致。
	*/
    int (*pre_check_confirmation) (modbus_t *ctx, const uint8_t *req,
                                   const uint8_t *rsp, int rsp_length);

	/*
	此函数用于主站设备和从站设备建立连接。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_connect()，
	    在函数中通过设置串口参数，并打开串口。
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_connect()或者_modbus_tcp_pi_connect()。
	*/
    int (*connect) (modbus_t *ctx);
    void (*close) (modbus_t *ctx);    //关闭主站设备与从站设备建立的连接

	/*
	内部函数，此函数用于关闭主站设备与从站设备建立的连接。
	在RTU模式下，其指向为文件modbus-tcp.c 中的函数_modbus_rtu_flush();
	在TCP模式下，其指向为文件modbus-tcp.c 中的函数_modubs_tcp_flush()。
	*/
    int (*flush) (modbus_t *ctx);

	/*
	内部函数，此函数用于设置超时并读取通信事件，
	以检测是否存在待接收数据。
	*/
    int (*select) (modbus_t *ctx, fd_set *rset, struct timeval *tv, int msg_length);
    void (*free) (modbus_t *ctx);  //此函数用于释放相关联的内存，防止内存泄漏
} modbus_backend_t;

struct _modbus {
    /* Slave address */
    int slave;                              //从站设备地址
    /* Socket or file descriptor */         
    int s;                                  //TCP模式下为Socket套接字；RTU下为串口句柄
    int debug;                              //是否启用Debug模式
    int error_recovery;                     //错误恢复模式
    struct timeval response_timeout;        //响应超时设置
    struct timeval byte_timeout;            //字节超时设置
    struct timeval indication_timeout;
    const modbus_backend_t *backend;      //包含一系列共通函数指针，如消息发送、接收等，试用TCP、RTU两种模式
    void *backend_data;                //上面共通部分之外的数据，如TCP模式下的特殊配置数据，或者RTU模式下的特殊配置数据
};

void _modbus_init_common(modbus_t *ctx);
void _error_print(modbus_t *ctx, const char *context);
int _modbus_receive_msg(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type);

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dest, const char *src, size_t dest_size);
#endif

MODBUS_END_DECLS

#endif  /* MODBUS_PRIVATE_H */

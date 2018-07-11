/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_H
#define MODBUS_H

/* Add this for macros that defined unix flavor */
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif

#include "modbus-version.h"

#if defined(_MSC_VER)
# if defined(DLLBUILD)
/* define DLLBUILD when building the DLL */
#  define MODBUS_API __declspec(dllexport)
# else
#  define MODBUS_API __declspec(dllimport)
# endif
#else
# define MODBUS_API
#endif

#ifdef  __cplusplus
# define MODBUS_BEGIN_DECLS  extern "C" {
# define MODBUS_END_DECLS    }
#else
# define MODBUS_BEGIN_DECLS
# define MODBUS_END_DECLS
#endif

MODBUS_BEGIN_DECLS

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

/* Modbus function codes */
#define MODBUS_FC_READ_COILS                0x01  //读线圈寄存器
#define MODBUS_FC_READ_DISCRETE_INPUTS      0x02  //读离散输出寄存器
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03  //读保持寄存器
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04  //读输入寄存器
#define MODBUS_FC_WRITE_SINGLE_COIL         0x05  //写单个线圈寄存器
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06  //写单个保持寄存器
#define MODBUS_FC_READ_EXCEPTION_STATUS     0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0F  //写多个线圈寄存器  
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10  //写多个保持寄存器
#define MODBUS_FC_REPORT_SLAVE_ID           0x11
#define MODBUS_FC_MASK_WRITE_REGISTER       0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS  0x17

#define MODBUS_BROADCAST_ADDRESS    0

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_WR_WRITE_REGISTERS      121
#define MODBUS_MAX_WR_READ_REGISTERS       125

/* The size of the MODBUS PDU is limited by the size constraint inherited from
 * the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
 * bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
 * address (1 byte) - CRC (2 bytes) = 253 bytes.
 */
#define MODBUS_MAX_PDU_LENGTH              253

/* Consequently:
 * - RTU MODBUS ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256
 *   bytes.
 * - TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes.
 * so the maximum of both backend in 260 bytes. This size can used to allocate
 * an array of bytes to store responses and it will be compatible with the two
 * backends.
 */
#define MODBUS_MAX_ADU_LENGTH              260

/* Random number to avoid errno conflicts */
#define MODBUS_ENOBASE 112345678

/* Protocol exceptions */
enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,  //非法的功能码
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,     //非法的数据地址
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,       //非法的数据值   
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,  //从站设备故障
    MODBUS_EXCEPTION_ACKNOWLEDGE,              //ACK异常
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,     //从站设备忙
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,     //否定应答 
    MODBUS_EXCEPTION_MEMORY_PARITY,            //内存奇偶校验错误
    MODBUS_EXCEPTION_NOT_DEFINED,              //未定义  
    MODBUS_EXCEPTION_GATEWAY_PATH,             //网关路径不可用
    MODBUS_EXCEPTION_GATEWAY_TARGET,           //目标设备未能回应
    MODBUS_EXCEPTION_MAX
};

#define EMBXILFUN  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_FUNCTION)
#define EMBXILADD  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS)
#define EMBXILVAL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE)
#define EMBXSFAIL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE)
#define EMBXACK    (MODBUS_ENOBASE + MODBUS_EXCEPTION_ACKNOWLEDGE)
#define EMBXSBUSY  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY)
#define EMBXNACK   (MODBUS_ENOBASE + MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE)
#define EMBXMEMPAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_MEMORY_PARITY)
#define EMBXGPATH  (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_PATH)
#define EMBXGTAR   (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_TARGET)

/* Native libmodbus error codes */
#define EMBBADCRC  (EMBXGTAR + 1)      //无效的CRC
#define EMBBADDATA (EMBXGTAR + 2)      //无效的数据  
#define EMBBADEXC  (EMBXGTAR + 3)      //无效的异常码
#define EMBUNKEXC  (EMBXGTAR + 4)      //保留，未使用
#define EMBMDATA   (EMBXGTAR + 5)      //数据过多
#define EMBBADSLAVE (EMBXGTAR + 6)     //响应与查询地址不匹配

extern const unsigned int libmodbus_version_major;
extern const unsigned int libmodbus_version_minor;
extern const unsigned int libmodbus_version_micro;

typedef struct _modbus modbus_t;

typedef struct _modbus_mapping_t {
    int nb_bits;                      //线圈寄存器的数量
    int start_bits;                   //线圈寄存器的起始地址
    int nb_input_bits;                //离散输入寄存器的数量
    int start_input_bits;             //离散输入寄存器的起始地址
    int nb_input_registers;           //输入寄存器的数量
    int start_input_registers;        //输入寄存器的起始地址
    int nb_registers;                 //保持寄存器的数量
    int start_registers;              //保持寄存器的起始地址
    uint8_t *tab_bits;                //指向线圈寄存器的值
    uint8_t *tab_input_bits;          //指向离散输入寄存器的值
    uint16_t *tab_input_registers;    //指向输入寄存器的值
    uint16_t *tab_registers;          //指向保持寄存器的值
} modbus_mapping_t;

typedef enum
{
    MODBUS_ERROR_RECOVERY_NONE          = 0,         //不恢复
    MODBUS_ERROR_RECOVERY_LINK          = (1<<1),    //链路层恢复
    MODBUS_ERROR_RECOVERY_PROTOCOL      = (1<<2)     //协议层恢复
} modbus_error_recovery_mode;

/*
此函数的功能是设置从站地址
RTU模式：如果libmodbus应用于主站设备端，则相当于定义远端设备ID；如果libmodbus应用
于从站设备端，则相当于定义自身设备ID；在RTU模式下，参数slave取值范围为0～247，
其中0(MODBUS_BROADCAST_ADDRESS)为广播地址。
TCP模式：通常，TCP模式下此函数不需要使用。在某些特殊场合，列入串行Modbus设备转换为
TCP模式传输的情况下，此函数才被使用。此种情况下，参数slave取值范围为0～247,0位广播地址；
如果不进行设置，则TCP模式下采用默认值MODBUS_TCP_SLAVE(0xFF)
*/
MODBUS_API int modbus_set_slave(modbus_t* ctx, int slave);
MODBUS_API int modbus_get_slave(modbus_t* ctx);

/*
用于在连接失败或者传输异常的情况下，设置错误恢复模式*/
MODBUS_API int modbus_set_error_recovery(modbus_t *ctx, modbus_error_recovery_mode error_recovery);

/*
此函数设置当前SOCKET或串口句柄，主要用于多客户端连接到单一服务器的场合*/
MODBUS_API int modbus_set_socket(modbus_t *ctx, int s);
MODBUS_API int modbus_get_socket(modbus_t *ctx);

MODBUS_API int modbus_get_response_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);

/*用于获取或设置响应超时，注意时间单位分别是秒和微秒*/
MODBUS_API int modbus_set_response_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

MODBUS_API int modbus_get_byte_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);

/*用于获取或设置连续字节之间的超时时间，注意时间单位分别是秒和微秒*/
MODBUS_API int modbus_set_byte_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

MODBUS_API int modbus_get_indication_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
MODBUS_API int modbus_set_indication_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

MODBUS_API int modbus_get_header_length(modbus_t *ctx);    //获取报文长度


/*
此函数用于主站设备与从站设备建立连接
RTU模式：它实质调用了文件 modbus-rtu.c 中的函数static int _modbus_rtu_connect(modbus_t * ctx);
    在此函数中，进行了串口波特率、校验位、数据位、停止位等的设置。
TCP模式：modbus_connect() 调用了文件 modbus-tcp.c 中的函数 static int _modbus_tcp_connect(modbus_t * ctx);
    在函数_modbus_tcp_connect()中，对TCP/IP各参数进行了设置和连接。
*/
MODBUS_API int modbus_connect(modbus_t *ctx);

/*
关闭Modbus 连接。在应用程序结束之前，一定调用此函数关闭连接。
RTU模式：它实质调用函数_modbus_rtu_close(modbus_t * ctx)关闭串口句柄；
TCP模式：它实质调用函数_modbus_tcp_close(modbus_t * ctx)关闭Socket句柄。
*/
MODBUS_API void modbus_close(modbus_t *ctx);

/*
释放libmodbus实例，使用完libmodbus需要释放掉
modbus_t *ctx：libmodbus实例
*/
MODBUS_API void modbus_free(modbus_t *ctx);

MODBUS_API int modbus_flush(modbus_t *ctx);

/*
此函数用于是否设置为DEBUG模式
若参数flag设置为TRUE，则进入DEBUG模式。若设置为FALSE，则切换为非DEBUG模式。
在DEBUG模式下，所有通信数据将按十六进制方式显示在屏幕上，以方便调试。
*/
MODBUS_API int modbus_set_debug(modbus_t *ctx, int flag);  

MODBUS_API const char *modbus_strerror(int errnum); //用于获取当前错误字符串


/*
此函数对应于功能码01(0x01)
读取线圈/离散量输出状态，可读取多个连续线圈的状态
modbus_t *ctx：Modbus实例
int addr: 线圈地址
int nb：读取线圈的个数
uint8_t *dest: 传出的状态值
*/
MODBUS_API int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);


/*
此函数对应于功能码02(0x02)
读取离散量输入状态，可读取多个连续输入的状态
modbus_t *ctx：Modbus实例
int addr：输入地址
int nb：读取输入的个数
uint8_t *dest：传出的状态值
*/
MODBUS_API int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);


/*
此函数对应于功能码03(0x03)
读取保持寄存器的值，可读取多个连续输入保持寄存器
modbus_t *ctx：Modbus实例
int addr：输入地址
int nb：读取保持寄存器的个数
uint8_t *dest：传出的寄存器值
*/
MODBUS_API int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);


/*
此函数对应于功能码04(0x04)
读取输入寄存器的值，可读取多个连续输入输入寄存器
modbus_t *ctx：Modbus实例
int addr：输入地址
int nb：读取输入寄存器的个数
uint8_t *dest：传出的寄存器值
*/
MODBUS_API int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);


/*
此函数对应于功能码05(0x05)
写入单个线圈或单个离散输出的状态
modbus_t *ctx：Modbus实例
int addr：线圈地址
int status：线圈状态
*/
MODBUS_API int modbus_write_bit(modbus_t *ctx, int coil_addr, int status);


/*
此函数对应于功能码06(0x06)
写入单个保持寄存器
modbus_t *ctx：Modbus实例
int addr：寄存器地址
int value：寄存器的值
*/
MODBUS_API int modbus_write_register(modbus_t *ctx, int reg_addr, int value);


/*
此函数对应于功能码15(0x0F)
写入多个连续线圈的状态
modbus_t *ctx：Modbus实例
int addr：线圈地址
int nb：线圈个数
const uint8_t *src：多个线圈状态
*/
MODBUS_API int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *data);


/*
此函数对应于功能码16(0x16)
写入多个连续保持寄存器
int addr：寄存器地址
int nb：寄存器的个数
const uint16_t *src：多个寄存器的值
*/
MODBUS_API int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *data);
MODBUS_API int modbus_mask_write_register(modbus_t *ctx, int addr, uint16_t and_mask, uint16_t or_mask);
MODBUS_API int modbus_write_and_read_registers(modbus_t *ctx, int write_addr, int write_nb,
                                               const uint16_t *src, int read_addr, int read_nb,
                                               uint16_t *dest);

/*
此函数对应于功能码17(0x17)
报告从站ID
int max_dest:最大存储空间
unint8_t *dest:存储返回数据
*/
MODBUS_API int modbus_report_slave_id(modbus_t *ctx, int max_dest, uint8_t *dest);

/*
函数modbus_mapping_new_start_address()与modbus_mapping_new()的
功能一致，即在内存中申请一段连续的空间，用于分别存储4个寄存器快的数据。
两个函数唯一的区别是，modbus_mapping_new()函数申请的寄存器地址默认从0
开始计算。
*/
MODBUS_API modbus_mapping_t* modbus_mapping_new_start_address(
    unsigned int start_bits, unsigned int nb_bits,
    unsigned int start_input_bits, unsigned int nb_input_bits,
    unsigned int start_registers, unsigned int nb_registers,
    unsigned int start_input_registers, unsigned int nb_input_registers);

MODBUS_API modbus_mapping_t* modbus_mapping_new(int nb_bits, int nb_input_bits,
                                                int nb_registers, int nb_input_registers);
MODBUS_API void modbus_mapping_free(modbus_mapping_t *mb_mapping);  //释放申请的内存，防止内存泄漏

MODBUS_API int modbus_send_raw_request(modbus_t *ctx, uint8_t *raw_req, int raw_req_length);

MODBUS_API int modbus_receive(modbus_t *ctx, uint8_t *req);

MODBUS_API int modbus_receive_confirmation(modbus_t *ctx, uint8_t *rsp);

MODBUS_API int modbus_reply(modbus_t *ctx, const uint8_t *req,
                            int req_length, modbus_mapping_t *mb_mapping);
MODBUS_API int modbus_reply_exception(modbus_t *ctx, const uint8_t *req,
                                      unsigned int exception_code);

/**
 * UTILS FUNCTIONS
 **/

#define MODBUS_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE(data) ((data) & 0xFF)
#define MODBUS_GET_INT64_FROM_INT16(tab_int16, index) \
    (((int64_t)tab_int16[(index)    ] << 48) + \
     ((int64_t)tab_int16[(index) + 1] << 32) + \
     ((int64_t)tab_int16[(index) + 2] << 16) + \
      (int64_t)tab_int16[(index) + 3])
#define MODBUS_GET_INT32_FROM_INT16(tab_int16, index) ((tab_int16[(index)] << 16) + tab_int16[(index) + 1])
#define MODBUS_GET_INT16_FROM_INT8(tab_int8, index) ((tab_int8[(index)] << 8) + tab_int8[(index) + 1])
#define MODBUS_SET_INT16_TO_INT8(tab_int8, index, value) \
    do { \
        tab_int8[(index)] = (value) >> 8;  \
        tab_int8[(index) + 1] = (value) & 0xFF; \
    } while (0)
#define MODBUS_SET_INT32_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 16; \
        tab_int16[(index) + 1] = (value); \
    } while (0)
#define MODBUS_SET_INT64_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 48; \
        tab_int16[(index) + 1] = (value) >> 32; \
        tab_int16[(index) + 2] = (value) >> 16; \
        tab_int16[(index) + 3] = (value); \
    } while (0)

MODBUS_API void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value);
MODBUS_API void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,
                                       const uint8_t *tab_byte);
MODBUS_API uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits);
MODBUS_API float modbus_get_float(const uint16_t *src);
MODBUS_API float modbus_get_float_abcd(const uint16_t *src);
MODBUS_API float modbus_get_float_dcba(const uint16_t *src);
MODBUS_API float modbus_get_float_badc(const uint16_t *src);
MODBUS_API float modbus_get_float_cdab(const uint16_t *src);

MODBUS_API void modbus_set_float(float f, uint16_t *dest);
MODBUS_API void modbus_set_float_abcd(float f, uint16_t *dest);
MODBUS_API void modbus_set_float_dcba(float f, uint16_t *dest);
MODBUS_API void modbus_set_float_badc(float f, uint16_t *dest);
MODBUS_API void modbus_set_float_cdab(float f, uint16_t *dest);

#include "modbus-tcp.h"
#include "modbus-rtu.h"

MODBUS_END_DECLS

#endif  /* MODBUS_H */

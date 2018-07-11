#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "modbus.h"
#include "errno.h"
#include "getopt.h"
#include "mod_common.h"
//定义选项
const char DebugOpt[]  = "debug";
const char TcpOptVar[] = "tcp";
const char RtuOptVal[] = "rtu";

typedef enum
{
	//定义功能码
	FuncNone                 = -1,
	ReadCoils                = 0x01,
	ReadDiscreteInput       = 0x02,
	ReadHoldingRegisters     = 0x03,
	ReadInputRegisters       = 0x04,
	WriteSingleCoil          = 0x05,
	WriteSingleRegister    = 0x06,
	WriteMultipleCoils       = 0x0f,
	WriteMultipleRegisters   = 0x10
}Function;

//打印帮助说明
void printerUsage(const char progName[])
{
	printf("%s [--%s] [--m{rtu|tcp}] [-a<slave-addr=1>] {-c<read-no>=1]\n\t"\
		"[-r<start-addr>=100] [-t<f-type>] [-o<timeout-ms>=1000]\n\t"\
		"[{rtu-params|tcp-params}] serialport|host [<writer-data>]\n",
		progName, DebugOpt);
	printf("NOTE: if first reference address start at 0, set -0\n");
	printf("f-type:\n"\
		"\t(0x01) Read Coils, (0x02) Read Discrete Input\n"\
		"\t(0x03) Read Holding Registers, (0x04) Read Input Registers\n"\
		"\t(0x05) Write Single Coils, (0x06) WriteSingle Register\n"\
		"\t(0x0F) WriterMultipleCoils, (0x10) Write Multiple register\n");
	printf("rtu-params:\n"\
		"\tb<baud-rate>=9600\n"\
		"\td{7|8}<data-bits>=8\n"\
		"\ts{1|2}<stop-bits>=1\n"\
		"\tp{none|even|odd}=even\n");
	printf("tcp-params:\n"\
		"\tp<port>=502\n");
	printf("Examples (run with default mbServer at port 1502): \n"\
		"\tWrite data: \t%s --debug -mtcp -t0x10 -r0 -p1502 127.0.0.1 0x01 0x02\n"\
		"\tRead that data:\t%s --debug -mtcp -t0x03 -r0 -p1502 127.0.0.1 -c3\n",
		progName, progName);
}

int main(int argc, char * * argv)
{
	int c;
	int ok;

	int debug = 0;
	BackendParams * backend = 0;
	int slaveAddr = 1;
	int startAddr = 100;
	int startReferenceAt0 = 0;
	int readWriteNo = 1;
	int fType = FuncNone;
	int timeout_ms = 1000;
	int hasDevice = 0;

	int isWriteFunction = 0;
	enum WriteDataType
	{
		DataInt,
		Data8Array,
		Data16Array
	}wDataType = DataInt;
	union Data
	{
		int dataInt;
		uint8_t * data8;
		uint16_t * data16;
	}data;

	while (1)
	{
		int option_intex = 0;
		static struct option long_options[] =
		{
			{ DebugOpt, no_argument, 0, 0 },
			{ 0, 0, 0, 0 }
		};

		//命令行解析
		c = getopt_long(argc, argv, "a:b:d:c:m:r:s:t:p:o:0",
			long_options, &option_intex);
		if (c == -1)
		{
			break;
		}

		//根据参数读取配置
		switch (c)
		{
		case 0:
			if (0 == strcmp(long_options[option_intex].name, DebugOpt))
			{
				debug = 1;
			}
			break;

		case 'a':
		{
			slaveAddr = getInt(optarg, &ok);
			if (0 == ok)
			{
				printf("Slave address (%s) is not integer!\n\n", optarg);
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}

		}
		break;
		case 'c':
		{
			readWriteNo = getInt(optarg, &ok);
			if (0 == ok)
			{
				printf("#elements to read/write (%s) is not integer!\n\n", optarg);
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}
		}
		break;

		case 'm':
			//创建TCP或RTU模式
			if (0 == strcmp(optarg, TcpOptVar))
			{
				backend = createTcpBackend((TcpBackend *)malloc(sizeof(TcpBackend)));
			}
			else if (0 == strcmp(optarg, RtuOptVal))
				backend = createRtuBackend((RtuBackend *)malloc(sizeof(RtuBackend)));
			else
			{
				printf("Unrecognized connection type %s\n\n", optarg);
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}
			break;

		case 'r':
		{
			startAddr = getInt(optarg, &ok);
			if (0 == ok)
			{
				printf("Start address (%s) is not integer!\n\n", optarg);
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}
		}
		break;

		case 't':
		{
			fType = getInt(optarg, &ok);
			if (0 == ok)
			{
				printf("Function type (%s) is not integer!\n\n", optarg);
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}
		}
		break;
		
		case 'o':
		{
			timeout_ms = getInt(optarg, &ok);
			if (0 == ok)
			{
				printf("Timeout (%s) is not integer!\n\n", optarg);
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}
			printf("Timeout set to %d\r\n", timeout_ms);
		}
		break;

		case '0':
			startReferenceAt0 = 1;
			break;
			//tcp/rtu params
		case 'p':
		case 'b':
		case 'd':
		case 's':
			if (0 == backend)
			{
				printf("Connect type (-m switch) has to be set!n");
				printerUsage(argv[0]);
				exit(EXIT_FAILURE);
			}
			else
			{
				if (0 == backend->setParam(backend,c,optarg))
				{
					printerUsage(argv[0]);
					exit(EXIT_FAILURE);
				}
			}
			break;
		case '?':
			break;

		default:
			printf("?? getopt return char code 0%o ??\n", c);
		}
	}
	
	if (0 == backend)
	{
		printf("Help:\n");
		printerUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (1 == startReferenceAt0)
	{
		startAddr--;
	}

	//choose write data type
	switch (fType)
	{
	case(ReadCoils) :
		wDataType = Data8Array;
		break;
	case(ReadDiscreteInput) :
		wDataType = DataInt;
		break;
	case(ReadHoldingRegisters) :
	case(ReadInputRegisters) :
		wDataType = Data16Array;
		break;
	case(WriteSingleCoil) :
	case(WriteSingleRegister) :
		wDataType = DataInt;
		isWriteFunction = 1;
		break;
	case(WriteMultipleCoils) :
		wDataType = Data8Array;
		isWriteFunction = 1;
		break;
	case(WriteMultipleRegisters) :
		wDataType = Data16Array;
		isWriteFunction = 1;
		break;
	default:
		printf("No correct function type chose\n");
		printerUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (isWriteFunction)
	{
		int dataNo = argc - optind - 1;
		readWriteNo = dataNo;	
	}
	memset(&data, 0, sizeof(union Data));
	//申请buffer
	switch (wDataType)
	{
	case DataInt:
		//no need to alloc anything
		break;
	case Data8Array:
		data.data8 = malloc(readWriteNo * sizeof(uint8_t));
		break;
	case Data16Array:
		data.data16 = malloc(readWriteNo * sizeof(uint16_t));
		break;
	default:
		printf("Data alloc error!\n");
		break;
	}

	int wDataIdx = 0;
	if (1 == debug && 1 == isWriteFunction)
		printf("Data to write: ");
	if (optind < argc)
	{
		while (optind < argc)
		{
			if (0 == hasDevice)
			{
				if (0 != backend)
				{
					if (RTU_T == backend->type)
					{
						RtuBackend * rtuP = (RtuBackend *)backend;
						strcpy(rtuP->devName, argv[optind]);
						hasDevice = 1;
					}
					else if (TCP_T == backend->type)
					{
						TcpBackend * tcpP = (TcpBackend *)backend;
						strcpy(tcpP->ip, argv[optind]);
						hasDevice = 1;
					}
				}
			}
			else      //设置写入数据buffer
			{
				switch (wDataType)
				{
				case DataInt:
					data.dataInt = getInt(argv[optind], 0);
					if (debug)
						printf("0x%x", data.dataInt);
					break;
				case Data8Array:
				{
					data.data8[wDataIdx] = getInt(argv[optind], 0);
					if (debug)
						printf("0x%02x ", data.data8[wDataIdx]);
				}
					break;
				case Data16Array:
				{
					data.data16[wDataIdx] = getInt(argv[optind], 0);
					if (debug)
						printf("0x%04x ", data.data16[wDataIdx]);
				}
					break;
				}
				wDataIdx++;
			}
			optind++;
		}
	}
	if (1 == debug && 1 == isWriteFunction)
		printf("\n");

	//create modbus context, and prepare it
	modbus_t * ctx = backend->createCtxt(backend);
	modbus_set_debug(ctx, debug);
	modbus_set_slave(ctx, slaveAddr);

	//issue the request
	int ret = -1;
	if (modbus_connect(ctx) == -1)
	{
		fprintf(stderr, "Connection failed: %s\n",
			modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	else
	{
		switch (fType)
		{
		case(ReadCoils) :
			ret = modbus_read_bits(ctx, startAddr, readWriteNo, data.data8);
			break;
		case(ReadDiscreteInput) :
			printf("ReadDiscreteInput: not implemented yet!\n");
			wDataType = DataInt;
			break;
		case(ReadHoldingRegisters) :
			ret = modbus_read_registers(ctx, startAddr, readWriteNo, data.data16);
			break;
		case(ReadInputRegisters) :
			ret = modbus_read_input_registers(ctx, startAddr, readWriteNo,data.data16);
			break;
		case(WriteSingleCoil) :
			ret = modbus_write_bit(ctx, startAddr, data.dataInt);
			break;
		case(WriteSingleRegister) :
			ret = modbus_write_register(ctx, startAddr, data.dataInt);
			break;
		case(WriteMultipleCoils) :
			ret = modbus_write_bits(ctx, startAddr, readWriteNo, data.data8);
			break;
		case(WriteMultipleRegisters) :
			ret = modbus_write_registers(ctx, startAddr, readWriteNo, data.data16);
			break;
		default:
			printf("No correct function type chosen");
			printerUsage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (ret == readWriteNo)  //success
	{
		if (isWriteFunction)
			printf("SUCCESS: write %d elements!\n", readWriteNo);
		else
		{
			printf("SUCCESS: read %d of elements:\n\tData: ", readWriteNo);
			int i = 0;
			if (DataInt == wDataType)
			{
				printf("0x%04x\n", data.dataInt);
			}
			else
			{
				const char Format8[]  = "0x%02x ";
				const char Format16[] = "0x%04x ";
				const char  * format = ((Data8Array == wDataType) ? Format8 : Format16);
				for (; i < readWriteNo; ++i)
				{
					printf(format, (Data8Array == wDataType) ?
						data.data8[i] : data.data16[i]);
				}
				printf("\n");
			}
		}
	}
	else
	{
		printf("ERROR occured!\n");
		modbus_strerror(errno);
	}

	//cleanup
	modbus_close(ctx);
	modbus_free(ctx);
	backend->del(backend);

	switch (wDataType)
	{
	case DataInt:
		//nothing to be done
		break;
	case Data8Array:
		free(data.data8);
		break;
	case Data16Array:
		free(data.data16);
		break;
	}

	exit(EXIT_FAILURE);
}

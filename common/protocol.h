#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


// ������ʹ��
#define MY_DEVICE_NAME TEXT("\\Device\\MyDevice-dsh")           // �豸��
#define MY_SYMBOL_NAME TEXT("\\DosDevices\\MyDevice-dsh")       // ������


// Ӧ�ó�����ʹ��
#define MY_DRIVER_FILE "MyHunterDriver.sys"         // �����ļ���
#define MY_SYMBOL_FILE TEXT("\\\\.\\MyDevice-dsh")  // ����������

//�Զ���������
#define MY_CODE(function) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800 + (function), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CODE_GET_GDT_BUFSIZE MY_CODE(0)     // ��ȡGDT��С
#define CODE_GET_GDT MY_CODE(1)             // ��ȡGDT

#define CODE_GET_EPROCESS MY_CODE(2)        // ��ȡEPROCESS
#define CODE_GET_IMAGE_PATH MY_CODE(3)      // ��ȡӳ��·��

#define CODE_GET_IDT MY_CODE(4)             // ��ȡIDT

#define CODE_GET_PROCESS_MODULE_COUNT MY_CODE(5)    // ��ȡ����ģ�����
#define CODE_GET_PROCESS_MODULE MY_CODE(6)          // ��ȡ����ģ����Ϣ

#define CODE_GET_PROCESS_THREAD_COUNT MY_CODE(7)    // ��ȡ����ģ�����
#define CODE_GET_PROCESS_THREAD MY_CODE(8)          // ��ȡ����ģ����Ϣ

#define CODE_SSDT_COUNT MY_CODE(9)  // ��ȡ����ģ�����
#define CODE_SSDT MY_CODE(10)       // ��ȡ����ģ����Ϣ

#define CODE_SHADOWSSDT_COUNT MY_CODE(11)   // ��ȡ����ģ�����
#define CODE_SHADOWSSDT MY_CODE(12)         // ��ȡ����ģ����Ϣ

#define CODE_DRIVER_MODULE_COUNT MY_CODE(13)    // ��ȡ����ģ�����
#define CODE_DRIVER_MODULE MY_CODE(14)          // ��ȡ����ģ��

// ������ݽṹ
#pragma pack(push)
#pragma pack(1)

/*
 * ��ѡ����
 */
struct SegmentSelector {
    unsigned int rpl : 2;       // ��Ȩ��
    unsigned int ti : 1;        // Ϊ0ָ��GDT��Ϊ1ָ��LDT
    unsigned int index : 13;    // ����
};

/*
 * ������������8�ֽ�
 *      base = �λ�ַ��32λ
 *      limit = �ν��ޣ�20λ
 *          �� G = 0ʱ��limit * 0 + 0xfff
 *          �� G = 1ʱ��limit * 4K + 0xfff
 */
struct SegmentDescriptor {
    union {
        unsigned int low32;                     // ��32λ�ֶ�
        struct {
            unsigned int limit_0_15 : 16;       // ��0-15λ�Ķν���
            unsigned int base_0_15 : 16;        // ��0-15λ�Ķλ�ַ
        };
    } SegDesLow;                                // ����������32λ�ֶ�

    union {
        unsigned int high32;                    // ��32λ�ֶ�
        struct {
            unsigned int base_16_23 : 8;        // ��16-23λ�Ķλ�ַ
            unsigned int type : 4;              // �ڴ�����
            unsigned int s : 1;                 // Ϊ0����ϵͳ�Σ�Ϊ1���Ǵ洢��
            unsigned int dpl : 2;               // ��Ȩ����0 ~ 3��
            unsigned int p : 1;                 // ����λ����ʾ���������Ƿ���Ч
            unsigned int limit_16_19 : 4;       // ��16-19λ�Ķν���
            unsigned int avl : 1;               // ���������λ���ɲ���ϵͳ��������
            unsigned int l : 1;                 // 64λ�α�־
            unsigned int d_b : 1;               // Ϊ0����16λ�Σ�Ϊ1����32λ��
            unsigned int g : 1;                 // ����λ��Ϊ1��λ��4K��Ϊ0�ǵ�λ��1�ֽ�
            unsigned int base_24_31 : 8;        // ��24-31λ�Ķλ�ַ
        };
    } SegDesHigh;                               // ����������32λ�ֶ�
};


/*
 * GDTRȫ����������Ĵ���
 */
struct GDTR {
    unsigned short limit;   // ȫ����������Ľ���
    unsigned int address;   // ȫ����������ĵ�ַ
};


/*
 * ������������
 */

#define TASK_GATE 5         // ������
#define INTERRUPT_GATE 6    // �ж���
#define TRAP_GATE 7         // ������

/*
 * ��������
 *      sλ�ض�Ϊ0
 */
struct GateDescriptor {
    union {
        unsigned int low32;                     // ��32λ�ֶ�
        struct {
            unsigned int offset_0_15 : 16;      // ��0-15λ��ƫ��
            unsigned int seg_sel : 16;          // ѡ����
        };
    } GateDesLow;                               // ����������32λ�ֶ�

    union {
        unsigned int high32;                    // ��32λ�ֶ�
        struct {
            unsigned int reserve_0_4 : 5;       // ����
            unsigned int reserve_5_7 : 3;       // ������ȫ0
            unsigned int type : 3;              // ���� Task: 5, Interrupt: 6, Trap: 7
            unsigned int d : 1;                 // �ŵĴ�С��1 = 32λ��0 = 16λ
            unsigned int s : 1;                 // 0ϵͳ��
            unsigned int dpl : 2;               // ��Ȩ��
            unsigned int p : 1;                 // ����λ
            unsigned int offset_16_31 : 16;     // ��16-31λ��ƫ��
        };
    } GateDesHigh;                              // ����������32λ�ֶ�
};


/*
 * IDTR�ж��������Ĵ���
 */
struct IDTR {
    unsigned short limit;   // ȫ����������Ľ���
    unsigned int address;   // ȫ����������ĵ�ַ
};


/*
 * SSDT
 */
typedef struct SSDT {
    unsigned int func_arr_ptr;      // api�����ָ��
    unsigned int reserve;           // ����
    unsigned int count;             // ����Ԫ�ظ���
    unsigned char param_arr_ptr;    // api��������ָ��
} SSDT, ShadowSSDT;

/*
 * ģ��ڵ�
 */
struct ModuleItem {
    wchar_t path[0x200];    // ·��
    unsigned int base;      // ��ַ
    unsigned int size;      // ��С
};

/*
 * �߳̽ڵ�
 */
struct ThreadItem {
    unsigned int pid;
    unsigned int ethread;
    unsigned int teb;
};


/*
 * SSDT�ڵ�
 */
typedef struct SSDTItem {
    unsigned int num;   // ���
    unsigned int addr;  // ��ַ
} SSDTItem, ShadowSSDTItem;


/*
 * ����ģ��ڵ�
 */
struct DriverModule {
    wchar_t name[0x100];    // ������
    unsigned int base;      // ��ַ
    unsigned int size;      // ��С
    wchar_t path[0x100];    // ����·��
};

#pragma pack(pop)
#endif

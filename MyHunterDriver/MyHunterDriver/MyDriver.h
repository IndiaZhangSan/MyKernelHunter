#pragma once

#include <ntifs.h>
#include <Ntddk.h>
#include "../../common/protocol.h"


// ��ں���
DRIVER_INITIALIZE DriverEntry;

// ж�غ���
DRIVER_UNLOAD DriverUnload;

// ����
NTSTATUS
MyDispatchCreate(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
);

// ����
NTSTATUS
MyDispatchControl(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
);

// �ر�
NTSTATUS
MyDispatchClose(
    _In_ struct _DEVICE_OBJECT *DeviceObject,
    _Inout_ struct _IRP *Irp
);


// �����������ҳ�ڴ���
#pragma alloc_text("INIT", DriverEntry)
#pragma alloc_text("PAGE", DriverUnload)
#pragma alloc_text("PAGE", MyDispatchCreate)
#pragma alloc_text("PAGE", MyDispatchControl)
#pragma alloc_text("PAGE", MyDispatchClose)


// δ��������ָ������
typedef NTSTATUS(__stdcall *pfnZwQueryInformationProcess)(
    _In_      HANDLE           ProcessHandle,
    _In_      PROCESSINFOCLASS ProcessInformationClass,
    _Out_     PVOID            ProcessInformation,
    _In_      ULONG            ProcessInformationLength,
    _Out_opt_ PULONG           ReturnLength
    );


// ���ܺ���

// ��ԭָ��cpu�������У�ȫ�����Ķ������У�
void restore_core_working();

// ��ȡ_EPROCESS.ActiveProcessLinks��ƫ��
ULONG get_activeprocesslinks_offset();

// ͨ��pid���ҽ���
PEPROCESS find_process_by_id(ULONG pid);

// ��ȡָ�����̵�CR3
ULONG get_process_cr3(ULONG pid);

/*
 * ��ȡGDT��������С
 *      core_mark: ָ��cpu���Ĺ�������
 *      output: ���������
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR get_gdt_buf_size(DWORD32 core_mark, DWORD32 *output);

/*
 * ��ȡGDT����
 *      core_mark: ָ��cpu���Ĺ�������
 *      output: ���������
 *      output_size: �����������С
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR get_gdt(DWORD32 core_mark, PVOID output, ULONG output_size);

/*
 * ��ȡIDT����
 *      core_mark: ָ��cpu���Ĺ�������
 *      output: ���������
 *      output_size: �����������С
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR get_idt(DWORD32 core_mark, PVOID output, ULONG output_size);

/*
 * ��ȡָ�����̵�EPROCESS
 *      pid: ����id
 *      output: ���������
 *      output_size: �����������С
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR get_eprocess(ULONG pid, PULONG output, ULONG output_size);

/*
 * ��ȡָ�����̵�ӳ��·��
 *      pid: ����id
 *      output: ���������
 *      output_size: �����������С
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR get_image_path(ULONG pid, PVOID output, ULONG output_size);

/*
 * ��ȡָ�����̵��ڴ�
 *      pid: ����id
 *      mem_addr: �ڴ��ַ
 *      read_buf: ������
 *      buf_size: ��������С
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR read_process_memory(ULONG pid, PVOID mem_addr, PVOID read_buf, ULONG buf_size);

/*
 * д��ָ�����̵��ڴ�
 *      pid: ����id
 *      mem_addr: �ڴ��ַ
 *      write_buf: ������
 *      buf_size: ��������С
 *  ����ֵ: IoStatus��Information
 */
ULONG_PTR write_process_memory(ULONG pid, PVOID mem_addr, PVOID write_buf, ULONG buf_size);

/*
 * ��ȡָ�����̵�ģ�����
 *      pid: ����id
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_process_module_count(ULONG pid, PULONG output);

/*
 * ��ȡָ�����̵�ģ��
 *      pid: ����id
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_process_module(ULONG pid, PVOID output, ULONG output_size);

/*
 * ��ȡָ�����̵��̸߳���
 *      pid: ����id
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_process_thread_count(ULONG pid, PULONG output);

/*
 * ��ȡָ�����̵��߳�
 *      pid: ����id
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_process_thread(ULONG pid, PVOID output, ULONG output_size);

/*
 * ��ȡSSDT����
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
extern ULONG KeServiceDescriptorTable;
ULONG_PTR get_ssdt_count(PULONG output, ULONG output_size);

/*
 * ��ȡSSDT
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_ssdt(PVOID output, ULONG output_size);

/*
 * ��ȡShadowSSDT����
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_shadowssdt_count(PULONG output, ULONG output_size);

/*
 * ��ȡShadowSSDT
 *      output: ���������
 *      output_size: �����������С
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_shadowssdt(PVOID output, ULONG output_size);

/*
 * û������LDR_DATA_TABLE_ENTRY�ṹ������
 */
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;
        };

        struct {
            PVOID LoadedImports;
        };
    };
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

/*
 * ��ȡ����ģ�����
 *      pDriverObject: ��������
 *      output: ���������
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_driver_module_count(PDRIVER_OBJECT pDriverObject, PULONG output);

/*
 * ��ȡ����ģ�����
 *      pDriverObject: ��������
 *      output: ���������
 * ����ֵ: IoStatus��Information
 */
ULONG_PTR get_driver_module(PDRIVER_OBJECT pDriverObject, PVOID output, ULONG output_size);
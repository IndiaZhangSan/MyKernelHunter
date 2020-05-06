#include "MyDriver.h"


// ��ں���
_Use_decl_annotations_
NTSTATUS
DriverEntry(struct _DRIVER_OBJECT* DriverObject, PUNICODE_STRING  RegistryPath)
{
    KdBreakPoint();
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Hello Kernel! - Install\n");

    // ע��ж�غ���
    DriverObject->DriverUnload = DriverUnload;

    // ע����ǲ����
    DriverObject->MajorFunction[IRP_MJ_CREATE] = MyDispatchCreate;              // ����
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyDispatchControl;     // ����
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyDispatchClose;                // �ر�

    // ����IOģʽ
    DriverObject->Flags |= DO_BUFFERED_IO;   // ���ƻ���������R0�п��ٿռ䣬���ƻ��������ݵ��˴�


    NTSTATUS status;    // ����״̬

    // �����豸����һ��ΪIO�����������ں��豸FILE_DEVICE_UNKNOWN
    UNICODE_STRING driver_name;
    RtlInitUnicodeString(&driver_name, MY_DEVICE_NAME);
    PDEVICE_OBJECT device_object_ptr = NULL;
    status = IoCreateDevice(DriverObject, 0, &driver_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object_ptr);

    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IoCreateDevice Error: %p\n", status);
        return status;
    }

    // ������������
    UNICODE_STRING symbo_name;
    RtlInitUnicodeString(&symbo_name, MY_SYMBOL_NAME);
    status = IoCreateSymbolicLink(&symbo_name, &driver_name);

    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IoCreateSymbolicLink Error: %p\n", status);
        return status;
    }

    return STATUS_SUCCESS;
}


// ж������
void DriverUnload(struct _DRIVER_OBJECT* DriverObject)
{
    // ɾ���豸
    IoDeleteDevice(DriverObject->DeviceObject);     // ���Ǹ���������豸����ʱ����Ҫ������

    // ɾ���������ӣ�����еĻ�
    UNICODE_STRING symbo_name;
    RtlInitUnicodeString(&symbo_name, MY_SYMBOL_NAME);
    IoDeleteSymbolicLink(&symbo_name);

    KdPrint(("Hello Kernel! - UnInstall\n"));
}


// ����
NTSTATUS
MyDispatchCreate(_In_ struct _DEVICE_OBJECT *DeviceObject, _Inout_ struct _IRP *Irp)
{
    // �������
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

// ����
NTSTATUS
MyDispatchControl(_In_ struct _DEVICE_OBJECT *DeviceObject, _Inout_ struct _IRP *Irp)
{
    KdPrint(("Dispatch: [MyDispatchControl]\n"));

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG_PTR Information = 0;

    PIO_STACK_LOCATION stack_location = IoGetCurrentIrpStackLocation(Irp);                  // ��ȡIrp��ջ
    ULONG input_length = stack_location->Parameters.DeviceIoControl.InputBufferLength;      // ��ȡ���뻺��������
    ULONG output_length = stack_location->Parameters.DeviceIoControl.OutputBufferLength;    // ��ȡ�������������
    ULONG control_code = stack_location->Parameters.DeviceIoControl.IoControlCode;          // ������
    PVOID systembuff = Irp->AssociatedIrp.SystemBuffer;                                     // �ں˻�����


    // ���幤������
    switch (control_code) {
    case CODE_GET_GDT_BUFSIZE:                  // ��ȡGDT��������С
        Information = get_gdt_buf_size(*(DWORD32 *)systembuff, systembuff);
        break;
    case CODE_GET_GDT:                          // ��ȡGDT
        Information = get_gdt(*(DWORD32 *)systembuff, systembuff, output_length);
        break;
    case CODE_GET_EPROCESS:                     // ��ȡָ�����̵�EPROCESS
        Information = get_eprocess(*(ULONG *)systembuff, systembuff, output_length);
        break;
    case CODE_GET_IMAGE_PATH:                   // ��ȡָ�����̵�ӳ��·��
        Information = get_image_path(*(ULONG *)systembuff, systembuff, output_length);
        break;
    case CODE_GET_IDT:                          // ��ȡIDT
        Information = get_idt(*(DWORD32 *)systembuff, systembuff, output_length);
        break;
    case CODE_GET_PROCESS_MODULE_COUNT:         // ��ȡָ������ģ�����
        Information = get_process_module_count(*(ULONG *)systembuff, systembuff);
        break;
    case CODE_GET_PROCESS_MODULE:               // ��ȡָ������ģ��
        Information = get_process_module(*(ULONG *)systembuff, systembuff, output_length);
        break;
    case CODE_GET_PROCESS_THREAD_COUNT:         // ��ȡָ�������̸߳���
        Information = get_process_thread_count(*(ULONG *)systembuff, systembuff);
        break;
    case CODE_GET_PROCESS_THREAD:               // ��ȡָ�������߳�
        Information = get_process_thread(*(ULONG *)systembuff, systembuff, output_length);
        break;
    case CODE_SSDT_COUNT:                       // ��ȡSSDT����
        Information = get_ssdt_count((PULONG)systembuff, output_length);
        break;
    case CODE_SSDT:                             // ��ȡSSDT
        Information = get_ssdt((PULONG)systembuff, output_length);
        break;
    case CODE_SHADOWSSDT_COUNT:                 // ��ȡSSDT����
        Information = get_shadowssdt_count((PULONG)systembuff, output_length);
        break;
    case CODE_SHADOWSSDT:                       // ��ȡSSDT
        Information = get_shadowssdt((PULONG)systembuff, output_length);
        break;
    case CODE_DRIVER_MODULE_COUNT:
        Information = get_driver_module_count(DeviceObject->DriverObject, (PULONG)systembuff);
        break;
    case CODE_DRIVER_MODULE:
        Information = get_driver_module(DeviceObject->DriverObject, systembuff, output_length);
        break;
    default:
        break;
    }

    // �������
    Irp->IoStatus.Information = Information;
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

// �ر�
NTSTATUS
MyDispatchClose(_In_ struct _DEVICE_OBJECT *DeviceObject, _Inout_ struct _IRP *Irp)
{
    // �������
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

void restore_core_working()
{
    // ��ȡCPU��������
    ULONG core_count = KeQueryActiveProcessorCount(NULL);

    // ���ɹ�������
    KAFFINITY mark = 0;
    for (ULONG i = 0; i < core_count; i++) {
        mark |= (1 << i);
    }

    // ���ù�������ָ����cpu��������
    KeSetSystemAffinityThread(mark);
}

ULONG get_activeprocesslinks_offset()
{
    NTSTATUS status = STATUS_SUCCESS;

    // ��ȡϵͳ�汾
    ULONG offset = 0;
    RTL_OSVERSIONINFOW os_info = { 0 };
    status = RtlGetVersion(&os_info);
    if (!NT_SUCCESS(status)) {
        return offset;
    }

    // �ж�ϵͳ�汾
    switch (os_info.dwMajorVersion) {
    case 6:
        switch (os_info.dwMinorVersion) {
        case 1:             // win7
#ifdef _WIN64
            offset = 0x188;
#else
            offset = 0xb8;
#endif
            break;
        }
        break;
    case 10:                // win10
#ifdef _WIN64
        offset = 0x2f0;
#else
        offset = 0xb8;
#endif
        break;
    }
    return offset;
}

PEPROCESS find_process_by_id(ULONG pid)
{
    // ��ȡActiveProcessLinks�ֶε�ƫ��
    ULONG offset = get_activeprocesslinks_offset();
    if (offset == 0) {
        return NULL;
    }

    // ��������
    PEPROCESS first_eprocess = NULL, traverse_eprocess = NULL;
    first_eprocess = PsGetCurrentProcess();
    traverse_eprocess = first_eprocess;
    UINT8 found = FALSE;

    do {
        // ��EPROCESS�л�ȡPID
        ULONG process_id = (ULONG)PsGetProcessId(traverse_eprocess);
        if (process_id == pid) {
            // �ҵ�
            found = TRUE;
            break;
        }

        // ����ƫ�Ƽ�����һ��EPROCESS
        traverse_eprocess = (PEPROCESS)((PUCHAR)(((PLIST_ENTRY)((PUCHAR)traverse_eprocess + offset))->Flink) - offset);
    } while (traverse_eprocess != first_eprocess);

    if (!found) {
        return NULL;
    }

    return traverse_eprocess;
}

ULONG get_process_cr3(ULONG pid)
{
    ULONG reg_cr3 = 0;
    PEPROCESS eprocess = find_process_by_id(pid);
    if (eprocess == NULL)
        return 0;

    KAPC_STATE kapc_state;
    // ���ӵ�����
    KeStackAttachProcess(eprocess, &kapc_state);

    // ��ȡcr3
    __asm {
        cli
        mov eax, cr3;
        mov reg_cr3, eax
            sti
    }

    // �������
    KeUnstackDetachProcess(&kapc_state);
    return reg_cr3;
}

ULONG_PTR get_gdt_buf_size(DWORD32 core_mark, DWORD32 *output)
{
    // ���ù�������ָ����cpu��������
    KeSetSystemAffinityThread(core_mark);

    // ��ȡGDTR
    struct GDTR gdtr;
    RtlSecureZeroMemory(&gdtr, sizeof(struct GDTR));
    __asm {
        sgdt gdtr;
    }

    // ��ȡgdt�Ĵ�С
    *output = gdtr.limit + 1;

    // ��ԭ���к���
    restore_core_working();

    return sizeof(DWORD32);
}

ULONG_PTR get_gdt(DWORD32 core_mark, PVOID output, ULONG output_size)
{
    // ���ù�������ָ����cpu��������
    KeSetSystemAffinityThread(core_mark);

    // �����ٽ���
    KeEnterCriticalRegion();

    // ��ȡGDTR
    struct GDTR gdtr;
    RtlSecureZeroMemory(&gdtr, sizeof(struct GDTR));
    __asm {
        sgdt gdtr;
    }

    // �жϿ������ݵĴ�С
    ULONG gdt_limit = gdtr.limit + 1;
    ULONG copy_size = output_size > gdt_limit ? gdt_limit : output_size;

    // �ж��ڴ��ַ�Ƿ���Ч
    if (!MmIsAddressValid((PVOID)gdtr.address)) {
        // ���ٽ���
        KeLeaveCriticalRegion();
        // ��ԭ���к���
        restore_core_working();
        return 0;
    }

    // �����ڴ�����
    RtlCopyMemory(output, (PVOID)gdtr.address, copy_size);

    // ���ٽ���
    KeLeaveCriticalRegion();

    // ��ԭ���к���
    restore_core_working();

    return copy_size;
}

ULONG_PTR get_idt(DWORD32 core_mark, PVOID output, ULONG output_size)
{
    //KdBreakPoint();
    // ���ù�������ָ����cpu��������
    KeSetSystemAffinityThread(core_mark);

    // �����ٽ���
    KeEnterCriticalRegion();

    // ��ȡIDTR
    struct IDTR idtr;
    __asm {
        sidt idtr
    }

    // �жϿ������ݵĴ�С
    ULONG idt_limit = idtr.limit + 1;
    ULONG copy_size = output_size > idt_limit ? idt_limit : output_size;

    // �ж��ڴ��ַ�Ƿ���Ч
    if (!MmIsAddressValid((PVOID)idtr.address)) {
        // ���ٽ���
        KeLeaveCriticalRegion();
        // ��ԭ���к���
        restore_core_working();
        return 0;

    }

    // �����ڴ�����
    RtlCopyMemory(output, (PVOID)idtr.address, copy_size);

    // ���ٽ���
    KeLeaveCriticalRegion();

    // ��ԭ���к���
    restore_core_working();
    return copy_size;
}

ULONG_PTR get_eprocess(ULONG pid, PULONG output, ULONG output_size)
{
    // ��������EPROCESS
    PEPROCESS eprocess = find_process_by_id(pid);
    if (eprocess == NULL)
        return 0;
    *output = (ULONG)eprocess;
    return output_size;
}

ULONG_PTR get_image_path(ULONG pid, PVOID output, ULONG output_size)
{
    // ��������EPROCESS
    PEPROCESS eprocess = find_process_by_id(pid);

    KAPC_STATE kapc_state;
    // ���ӵ�����
    KeStackAttachProcess(eprocess, &kapc_state);

    // ��ȡPEB
    PPEB peb_ptr = (PPEB)(*(PUINT_PTR)((PUCHAR)eprocess + 0x1a8));
    if (peb_ptr == NULL) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // ��ȡPROCESS_PARAM
    PUINT_PTR user_proc_param = (PUINT_PTR)(*(PUINT_PTR)((PCHAR)peb_ptr + 0x010));
    if (user_proc_param == NULL) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // ��ȡӳ��·��
    PUNICODE_STRING image_path = (PUNICODE_STRING)((PUCHAR)user_proc_param + 0x038);
    if (!MmIsAddressValid(image_path) || !MmIsAddressValid(image_path->Buffer)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // ���㻺������С
    output_size = output_size > image_path->Length ? image_path->Length : output_size;

    // �����ٽ���
    KeEnterCriticalRegion();

    RtlCopyMemory(output, image_path->Buffer, output_size);

    // ���ٽ���
    KeLeaveCriticalRegion();

    // �������
    KeUnstackDetachProcess(&kapc_state);

    return output_size;
}

ULONG_PTR read_process_memory(ULONG pid, PVOID mem_addr, PVOID read_buf, ULONG buf_size)
{
    // ��ȡָ�����̵�EPROCESS
    PEPROCESS eprocess = find_process_by_id(pid);
    // �ҵ�Ŀ¼���ַ
    PVOID old_cr3 = 0;
    PVOID dir_base = (PVOID)(*(PUINT_PTR)((PUCHAR)eprocess + 0x18));

    __asm {
        // �����жϣ���ֹ�߳��л�
        cli

        // ����ԭ����cr3
        mov eax, cr3
        mov old_cr3, eax

        // �޸�cr3
        mov eax, dir_base
        mov cr3, eax
    }

    // ����ڴ��Ƿ���Ч
    if (MmIsAddressValid(mem_addr)) {
        // �����С����������ҳ
        ULONG size = ((ULONG)mem_addr / PAGE_SIZE + 1) * PAGE_SIZE - (ULONG)mem_addr;
        buf_size = buf_size > size ? size : buf_size;
        RtlCopyMemory(read_buf, mem_addr, buf_size);
    }

    __asm {
        // ��ԭcr3
        mov eax, old_cr3
        mov cr3, eax

        // �ָ��ж�
        sti
    }

    return buf_size;
}

ULONG_PTR write_process_memory(ULONG pid, PVOID mem_addr, PVOID write_buf, ULONG buf_size)
{
    // ��ȡָ�����̵�EPROCESS
    PEPROCESS eprocess = find_process_by_id(pid);
    // �ҵ�Ŀ¼���ַ
    PVOID old_cr3 = 0;
    PVOID dir_base = (PVOID)(*(PUINT_PTR)((PUCHAR)eprocess + 0x18));

    __asm {
        // �����жϣ���ֹ�߳��л�
        cli

        // �ر��ڴ�д����
        mov eax, cr0
        and eax, not 10000h
        mov cr0, eax

        // ����ԭ����cr3
        mov eax, cr3
        mov old_cr3, eax

        // �޸�cr3
        mov eax, dir_base
        mov cr3, eax
    }

    // ����ڴ��Ƿ���Ч
    if (MmIsAddressValid(mem_addr)) {
        // �����С����������ҳ
        ULONG size = ((ULONG)mem_addr / PAGE_SIZE + 1) * PAGE_SIZE - (ULONG)mem_addr;
        buf_size = buf_size > size ? size : buf_size;
        RtlCopyMemory(mem_addr, write_buf, buf_size);
    }

    __asm {
        // ��ԭcr3
        mov eax, old_cr3
        mov cr3, eax

        // ��ԭ�ڴ�д����
        mov eax, cr0
        or eax, 10000h
        mov cr0, eax

        // �ָ��ж�
        sti
    }

    return buf_size;
}

ULONG_PTR get_process_module_count(ULONG pid, PULONG output)
{
    NTSTATUS status;
    // ��ȡΪ������API
    pfnZwQueryInformationProcess ZwQueryInformationProcess;
    UNICODE_STRING UtrZwQueryInformationProcessName = RTL_CONSTANT_STRING(L"ZwQueryInformationProcess");
    ZwQueryInformationProcess = (pfnZwQueryInformationProcess)MmGetSystemRoutineAddress(&UtrZwQueryInformationProcessName);

    HANDLE p_handle = 0;
    CLIENT_ID cid = { 0 };
    cid.UniqueProcess = (HANDLE)pid;
    OBJECT_ATTRIBUTES obj;
    InitializeObjectAttributes(&obj, 0, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
    status = ZwOpenProcess(&p_handle, PROCESS_ALL_ACCESS, &obj, &cid);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    // ��ȡbase_info��Ϣ
    PROCESS_BASIC_INFORMATION base_info;
    ULONG ret_bytes = 0;
    status = ZwQueryInformationProcess(p_handle, ProcessBasicInformation, &base_info, sizeof(base_info), &ret_bytes);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    KAPC_STATE kapc_state;
    PEPROCESS eprocess = find_process_by_id(pid);
    // ���ӵ�����
    KeStackAttachProcess(eprocess, &kapc_state);

    // ��ȡPEB
    PPEB peb_ptr = base_info.PebBaseAddress;
    if (!MmIsAddressValid(peb_ptr)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }



    // ��ȡ_PEB_LDR_DATA
    PULONG_PTR peb_ldr_data_ptr = *(PULONG_PTR *)((PUCHAR)peb_ptr + 0x00c);
    if (!MmIsAddressValid(peb_ldr_data_ptr)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // ��ȡ����ģ������
    PLIST_ENTRY list_entry_ptr = *(PLIST_ENTRY *)((PUCHAR)peb_ldr_data_ptr + 0x00c);
    if (!MmIsAddressValid(list_entry_ptr)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // �����ٽ���
    KeEnterCriticalRegion();

    // ��������
    PLIST_ENTRY ptr = list_entry_ptr;
    ULONG count = 0;

    do {
        // �ж��Ƿ���β�ڵ�
        ULONG dll_base = *(PULONG_PTR)((PUCHAR)ptr + 0x018);
        if (dll_base == 0) {
            ptr = ptr->Flink;
            continue;
        }

        count++;
        ptr = ptr->Flink;
    } while (ptr != list_entry_ptr);

    *output = count;

    // ���ٽ���
    KeLeaveCriticalRegion();

    // �������
    KeUnstackDetachProcess(&kapc_state);

    return sizeof(ULONG);
}

ULONG_PTR get_process_module(ULONG pid, PVOID output, ULONG output_size)
{
    
    NTSTATUS status;
    // ��ȡΪ������API
    pfnZwQueryInformationProcess ZwQueryInformationProcess;
    UNICODE_STRING UtrZwQueryInformationProcessName = RTL_CONSTANT_STRING(L"ZwQueryInformationProcess");
    ZwQueryInformationProcess = (pfnZwQueryInformationProcess)MmGetSystemRoutineAddress(&UtrZwQueryInformationProcessName);

    HANDLE p_handle = 0;
    CLIENT_ID cid = { 0 };
    cid.UniqueProcess = (HANDLE)pid;
    OBJECT_ATTRIBUTES obj;
    InitializeObjectAttributes(&obj, 0, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
    status = ZwOpenProcess(&p_handle, PROCESS_ALL_ACCESS, &obj, &cid);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    // ��ȡbase_info��Ϣ
    PROCESS_BASIC_INFORMATION base_info;
    ULONG ret_bytes = 0;
    status = ZwQueryInformationProcess(p_handle, ProcessBasicInformation, &base_info, sizeof(base_info), &ret_bytes);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    KAPC_STATE kapc_state;
    PEPROCESS eprocess = find_process_by_id(pid);
    // ���ӵ�����
    KeStackAttachProcess(eprocess, &kapc_state);

    // ��ȡPEB
    PPEB peb_ptr = base_info.PebBaseAddress;
    if (!MmIsAddressValid(peb_ptr)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }


    // ��ȡ_PEB_LDR_DATA
    PULONG_PTR peb_ldr_data_ptr = *(PULONG_PTR *)((PUCHAR)peb_ptr + 0x00c);
    if (!MmIsAddressValid(peb_ldr_data_ptr)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // ��ȡ����ģ������
    PLIST_ENTRY list_entry_ptr = *(PLIST_ENTRY *)((PUCHAR)peb_ldr_data_ptr + 0x00c);
    if (!MmIsAddressValid(list_entry_ptr)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    // �����ٽ���
    KeEnterCriticalRegion();

    // ��������
    PLIST_ENTRY ptr = list_entry_ptr;
    ULONG copy_size = 0;

    struct ModuleItem *module_ptr = (struct ModuleItem *)output;
    do {
        // �ж��Ƿ���β�ڵ�
        ULONG dll_base = *(PULONG_PTR)((PUCHAR)ptr + 0x018);
        if (dll_base == 0) {
            ptr = ptr->Flink;
            continue;
        }

        // �ж��Ƿ��������������
        if (copy_size >= output_size)
            break;

        // �ж��ڴ��Ƿ���Ч
        if (!MmIsAddressValid((PVOID)module_ptr))
            break;

        // ��ȡģ��·��
        PUNICODE_STRING full_name = (PUNICODE_STRING)((PUCHAR)ptr + 0x024);
        if(MmIsAddressValid(full_name->Buffer)) {
            RtlCopyMemory(module_ptr->path, full_name->Buffer, full_name->MaximumLength);      // ����·��
            module_ptr->base = *(PULONG)((PUCHAR)ptr + 0x018);      // ��ȡģ���ַ
            module_ptr->size = *(PULONG)((PUCHAR)ptr + 0x020);      // ��ȡģ���С
            module_ptr++;
            copy_size += sizeof(struct ModuleItem);
        }

        ptr = ptr->Flink;
    } while (ptr != list_entry_ptr);


    // ���ٽ���
    KeLeaveCriticalRegion();

    // �������
    KeUnstackDetachProcess(&kapc_state);

    return copy_size;
}

ULONG_PTR get_process_thread_count(ULONG pid, PULONG output)
{
    KAPC_STATE kapc_state;

    // �ҵ�eprocess
    PEPROCESS eprocess = find_process_by_id(pid);

    // ��ȡPEB
    PPEB peb_ptr = (PPEB)(*(PUINT_PTR)((PUCHAR)eprocess + 0x1a8));
    if (peb_ptr == NULL) {
        return 0;
    }

    // ���ӵ�����
    KeStackAttachProcess(eprocess, &kapc_state);

    ULONG count = 0;

    // ��ȡ�̵߳�ThreadListHead
    PLIST_ENTRY list_entry = (PLIST_ENTRY)((PUCHAR)eprocess + 0x02c);
    if (!MmIsAddressValid((PVOID)list_entry)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    PLIST_ENTRY traversal = list_entry;
    // ����
    do {
        // ��ȡethread
        PETHREAD ethread = (PETHREAD)((PUCHAR)traversal - 0x1e0);
        // ��ȡteb
        PVOID teb = *(PVOID *)((PUCHAR)list_entry - 0x158);
        // �ж��Ƿ���Ч
        if (MmIsAddressValid((PVOID)ethread)) {
                count++;
        }
        // ָ����һ���ڵ�
        traversal = traversal->Flink;
    } while (traversal != list_entry);

    // �������
    KeUnstackDetachProcess(&kapc_state);

    *output = count;

    return sizeof(count);
}

ULONG_PTR get_process_thread(ULONG pid, PVOID output, ULONG output_size)
{
    KAPC_STATE kapc_state;

    // �ҵ�eprocess
    PEPROCESS eprocess = find_process_by_id(pid);

    // ��ȡ��ǰtid
    ULONG tid = (ULONG)PsGetCurrentThreadId();

    // ��ȡPEB
    PPEB peb_ptr = (PPEB)(*(PUINT_PTR)((PUCHAR)eprocess + 0x1a8));
    if (peb_ptr == NULL) {
        return 0;
    }

    // ���ӵ�����
    KeStackAttachProcess(eprocess, &kapc_state);

    ULONG count = 0;

    // ��ȡ�̵߳�ThreadListHead
    PLIST_ENTRY list_entry = (PLIST_ENTRY)((PUCHAR)eprocess + 0x02c);
    if (!MmIsAddressValid((PVOID)list_entry)) {
        // �������
        KeUnstackDetachProcess(&kapc_state);
        return 0;
    }

    PLIST_ENTRY traversal = list_entry;
    struct ThreadItem *ptr = (struct ThreadItem *)output;
    ULONG copy_size = 0;

    // ����
    do {
        if (copy_size >= output_size)
            break;
        // ��ȡethread
        PETHREAD ethread = (PETHREAD)((PUCHAR)traversal - 0x1e0);
        // ��ȡtab
        PVOID teb = *(PVOID *)((PUCHAR)list_entry - 0x158);
        // ��ȡpid
        CLIENT_ID cid = *(PCLIENT_ID)((PUCHAR)ethread + 0x22c);
        // �ж��Ƿ���Ч
        if (MmIsAddressValid((PVOID)ethread)) {
            ptr->ethread = (unsigned int)ethread;
            ptr->teb = (unsigned int)teb;
            ptr->pid = (unsigned int)cid.UniqueThread;
            copy_size += sizeof(struct ThreadItem);
            ptr++;
        }
        // ָ����һ���ڵ�
        traversal = traversal->Flink;
    } while (traversal != list_entry);

    // �������
    KeUnstackDetachProcess(&kapc_state);

    return copy_size;
}

ULONG_PTR get_ssdt_count(PULONG output, ULONG output_size)
{
    // ��ȡSSDT��λ��
    SSDT *ssdt = (SSDT *)KeServiceDescriptorTable;

    if (!MmIsAddressValid((PVOID)ssdt))
        return 0;
    *output = ssdt->count;
    return output_size;
}

ULONG_PTR get_ssdt(PVOID output, ULONG output_size)
{
    // ��ȡSSDT
    SSDT *ssdt = (SSDT *)KeServiceDescriptorTable;

    if (!MmIsAddressValid((PVOID)ssdt) && !MmIsAddressValid((PVOID)(ssdt->func_arr_ptr)))
        return 0;

    // ����
    SSDTItem *ptr = (SSDTItem *)output;
    for(ULONG i = 0; i < output_size / sizeof(SSDTItem); i++, ptr++) {
        ptr->num = i;
        ptr->addr = ((PULONG)(ssdt->func_arr_ptr))[i];
    }

    return output_size;
}

ULONG_PTR get_shadowssdt_count(PULONG output, ULONG output_size)
{
    // ��ȡShadowSSDTλ��
    ShadowSSDT *shadow_ssdt = (ShadowSSDT *)(KeServiceDescriptorTable + 0x50);

    if (!MmIsAddressValid((PVOID)shadow_ssdt))
        return 0;
    *output = shadow_ssdt->count;
    return output_size;
}

ULONG_PTR get_shadowssdt(PVOID output, ULONG output_size)
{
    // ��ȡShadowSSDT
    ShadowSSDT *shadow_ssdt = (ShadowSSDT *)(KeServiceDescriptorTable + 0x50);

    if (!MmIsAddressValid((PVOID)shadow_ssdt) && !MmIsAddressValid((PVOID)(shadow_ssdt->func_arr_ptr)))
        return 0;

    // ����
    ShadowSSDTItem *ptr = (ShadowSSDTItem *)output;
    for (ULONG i = 0; i < output_size / sizeof(ShadowSSDTItem); i++, ptr++) {
        ptr->num = i;
        ptr->addr = ((PULONG)(shadow_ssdt->func_arr_ptr))[i];
    }

    return output_size;
}

ULONG_PTR get_driver_module_count(PDRIVER_OBJECT pDriverObject, PULONG output)
{
    LDR_DATA_TABLE_ENTRY *pDataTableEntry, *pTempDataTableEntry;

    // ˫��ѭ������
    PLIST_ENTRY pList;

    // ָ�����������DriverSection
    pDataTableEntry = (LDR_DATA_TABLE_ENTRY*)pDriverObject->DriverSection;

    // �ж��Ƿ�Ϊ��
    if (!pDataTableEntry)
        return 0;

    ULONG count = 0;

    // ��ʼ����������������
    pList = pDataTableEntry->InLoadOrderLinks.Flink;
    while (pList != &pDataTableEntry->InLoadOrderLinks) {
        pTempDataTableEntry = (LDR_DATA_TABLE_ENTRY *)pList;
        count++;
        pList = pList->Flink;
    }

    *output = count;

    return sizeof(count);
}

ULONG_PTR get_driver_module(PDRIVER_OBJECT pDriverObject, PVOID output, ULONG output_size)
{
    LDR_DATA_TABLE_ENTRY *pDataTableEntry, *pTempDataTableEntry;

    // ˫��ѭ������
    PLIST_ENTRY pList;

    // ָ�����������DriverSection
    pDataTableEntry = (LDR_DATA_TABLE_ENTRY*)pDriverObject->DriverSection;

    // �ж��Ƿ�Ϊ��
    if (!pDataTableEntry)
        return 0;

    ULONG count = 0;
    ULONG copy_bytes = 0;
    struct DriverModule *ptr = (struct DriverModule *)output;

    // ��ʼ����������������
    pList = pDataTableEntry->InLoadOrderLinks.Flink;
    while (pList != &pDataTableEntry->InLoadOrderLinks) {
        pTempDataTableEntry = (LDR_DATA_TABLE_ENTRY *)pList;

        if(MmIsAddressValid(pTempDataTableEntry->BaseDllName.Buffer) && pTempDataTableEntry->BaseDllName.Buffer) {
            // ������
            RtlCopyMemory(ptr->name, pTempDataTableEntry->BaseDllName.Buffer, pTempDataTableEntry->BaseDllName.MaximumLength);
            // ��ַ
            ptr->base = (unsigned int)pTempDataTableEntry->DllBase;
            // ��С
            ptr->size = pTempDataTableEntry->SizeOfImage;
            // ����·��
            RtlCopyMemory(ptr->path, pTempDataTableEntry->FullDllName.Buffer, pTempDataTableEntry->FullDllName.MaximumLength);
            ptr++;
            copy_bytes += sizeof(struct DriverModule);
        }

        if (copy_bytes >= output_size)
            break;

        pList = pList->Flink;
    }

    return copy_bytes;
}

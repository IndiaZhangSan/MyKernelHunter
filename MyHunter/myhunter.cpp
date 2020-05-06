#include "myhunter.h"
#include <QCoreApplication>
#include <TlHelp32.h>
#include "msgboxunit.h"
#include <QDebug>

MyHunter::MyHunter(QObject *parent) : QObject(parent)
{
    driver_manager_ = new DriverManager();              // ��������������
    model_tableView_ = new QStandardItemModel(this);    // ����tableViewģ��
    model_child_tableView_ = new QStandardItemModel(this);  // �����Ӵ��ڵ�tableViewģ��
    cpu_core_count_ = QThread::idealThreadCount();      // ��ȡcpu��������

    // �����źŲ�
    connect(this, static_cast<void (MyHunter::*)(QStringList)>(&MyHunter::tableview_append),
        this, [=](QStringList labels) {
            // tableViewģ�����һ����Ϣ
            QList<QStandardItem *> items;
            for(auto &item : labels) {
                items.push_back(new QStandardItem(item));
            }
            model_tableView_->appendRow(items);
        });

    connect(this, static_cast<void (MyHunter::*)(QStringList)>(&MyHunter::child_tableview_append),
        this, [=](QStringList labels) {
            // tableViewģ�����һ����Ϣ
            QList<QStandardItem *> items;
            for(auto &item : labels) {
                items.push_back(new QStandardItem(item));
            }
            model_child_tableView_->appendRow(items);
        });
}

MyHunter::~MyHunter()
{
    if (driver_manager_)
        delete driver_manager_;
}

bool MyHunter::initialization()
{
    // ��ȡ��ģ��·��
    QString driver_path = QCoreApplication::applicationDirPath();
    if(driver_path[driver_path.length()] == "/") {
        driver_path += driver_name_;
    } else {
        driver_path += ("/" + driver_name_);
    }

#ifndef _DEBUG
    // ��������������·��
    driver_manager_->set_driver(QStringToTCHAR(driver_path));

    // ��װ����
    if(!driver_manager_->install_driver()) {
        MsgBoxUnit::msgbox_critical(QString("DriverManager Error"), 
            QString("[install_driver] error: %1").arg((ulong)driver_manager_->get_last_error()));
        return false;
    }

    // ��������
    if (!driver_manager_->start_driver()) {
        MsgBoxUnit::msgbox_critical(QString("DriverManager Error"),
            QString("[start_driver] error: %1").arg((ulong)driver_manager_->get_last_error()));
        return false;
    }

    // ��������
    if (!driver_manager_->connect_driver(MY_SYMBOL_FILE)) {
        MsgBoxUnit::msgbox_critical(QString("DriverManager Error"),
            QString("[connect_driver] error: %1").arg((ulong)driver_manager_->get_last_error()));
        return false;
    }
#endif

    return true;
}

bool MyHunter::uninitialization()
{
#ifndef _DEBUG
    // �ر�����
    driver_manager_->disconnect_driver();

    // ֹͣ����
    if(!driver_manager_->stop_driver()) {
        MsgBoxUnit::msgbox_critical(QString("DriverManager Error"),
            QString("[stop_driver] error: %1").arg((ulong)driver_manager_->get_last_error()));
        return false;
    }

    // ж������
    if (!driver_manager_->uninstall_driver()) {
        MsgBoxUnit::msgbox_critical(QString("DriverManager Error"),
            QString("[uninstall_driver] error: %1").arg((ulong)driver_manager_->get_last_error()));
        return false;
    }
#endif

    return true;
}

QStandardItemModel * MyHunter::get_tableView_model() const
{
    return model_tableView_;
}

QStandardItemModel * MyHunter::get_child_tableView_model() const
{
    return model_child_tableView_;
}

void MyHunter::tableView_model_clear()
{
    model_tableView_->removeRows(0, model_tableView_->rowCount());
}

void MyHunter::tableView_model_clearall()
{
    model_tableView_->clear();
}

void MyHunter::tableView_child_model_clear()
{
    model_child_tableView_->removeRows(0, model_child_tableView_->rowCount());
}

void MyHunter::tableView_child_model_clearall()
{
    model_child_tableView_->clear();
}

void MyHunter::set_tableView_header(QStringList &string_list)
{
    if (!model_tableView_)
        return;
    model_tableView_->setHorizontalHeaderLabels(string_list);
}

void MyHunter::set_child_tableView_header(QStringList &string_list)
{
    if (!model_child_tableView_)
        return;
    model_child_tableView_->setHorizontalHeaderLabels(string_list);
}

QString MyHunter::get_module_index_text(QStandardItemModel *module, QModelIndex model_index, unsigned col)
{
    return module->index(model_index.row(), col).data().toString();
}

unsigned MyHunter::get_gdt(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    int core_count = my_hunter->cpu_core_count_; // cpu��������
    DWORD mark = 0;     // cpu���ĵĹ�������
    
    for(int i = 0; i < core_count; i++) {
        // ���ù�������
        mark = (1 << i);

        // ���������Ϳ�����Ϣ
        DWORD ret_bytes = 0;

        // ��ȡ�����������С������Ϊ�޶���������
        DWORD buf_size = 0;
        driver_manager->io_control(CODE_GET_GDT_BUFSIZE, &mark, sizeof(mark), &buf_size, sizeof(buf_size), &ret_bytes);
        if(buf_size != 0) {
            // �������������
            BYTE *buf = new BYTE[buf_size];

            // ��ȡGDT������Ϊ�޶���������
            driver_manager->io_control(CODE_GET_GDT, &mark, sizeof(mark), buf, buf_size, &ret_bytes);

            // ����GDT����
            if(ret_bytes != 0) {
                my_hunter->deal_gdt_data(buf, ret_bytes / sizeof(SegmentDescriptor), i);
            }

            // �ͷ�����������ռ�
            delete[] buf;
        }
    }


    return 0;
}

void MyHunter::deal_gdt_data(BYTE *buf, DWORD item_count, DWORD cpu_core)
{
    QStringList labels;
    QString core_num = QString("%1").arg((uint)cpu_core);   // ��ȡcpu���ı��

    // ����dgt
    SegmentDescriptor *seg_des_ptr = (SegmentDescriptor *)buf;
    for(DWORD i = 0; i < item_count; i++, seg_des_ptr++) {
        // �ж϶��������Ƿ���Ч
        if(seg_des_ptr->SegDesHigh.p == 1) {
            labels.clear();

            // ���cpu���ı��
            labels << core_num;

            // ��ѡ����
            labels << "0x" + QString("%1").arg((uint)i, 4, 16, QLatin1Char('0')).toUpper();

            //�λ�ַ
            labels << "0x" + QString("%1").arg((uint)get_segdes_base(seg_des_ptr), 8, 16, QLatin1Char('0')).toUpper();

            //�ν���
            labels << "0x" + QString("%1").arg((uint)get_segdes_limit(seg_des_ptr), 8, 16, QLatin1Char('0')).toUpper();

            // ������
            if(seg_des_ptr->SegDesHigh.g == 1) {
                labels << "Page";
            } else {
                labels << "Byte";
            }

            // ��Ȩ��
            labels << QString("%1").arg((uint)seg_des_ptr->SegDesHigh.dpl);

            // ����
            QString seg_type;
            static const char *type_lab[] = {
                "Data RO",              // 0    Data    Read-Only
                "Data RO Ac",           // 1    Data    Read-Only, accessed
                "Data RW",              // 2    Data    Read/Write
                "Data RW Ac",           // 3    Data    Read/Write, accessed
                "Data RO Ex",           // 4    Data    Read-Only, expand-down
                "Data RO Ex Ac",        // 5    Data    Read-Only, expand-down, accessed
                "Data RW Ex",           // 6    Data    Read/Write, expand-down
                "Data RW Ex Ac",        // 7    Data    Read/Write, expand-down, accessed
                "Code EO",              // 8    Code    Execute-Only
                "Code EO Ac",           // 9    Code    Execute-Only, accessed
                "Code ER",              // 10   Code    Execute/Read
                "Code ER Ac",           // 11   Code    Execute/Read, accessed
                "Code EO Con",          // 12   Code    Execute-Only, conforming
                "Code EO Con Ac",       // 13   Code    Execute-Only, conforming, accessed
                "Code ER Con",          // 14   Code    Execute/Read, conforming
                "Code ER Con Ac",       // 15   Code    Execute/Read, conforming, accessed
            };
            // �ж��Ǵ洢�λ���ϵͳ��
            if(seg_des_ptr->SegDesHigh.s == 1) {
                seg_type = type_lab[seg_des_ptr->SegDesHigh.type];
            } else {
                seg_type = "Sys Segment";
            }

            // ִ����Ӳ���
            labels << seg_type;
            emit tableview_append(labels);
        }
    }
}

unsigned MyHunter::get_idt(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    int core_count = my_hunter->cpu_core_count_; // cpu��������
    DWORD mark = 0;     // cpu���ĵĹ�������

    for (int i = 0; i < core_count; i++) {
        // ���ù�������
        mark = (1 << i);

        // ���������Ϳ�����Ϣ
        DWORD ret_bytes = 0;
        BYTE buf[0x800] = { 0 };
        // ��ȡIDT������Ϊ�޶���������
        driver_manager->io_control(CODE_GET_IDT, &mark, sizeof(mark), buf, 0x800, &ret_bytes);

        // ����IDT����
        if(ret_bytes != 0) {
            my_hunter->deal_idt_data(buf, ret_bytes / sizeof(GateDescriptor), i);
        }
    }


    return 0;
}

void MyHunter::deal_idt_data(BYTE *buf, DWORD item_count, DWORD cpu_core)
{
    QStringList labels;
    QString core_num = QString("%1").arg((uint)cpu_core);   // ��ȡcpu���ı��

    // ����IDT
    GateDescriptor *gate_descriptor = (GateDescriptor *)buf;
    for(DWORD i = 0; i < item_count; i++, gate_descriptor++) {
        // �ж����������Ƿ���ڣ��ж��Ƿ���ϵͳ��
        if(gate_descriptor->GateDesHigh.p == 1 && gate_descriptor->GateDesHigh.s == 0) {
            labels.clear();

            // ���cpu���ı��
            labels << core_num;

            // ��ȡ�����
            labels << QString("%1").arg((uint)i, 2, 16, QLatin1Char('0')).toUpper();

            // ��ȡѡ����
            USHORT tmp = gate_descriptor->GateDesLow.seg_sel;
            SegmentSelector *seg_sel = (SegmentSelector *)&tmp;
            labels << "0x" + QString("%1").arg((uint)seg_sel->index, 2, 16, QLatin1Char('0')).toUpper();

            // ��ȡ������ַ
            DWORD offset_0_15 = gate_descriptor->GateDesLow.offset_0_15;
            DWORD offset_16_31 = gate_descriptor->GateDesHigh.offset_16_31;
            offset_16_31 <<= 16;
            DWORD offset = offset_0_15 | offset_16_31;
            labels << "0x" + QString("%1").arg((uint)offset, 8, 16, QLatin1Char('0')).toUpper();

            // ִ����Ӳ���
            emit tableview_append(labels);
        }
    }
}

unsigned MyHunter::get_ssdt(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    // ���������Ϳ�����Ϣ
    DWORD ret_bytes = 0;
    ULONG count = 0;

    // ��ȡSSDT��Ŀ����
    driver_manager->io_control(CODE_SSDT_COUNT, NULL, 0, &count, sizeof(count), &ret_bytes);
    if(ret_bytes) {
        // ����ռ�
        SSDTItem *ssdt_items = new SSDTItem[count];

        // ��ȡSSDT
        driver_manager->io_control(CODE_SSDT, NULL, 0, ssdt_items, sizeof(SSDTItem) * count, &ret_bytes);
        if(ret_bytes) {
            // ��������
            for(ULONG i = 0; i < ret_bytes / sizeof(SSDTItem); i++) {
                QStringList labels;
                labels << QString::number(ssdt_items[i].num);
                labels << "0x" + QString("%1").arg((ulong)ssdt_items[i].addr, 8, 16, QLatin1Char('0')).toUpper();
                emit my_hunter->tableview_append(labels);
            }
        }

        delete[] ssdt_items;
    }
    return 0;
}

unsigned MyHunter::get_shadow_ssdt(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    // ���������Ϳ�����Ϣ
    DWORD ret_bytes = 0;
    ULONG count = 0;

    // ��ȡShadowSSDT��Ŀ����
    driver_manager->io_control(CODE_SHADOWSSDT_COUNT, NULL, 0, &count, sizeof(count), &ret_bytes);
    if (ret_bytes) {
        // ����ռ�
        ShadowSSDTItem *shadowssdt_items = new ShadowSSDTItem[count];

        // ��ȡShadowSSDT
        driver_manager->io_control(CODE_SHADOWSSDT, NULL, 0, shadowssdt_items, sizeof(ShadowSSDTItem) * count, &ret_bytes);
        if (ret_bytes) {
            // ��������
            for (ULONG i = 0; i < ret_bytes / sizeof(ShadowSSDTItem); i++) {
                QStringList labels;
                labels << QString::number(shadowssdt_items[i].num);
                labels << "0x" + QString("%1").arg((ulong)shadowssdt_items[i].addr, 8, 16, QLatin1Char('0')).toUpper();
                emit my_hunter->tableview_append(labels);
            }
        }

        delete[] shadowssdt_items;
    }
    return 0;
}

unsigned MyHunter::get_driver_module(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    // ���������Ϳ�����Ϣ
    DWORD ret_bytes = 0;
    ULONG count = 0;

    // ��ȡ����ģ�����
    driver_manager->io_control(CODE_DRIVER_MODULE_COUNT, NULL, 0, &count, sizeof(count), &ret_bytes);
    if(ret_bytes) {
        // ����ռ�
        DriverModule *driver_modules = new DriverModule[count];
        DWORD buf_size = sizeof(DriverModule) * count;
        memset(driver_modules, 0, buf_size);

        // ��ȡ����ģ��
        driver_manager->io_control(CODE_DRIVER_MODULE, NULL, 0, driver_modules, buf_size, &ret_bytes);
        if(ret_bytes) {
            // ��������
            for(DWORD i = 0; i < ret_bytes / sizeof(DriverModule); i++) {
                QStringList labels;
                // ������
                labels << TCHARToQString(driver_modules[i].name);
                // ��ַ
                labels << "0x" + QString("%1").arg((uint)driver_modules[i].base, 8, 16, QLatin1Char('0')).toUpper();
                // ��С
                labels << "0x" + QString("%1").arg((uint)driver_modules[i].size, 8, 16, QLatin1Char('0')).toUpper();
                // ����·��
                labels << TCHARToQString(driver_modules[i].path);
                emit my_hunter->tableview_append(labels);
            }
        }
        delete[] driver_modules;
    }

    return 0;
}

unsigned MyHunter::get_process_list(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������
    QStringList labels;

    // ��������
    HANDLE process_snap;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (process_snap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (!Process32First(process_snap, &pe32)) {
        return 0;
    }

    // ��ȡ������Ϣ
    do {
        labels.clear();

        // �ж�PID�Ƿ�Ϊ0
        if(pe32.th32ProcessID == 0)
            continue;

        labels << TCHARToQString(pe32.szExeFile);                       // ��ȡ������
        labels << QString("%1").arg((uint)pe32.th32ProcessID);          // ��ȡPID
        labels << QString("%1").arg((uint)pe32.th32ParentProcessID);    // ��ȡParent PID

        ULONG ret_bytes = 0;

        // ��ȡӳ��·��
        TCHAR image_path[1024] = { 0 };
        driver_manager->io_control(CODE_GET_IMAGE_PATH, &pe32.th32ProcessID, sizeof(pe32.th32ProcessID), image_path, 1024, &ret_bytes);
        if (ret_bytes) {
            // ��r0��ȡ������
            labels << TCHARToQString(image_path);
        } else {
            // û�д�r0��ȡ������
            QString module_name = TCHARToQString(pe32.szExeFile);;
            // ��ȡ��һ��ģ��
            HANDLE module_snap = INVALID_HANDLE_VALUE;
            MODULEENTRY32 me32;
            me32.dwSize = sizeof(MODULEENTRY32);
            module_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
            if (module_snap != INVALID_HANDLE_VALUE) {
                if(Module32First(module_snap, &me32) && me32.th32ProcessID == pe32.th32ProcessID) {
                    module_name = TCHARToQString(me32.szExePath);
                }
                ::CloseHandle(module_snap);
            }
            labels << module_name;
        }
            
        // ��ȡEPROCESS
        ULONG eprocess = 0;
        ret_bytes = 0;
        driver_manager->io_control(CODE_GET_EPROCESS, &pe32.th32ProcessID, sizeof(pe32.th32ProcessID), 
            &eprocess, sizeof(eprocess), &ret_bytes);
        if (ret_bytes != 0)
            labels << "0x" + QString("%1").arg((ulong)eprocess, 8, 16, QLatin1Char('0')).toUpper();
        else
            labels << "Unknown";

        // ��ȡ��Ȩ��
        labels << "-";

        // ִ����Ӳ���
        emit my_hunter->tableview_append(labels);
    } while (Process32Next(process_snap, &pe32));
    CloseHandle(process_snap);

    return 0;
}

unsigned MyHunter::get_process_module_list(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    QStringList labels;

    DWORD module_count = 0, ret_bytes = 0;

    // ��ȡģ���б�Ԫ�ظ���
    driver_manager->io_control(CODE_GET_PROCESS_MODULE_COUNT, &my_hunter->param_pid, sizeof(my_hunter->param_pid), 
        &module_count, sizeof(module_count), &ret_bytes);

    if(ret_bytes) {
        // ���仺�����ռ�
        ModuleItem *module_items = new ModuleItem[module_count];
        memset(module_items, 0, sizeof(ModuleItem) * module_count);
        // ��ȡ����ģ���б�
        driver_manager->io_control(CODE_GET_PROCESS_MODULE, &my_hunter->param_pid, sizeof(my_hunter->param_pid),
            module_items, sizeof(ModuleItem) * module_count, &ret_bytes);
        if(ret_bytes) {
            for(DWORD i = 0; i < ret_bytes / sizeof(ModuleItem); i++) {
                labels.clear();
                auto &item = module_items[i];
                labels << TCHARToQString(item.path);    // ·��
                labels << "0x" + QString("%1").arg((uint)item.base, 8, 16, QLatin1Char('0'));   // ��ַ
                labels << "0x" + QString("%1").arg((uint)item.size, 8, 16, QLatin1Char('0'));   // ��С

                // �����Ϣ
                emit my_hunter->child_tableview_append(labels);
            }
        }
        // �ͷŻ�����
        delete[] module_items;
    }

    // ���Ӵ�������ģ��
    emit my_hunter->child_wnd_set_modle();

    return 0;
}

unsigned MyHunter::get_process_thread_list(void *arg)
{
    MyHunter *my_hunter = (MyHunter *)arg;
    DriverManager *driver_manager = my_hunter->driver_manager_; // ����������

    DWORD thread_count = 0, ret_bytes = 0;

    // ��ȡ���̵��̸߳���
    driver_manager->io_control(CODE_GET_PROCESS_THREAD_COUNT, &my_hunter->param_pid, sizeof(my_hunter->param_pid),
        &thread_count, sizeof(thread_count), &ret_bytes);

    if(ret_bytes) {
        // ���仺�����ռ�
        ThreadItem *thread_items = new ThreadItem[thread_count];

        // ��ȡָ�����̵��߳�
        driver_manager->io_control(CODE_GET_PROCESS_THREAD, &my_hunter->param_pid, sizeof(my_hunter->param_pid),
            thread_items, thread_count * sizeof(ThreadItem), &ret_bytes);

        if(ret_bytes) {
            for(DWORD i = 0; i < ret_bytes / sizeof(ThreadItem); i++) {
                QStringList labels;
                // ��ȡpid
                labels << QString::number(thread_items[i].pid);
                // ��ȡETHREAD
                labels << "0x" + QString("%1").arg((uint)thread_items[i].ethread, 8, 16, QLatin1Char('0')).toUpper();
                // ��ȡteb
                labels << "0x" + QString("%1").arg((uint)thread_items[i].teb, 8, 16, QLatin1Char('0')).toUpper();

                // �����Ϣ
                emit my_hunter->child_tableview_append(labels);
            }
        }

        // �ͷŻ�����
        delete[] thread_items;
    }

    // ���Ӵ�������ģ��
    emit my_hunter->child_wnd_set_modle();

    return 0;
}

unsigned MyHunter::get_process_handle_list(void *arg)
{
    return 0;
}

DWORD MyHunter::get_segdes_base(SegmentDescriptor *segment_descriptor)
{
    // ƴ���ֶ�
    DWORD base_0_15 = segment_descriptor->SegDesLow.base_0_15;
    DWORD base_16_23 = segment_descriptor->SegDesHigh.base_16_23;
    DWORD base_24_31 = segment_descriptor->SegDesHigh.base_24_31;
    DWORD base = 0;
    base_24_31 <<= 24;
    base_16_23 <<= 16;
    base = base_0_15 | base_16_23 | base_24_31;
    return base;
}

DWORD MyHunter::get_segdes_limit(SegmentDescriptor *segment_descriptor, bool is_comput)
{
    // ƴ���ֶ�
    DWORD limit_0_15 = segment_descriptor->SegDesLow.limit_0_15;
    DWORD limit_16_19 = segment_descriptor->SegDesHigh.limit_16_19;
    limit_16_19 <<= 16;
    DWORD limit = 0;
    limit = limit_0_15 | limit_16_19;

    // �Ƿ�ֱ�Ӽ���
    if(is_comput) {
        // �ж����ȵ�λ
        if (segment_descriptor->SegDesHigh.g == 1) {
            // ��4kΪ��λ
            limit = limit * 4096;
        }
        limit += 0xfff;
    }
    
    return limit;
}

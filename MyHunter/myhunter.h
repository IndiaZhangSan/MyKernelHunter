#ifndef MYHUNTER_H
#define MYHUNTER_H

#include <QObject>
#include "drivermanager.h"
#include <QString>
#include <QStandardItemModel>
#include <QThread>
#include <QModelIndex>
#include <winioctl.h>
#include "../common/protocol.h"
#include "tableviewdialog.h"

// �ַ�ת����
#ifdef UNICODE
#define QStringToTCHAR(x)     (wchar_t*) (x).utf16()
#define PQStringToTCHAR(x)    (wchar_t*) (x)->utf16()
#define TCHARToQString(x)     QString::fromUtf16((ushort *)(x))
#define TCHARToQStringN(x,y)  QString::fromUtf16((x),(y))
#else
#define QStringToTCHAR(x)     (x).local8Bit().constData()
#define PQStringToTCHAR(x)    (x)->local8Bit().constData()
#define TCHARToQString(x)     QString::fromLocal8Bit((x))
#define TCHARToQStringN(x,y)  QString::fromLocal8Bit((x),(y))
#endif

// model�к�
#define M_PROCESS_NAME 0        // ������
#define M_PROCESS_PID 1         // ����ID
#define M_PROCESS_PPID 2        // ������ID
#define M_PROCESS_IMAGE_PATH 3  // ӳ��·��
#define M_PROCESS_EPROCESS 4    // EPROCESS
#define M_PROCESS_PRIVILEGE 5   // ��Ȩ��

class MyHunter : public QObject
{
    Q_OBJECT
public:
    explicit MyHunter(QObject *parent = nullptr);
    virtual ~MyHunter();

    bool initialization();      // ��ʼ��
    bool uninitialization();    // ����ʼ��

    QStandardItemModel *get_tableView_model() const;    // ��ȡtableViewģ��
    QStandardItemModel *get_child_tableView_model() const;  // ��ȡ�Ӵ��ڵ�tableViewģ��
    void tableView_model_clear();       // ���tableViewģ������
    void tableView_model_clearall();    // ���tableViewģ���������ݣ�������ͷ��
    void tableView_child_model_clear();         // ����Ӵ���tableView��ģ������
    void tableView_child_model_clearall();      // ����Ӵ��ڵ�tableViewģ���������ݣ�������ͷ��

    void set_tableView_header(QStringList &string_list);            // ����tableViewģ�ͱ�ͷ
    void set_child_tableView_header(QStringList &string_list);      // �����Ӵ��ڵ�tableViewģ�ͱ�ͷ

    QString get_module_index_text(QStandardItemModel *module, QModelIndex model_index, unsigned col = 0);    // ��ȡָ��ģ�͵�ǰѡ�����ĵ�col���ı�


    static unsigned __stdcall get_gdt(void *arg);       // ��ȡGDT����
    void deal_gdt_data(BYTE *buf, DWORD item_count, DWORD cpu_core);    // ����GDT����

    static unsigned __stdcall get_idt(void *arg);       // ��ȡIDT����
    void deal_idt_data(BYTE *buf, DWORD item_count, DWORD cpu_core);    // ����IDT����

    static unsigned __stdcall get_ssdt(void *arg);          // ��ȡSSDT
    static unsigned __stdcall get_shadow_ssdt(void *arg);   // ��ȡShadowSSDT
    static unsigned __stdcall get_driver_module(void *arg); // ��ȡ����ģ��

    static unsigned __stdcall get_process_list(void *arg);      // ��ȡ�����б�
    static unsigned __stdcall get_process_module_list(void *arg);   // ��ȡ����ģ���б�
    static unsigned __stdcall get_process_thread_list(void *arg);   // ��ȡ�����߳��б�
    static unsigned __stdcall get_process_handle_list(void *arg);   // ��ȡ���̾���б�


    // �������
    DWORD param_pid = 0;

protected:
    DWORD get_segdes_base(SegmentDescriptor *segment_descriptor);       // ��ȡ���������еĶλ�ַ
    DWORD get_segdes_limit(SegmentDescriptor *segment_descriptor, bool is_comput = false);      // ��ȡ���������еĶν���

signals:
    void tableview_append(QStringList labels);
    void child_tableview_append(QStringList labels);
    void child_wnd_set_modle();

public slots:


private:
    DriverManager *driver_manager_ = nullptr;       // ����������
    const QString driver_name_ = MY_DRIVER_FILE;    // �����ļ���

    QStandardItemModel *model_tableView_ = nullptr; // tableViewģ��
    QStandardItemModel *model_child_tableView_ = nullptr;   // �Ӵ��ڵ�tableViewģ��

    
    int cpu_core_count_ = 0;    // cpu��������
};

#endif // MYHUNTER_H

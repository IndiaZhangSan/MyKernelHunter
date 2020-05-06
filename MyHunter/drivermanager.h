#ifndef DRIVERMANAGER_H
#define DRIVERMANAGER_H

#include <Windows.h>

#define SERVICE_NAME TEXT("MyHunter")
#define SERVICE_SHOW SERVICE_NAME

class DriverManager
{
public:
    DriverManager();
    virtual ~DriverManager();

    bool install_driver();      // ��װ����
    bool start_driver();        // ��������
    bool stop_driver();         // ֹͣ����
    bool uninstall_driver();    // ж������

    bool connect_driver(const TCHAR *symbol_name);      // ��������
    void disconnect_driver();

    /*
     * ��������
     *      control_code: ������
     *      in_buf�����뻺����
     *      in_buf_size�����뻺������С
     *      out_buf�����������
     *      out_buf_size�������������С
     *      ret_bytes�������ֽ���
     */
    bool io_control(DWORD control_code, LPVOID in_buf, DWORD in_buf_size, LPVOID out_buf, DWORD out_buf_size, LPDWORD ret_bytes);

    void set_driver(const TCHAR *path);   // ��������·��

    inline DWORD get_last_error() const { return last_error_; } // ��ȡ������

private:
    TCHAR *driver_path = nullptr;       // ����·��
    SC_HANDLE scm_manager_ = nullptr;   // ������������
    SC_HANDLE service_ = nullptr;       // ������
    HANDLE driver_ = INVALID_HANDLE_VALUE;  // �������

    DWORD last_error_ = 0;  // ������
};

#endif // DRIVERMANAGER_H

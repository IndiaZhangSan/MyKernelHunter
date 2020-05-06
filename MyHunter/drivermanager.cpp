#include "drivermanager.h"
#include <tchar.h>

DriverManager::DriverManager()
{

}

DriverManager::~DriverManager()
{
    // ������Դ
    if (driver_path)
        delete[] driver_path;
    if (service_)
        ::CloseServiceHandle(service_);
    if (scm_manager_)
        ::CloseServiceHandle(scm_manager_);
}

bool DriverManager::install_driver()
{
    // ������ָ��������ϵķ�����ƹ����������ӣ�����ָ���ķ�����ƹ��������ݿ�
    scm_manager_ = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(!scm_manager_) {
        // ��ȡ������
        last_error_ = ::GetLastError();
        return false;
    }

    // ��������
    service_ = ::CreateService(scm_manager_, SERVICE_NAME, SERVICE_SHOW, SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, driver_path,
        nullptr, nullptr, nullptr, nullptr, nullptr);
    if(!service_) {
        // ��ȡ�����벢�رշ�����������
        last_error_ = ::GetLastError();
        ::CloseServiceHandle(scm_manager_);
        scm_manager_ = nullptr;
        return false;
    }

    // �رվ��
    ::CloseServiceHandle(service_);
    ::CloseServiceHandle(scm_manager_);
    service_ = nullptr;
    scm_manager_ = nullptr;

    return true;
}

bool DriverManager::start_driver()
{
    // ������ָ��������ϵķ�����ƹ����������ӣ�����ָ���ķ�����ƹ��������ݿ�
    scm_manager_ = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm_manager_) {
        // ��ȡ������
        last_error_ = ::GetLastError();
        return false;
    }

    // �����з���
    service_ = ::OpenService(scm_manager_, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!service_) {
        // ��ȡ�����벢�رշ�����������
        last_error_ = ::GetLastError();
        ::CloseServiceHandle(scm_manager_);
        scm_manager_ = nullptr;
        return false;
    }

    // ��������
    bool ret = ::StartService(service_, 0, nullptr);
    if(!ret) {
        // ����ʧ�ܣ���ȡ������
        last_error_ = ::GetLastError();
    }

    // �رվ��
    ::CloseServiceHandle(service_);
    ::CloseServiceHandle(scm_manager_);
    service_ = nullptr;
    scm_manager_ = nullptr;

    return ret;
}

bool DriverManager::stop_driver()
{
    // ������ָ��������ϵķ�����ƹ����������ӣ�����ָ���ķ�����ƹ��������ݿ�
    scm_manager_ = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm_manager_) {
        // ��ȡ������
        last_error_ = ::GetLastError();
        return false;
    }

    // �����з���
    service_ = ::OpenService(scm_manager_, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!service_) {
        // ��ȡ�����벢�رշ�����������
        last_error_ = ::GetLastError();
        ::CloseServiceHandle(scm_manager_);
        scm_manager_ = nullptr;
        return false;
    }

    // ֹͣ����
    SERVICE_STATUS status = { 0 };
    bool ret = ::ControlService(service_, SERVICE_CONTROL_STOP, &status);
    if (!ret) {
        // ����ʧ�ܣ���ȡ������
        last_error_ = ::GetLastError();
    }

    // �رվ��
    ::CloseServiceHandle(service_);
    ::CloseServiceHandle(scm_manager_);
    service_ = nullptr;
    scm_manager_ = nullptr;

    return ret;
}

bool DriverManager::uninstall_driver()
{
    // ������ָ��������ϵķ�����ƹ����������ӣ�����ָ���ķ�����ƹ��������ݿ�
    scm_manager_ = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm_manager_) {
        // ��ȡ������
        last_error_ = ::GetLastError();
        return false;
    }

    // �����з���
    service_ = ::OpenService(scm_manager_, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!service_) {
        // ��ȡ�����벢�رշ�����������
        last_error_ = ::GetLastError();
        ::CloseServiceHandle(scm_manager_);
        scm_manager_ = nullptr;
        return false;
    }

    bool ret = ::DeleteService(service_);
    if (!ret) {
        // ����ʧ�ܣ���ȡ������
        last_error_ = ::GetLastError();
    }

    // �رվ��
    ::CloseServiceHandle(service_);
    ::CloseServiceHandle(scm_manager_);
    service_ = nullptr;
    scm_manager_ = nullptr;

    return ret;
}

bool DriverManager::connect_driver(const TCHAR *symbol_name)
{
    // ��������
    driver_ = ::CreateFile(symbol_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(driver_ == INVALID_HANDLE_VALUE) {
        // ����ʧ�ܣ���ȡ����ֵ
        last_error_ = ::GetLastError();
        return false;
    }
    return true;
}

void DriverManager::disconnect_driver()
{
    if(driver_ != INVALID_HANDLE_VALUE)
        ::CloseHandle(driver_);
}

bool DriverManager::io_control(DWORD control_code, LPVOID in_buf, DWORD in_buf_size, LPVOID out_buf, DWORD out_buf_size,
    LPDWORD ret_bytes)
{
    if(!::DeviceIoControl(driver_, control_code, in_buf, in_buf_size, out_buf, out_buf_size, ret_bytes, NULL)) {
        last_error_ = ::GetLastError();
        return false;
    }
    return true;
}

void DriverManager::set_driver(const TCHAR *path)
{
    // �ͷſռ�
    if (driver_path)
        delete[] driver_path;

    // ����ռ�
    DWORD length = _tcslen(path);
    DWORD bytes = (length + 1) * sizeof(TCHAR);
    driver_path = new TCHAR[bytes];
    // �����ַ���
    _tcsncpy(driver_path, path, length + 1);
}

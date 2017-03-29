#pragma once
#include "lock.h"

// ���ڿɼ����Ż������԰�д�ļ�����ĻŪ��һ����̨�߳���ȥ

class CLogger
{
public:
    enum LOG_TYPE
    {
        LT_DEBUG = 0x1,
        LT_INFO = 0x2,
        LT_WARNING = 0x4,
        LT_ERROR = 0x8,
        LT_FATAL = 0x10,
    };

public:
    CLogger(void);
    ~CLogger(void);

    void    Debug(const char* format, ...);
    void    Info(const char* format, ...);
    void    Warning(const char* format, ...);
    void    Error(const char* format, ...);
    void    Fatal(const char* format, ...);

    bool    Init(const char* szfilename);
    void	Release();

    void    SetColor(uint32 idx);
    void    ResetColor();

    void    SetScreenMask(UINT32 mask) { _screen_mask |= mask; }
    void    SetFileMask(uint32 mask) { _file_mask |= mask; }

private:
    void    _output(LOG_TYPE type, uint32 idx, const char* format, va_list argp);

private:

#ifdef PLAT_WIN
    HANDLE  _file = INVALID_HANDLE_VALUE;
#else
    FILE*   _file = nullptr;
#endif
    uint32  _screen_mask = 0;
    uint32  _file_mask = 0;

private:
    CCriticalSection _cs;
};

#pragma once


class CDbMgr
{
public:
    CDbMgr();
   ~CDbMgr();

public:
    // ������ֹͣ�����߳�
    void Start();
    void Stop();

public:
    void execute_sql(char* ddd) {}


private:
    void __work_proc();

private:
    std::thread _thread;
    bool _thread_running;

};


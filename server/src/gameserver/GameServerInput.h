#pragma once
#include "commandInput.h"

class CGameServerInput : public CCommandInput
{
public:
    CGameServerInput();
    ~CGameServerInput();


private:
    int OnZcg(int argc, char argv[PARAM_CNT][PARAM_LEN]);
    int OnTest(int argc, char argv[PARAM_CNT][PARAM_LEN]);
    int OnReload(int argc, char argv[PARAM_CNT][PARAM_LEN]);
    int OnTestDB(int argc, char argv[PARAM_CNT][PARAM_LEN]);
    int OnTestLua(int argc, char argv[PARAM_CNT][PARAM_LEN]);

private:
    void on_thread_proc();
    void on_thread_proc_stmt();

};



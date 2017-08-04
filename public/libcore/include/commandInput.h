#pragma once

#include "lock.h"

enum
{
    PARAM_CNT = 10,         // �����������
    PARAM_LEN = 16,         // �����ַ����ĳ���
    HELP_LEN = 64,          // ������Ϣ����
    DESC_LEN = 64,          // ������Ϣ�ĳ���   
};

class CCommandInput;
using CMD_FUNC = int (CCommandInput::*)(int argc, char argv[PARAM_CNT][PARAM_LEN]);


class CCommandInput
{
private:
    struct cmd_input
    {
        uint32      argc = 0;
        char        argv[PARAM_CNT][PARAM_LEN] = { { 0 } };
        CMD_FUNC    func = nullptr;
    };

    struct cmd_config
    {
        char        name[PARAM_LEN] = { 0 };
        char        help[HELP_LEN] = { 0 };
        char        desc[DESC_LEN] = { 0 };
        CMD_FUNC    func = nullptr;
    };

public:
    CCommandInput(void);
    virtual ~CCommandInput(void);

public:
    void Run();
    void Update();

protected:
    // ����������
    void add_command(const char* name, CMD_FUNC func, const char* help, const char* desc);
    void set_prompt(const char* pstr);
  
private:
    // ��������
    bool _parse_command(char* szCmdLine);
    void _print_prompt();
   
private:
    // common command function
    int OnQuit(int argc, char argv[PARAM_CNT][PARAM_LEN]);
    int OnHelp(int argc, char argv[PARAM_CNT][PARAM_LEN]);
    int OnList(int argc, char argv[PARAM_CNT][PARAM_LEN]);

protected:
    virtual void on_quit() {};

private:
    bool                    m_bRunning = false;
    std::string             m_strPrompt= "server# ";
    std::vector<cmd_config> m_vecCmdConfig;
    std::list<cmd_input>    m_lstCmdInput;
    std::mutex              m_mutex;
};


-- make gc more aggresive
collectgarbage("setstepmul", 1000)


-- init random
math.randomseed(os.time())
for i = 1, 100 do math.random() end



--[[
    ���ڽű������Ⱥ�˳�����������⣬
    ��:��1.lua�ļ��������ָ��2.lua�ļ��вŶ���ı���
    �϶����岻�ɹ���������˽���취
    ��������Լ���lua�ļ����ṩһ����������ע��
    ��ϵͳ�����еĽű�������ɵ�ʱ��ϵͳ���ͳһ��������ע��ĺ���
--]]
local _file_init_funs = {}


-- �ڸ����ű��ļ����ṩһ��local���ĺ����������ʼ���ű��ڲ�����
RegisterFileInit = function(func)
    table.insert(_file_init_funs, func)
end


RunFileInit = function()
    for _, func in ipairs(_file_init_funs) do
        func()
    end
    _file_init_funs = {}
end

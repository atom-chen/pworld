#pragma once

template<typename T, uint32 _type = 1>
class CSingleton
{
public:
    CSingleton()
    {
    }

    ~CSingleton()
    {
    }

    static void CreateInstance()
    {
        if (!_ptr)
        {
            _ptr = new T();
        }
    }

    static void DestroyInstance()
    {
        if (_ptr)
        {
            delete _ptr;
            _ptr = nullptr;
        }
    }

    static T* GetInstance()
    {
        return _ptr;
    }

private:
    static T* _ptr;
};

template<typename T, uint32 _type> T* CSingleton<T, _type>::_ptr = nullptr;



#define CREATE_INSTANCE(CLASS)  CSingleton<CLASS>::CreateInstance();
#define DESTROY_INSTANCE(CLASS) CSingleton<CLASS>::DestroyInstance();

#define INSTANCE(CLASS) CSingleton<CLASS>::GetInstance()

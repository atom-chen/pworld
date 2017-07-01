#include "stdafx.h"
#include "session.h"
#include "poller.h"
#include "singleton.h"
#include "logger.h"


namespace Net
{
#ifdef PLAT_WIN32

    void CSession::_set_session_deading()
    {
        if (_status == SOCK_STATUS::SOCK_STATUS_ALIVE)
        {
            _status = SOCK_STATUS::SOCK_STATUS_DEADING;
        }
    }

    void CORE_STDCALL CSession::session_cb(void* key, OVERLAPPED* overlapped, DWORD bytes)
    {
        CSession*  pThis = reinterpret_cast<CSession*>(key);
        PerIoData* pData = reinterpret_cast<PerIoData*>(overlapped);

        if (pThis->_status == SOCK_STATUS::SOCK_STATUS_DEADING)
        {
            pThis->_status = SOCK_STATUS::SOCK_STATUS_DEADED;
            return;
        }

        if (pData->_type == IO_TYPE::IO_TYPE_Send)
        {
            if (pData->_stag == IO_STATUS::IO_STATUS_SUCCESSD)
            {
                pThis->_on_send((char*)pThis->_b_send->Data(), bytes);
                // 数据是否发送完
                if (pThis->_b_send->DataLength() != bytes)
                {
                    INSTANCE(CLogger)->Error("数据未完全发送");
                }
                INSTANCE_2(CMessageQueue)->FreeMessage(pThis->_b_send);
                pThis->_b_send = nullptr;
            }
            else
            {
                pThis->_set_session_deading();
                INSTANCE(CLogger)->Error("数据发送失败，居然有这种情况，，");
                DWORD err = pData->_err;
                pThis->_on_send_error(err);
            }
        }
        else if (pData->_type == IO_TYPE::IO_TYPE_Recv)
        {
            if (pData->_stag == IO_STATUS::IO_STATUS_SUCCESSD)
            {
                if (bytes)
                {
                    pThis->_on_recv((char*)pThis->_io_recv._data, bytes);
                    pThis->_post_recv();
                }
                else
                {
                    pThis->_set_session_deading();
                }
            }
            else
            {
                pThis->_set_session_deading();
                DWORD err = pData->_err;
                pThis->_on_recv_error(err);
            }
        }

    }


    CSession::CSession() :
        _io_send(IO_TYPE::IO_TYPE_Send, 0),
        _io_recv(IO_TYPE::IO_TYPE_Recv, 0x1000),
        _status(SOCK_STATUS::SOCK_STATUS_NONE),
        _header(sizeof(uint32))
    {
    }


    CSession::~CSession()
    {
        SAFE_DELETE(_key);
    }


    void CSession::Attach(SOCKET_HANDER socket, void* key)
    {
        _header.Reset(4);

        _socket = socket;
        u_long v = 1;
        ::ioctlsocket(_socket, FIONBIO, &v);

        linger ln = { 1, 0 };
        setsockopt(_socket, SOL_SOCKET, SO_LINGER, (CHAR*)&ln, sizeof(linger));

        if (key)
        {
            _key = (Poll::CompletionKey*)key;
            _key->obj = this;
            _key->func = &CSession::session_cb;
        }
        else
        {
            _key = new Poll::CompletionKey{ this, &CSession::session_cb };
            if (INSTANCE(Poll::CPoller)->RegisterHandler((HANDLE)_socket, _key))
            {
                return;
            }
        }

        _io_recv._stag = IO_STATUS::IO_STATUS_SUCCESSD;
        _io_send._stag = IO_STATUS::IO_STATUS_SUCCESSD;
        _status = SOCK_STATUS::SOCK_STATUS_ALIVE;
        _post_recv();
    }


    void CSession::Send(const char* data, uint32 size)
    {
        if (!Alive())
            return;

        while (size)
        {
            CMessage* msg_writer = nullptr;
            if (_q_send.empty() || _q_send.back()->Full())
            {
                CMessage* msg = INSTANCE_2(CMessageQueue)->ApplyMessage();
                msg->Reset(0x1000);
                _q_send.push(msg);
            }
            msg_writer = _q_send.back();
            char* pdata = const_cast<char*>(data);
            msg_writer->Fill(pdata, size);
        }
    }


    bool CSession::Update()
    {
        if (Alive())
        {
            _post_send();
            return false;
        }
        else
        {
            _close();
            return true;
        }
    }


    void CSession::_post_send()
    {
        if (!Alive())
            return;

        if (_io_send._stag != IO_STATUS::IO_STATUS_SUCCESSD)
            return;

        if (_b_send)
            return;

        if (_q_send.empty())
            return;

        _b_send = _q_send.front();

        if (!_b_send->DataLength())
            return;

        _q_send.pop();

        WSABUF buf;
        buf.buf = (char*)_b_send->Data();
        buf.len = _b_send->DataLength();
        _io_send.Reset();
        int ret = ::WSASend(_socket, &buf, 1, nullptr, 0, &_io_send._over, nullptr);
        if (ret)
        {
            int err = WSAGetLastError();
            if (err == WSA_IO_PENDING)
            {
                _io_send._stag = IO_STATUS::IO_STATUS_PENDING;
            }
            else
            {
                _io_send._stag = IO_STATUS::IO_STATUS_QUIT;
                _on_send_error(err);
            }
        }
        else
        {
            _io_send._stag = IO_STATUS::IO_STATUS_SUCCESSD;
            INSTANCE(CLogger)->Debug("数据发送立即完成");
        }
    }


    void CSession::_post_recv()
    {
        if (!Alive())
            return;

        if (_io_recv._stag != IO_STATUS::IO_STATUS_SUCCESSD)
            return;

        WSABUF buf;
        buf.buf = (char*)_io_recv._data;
        buf.len = 0x1000;

        DWORD flags = 0;
        _io_recv.Reset();
        int ret = ::WSARecv(_socket, &buf, 1, nullptr, &flags, &_io_recv._over, nullptr);
        if (ret)
        {
            int err = WSAGetLastError();
            if (err == WSA_IO_PENDING)
            {
                _io_recv._stag = IO_STATUS::IO_STATUS_PENDING;
            }
            else
            {
                _io_recv._stag = IO_STATUS::IO_STATUS_QUIT;
                _on_recv_error(err);
            }
        }
        else
        {
            _io_recv._stag = IO_STATUS::IO_STATUS_SUCCESSD;
            INSTANCE(CLogger)->Debug("数据接收立即完成，这该怎么办啊");
        }
    }


    void CSession::_on_recv_error(uint32 err)
    {
        _recv_error = err;
    }


    void CSession::_on_send_error(uint32 err)
    {
        _send_error = err;
    }


    void CSession::_close()
    {
        if (_status == SOCK_STATUS::SOCK_STATUS_DEADED)
        {
            _status = SOCK_STATUS::SOCK_STATUS_DECAY;
            g_net_close_socket(_socket);
            on_closed();
        }
    }


    void CSession::Disconnect()
    {
        if (!Alive())
            return;

        if (_io_recv._stag != IO_STATUS::IO_STATUS_PENDING && _io_send._stag != IO_STATUS::IO_STATUS_PENDING)
        {
            _status = SOCK_STATUS::SOCK_STATUS_DEADED;
        }
        else
        {
            _status = SOCK_STATUS::SOCK_STATUS_DEADING;
            ::shutdown(_socket, SD_SEND);
        }
    }


    void CSession::_on_recv(char* pdata, uint32 size)
    {
        do
        {
            while (size && !_header.Full())
            {
                _header.Fill(pdata, size);
            }

            if (!size) return;

            if (!_msg)
            {
                uint32 data_len = *(uint32*)_header.Data();
                if (data_len >= max_packet_size() || data_len < 2)
                {
                    INSTANCE(CLogger)->Error("packet size exceed max_packet_size !!!");
                    this->Disconnect();
                    return;
                }

                _msg = INSTANCE(CMessageQueue)->ApplyMessage();
                _msg->Reset(data_len);
            }

            while (size && !_msg->Full())
            {
                _msg->Fill(pdata, size);
            }

            if (_msg->Full())
            {
                _msg->_param = 0;
                _msg->_ptr = nullptr;
                INSTANCE(CMessageQueue)->PushMessage(_msg);
                _msg = nullptr;
                _header.Reset(4);
            }

        } while (size);
    }


    void CSession::_on_send(char* pdata, uint32 size)
    {

    }


    void CSession::on_closed()
    {
        INSTANCE(CLogger)->Info("CSession::on_closed send_err=%u, recv_err=%u", _send_error, _recv_error);
    }



#else



    void CSession::_set_session_deading()
    {
        if (_status == SOCK_STATUS::SOCK_STATUS_ALIVE)
        {
            _status = SOCK_STATUS::SOCK_STATUS_DEADING;
        }
    }

    void CORE_STDCALL CSession::session_cb(void* obj, uint32 events)
    {
        CSession* pThis = (CSession*)obj;



        if (events | EPOLLERR)
        {
            pThis->_set_socket_status(SOCK_STATUS::SOCK_STATUS_ERROR);
            return;
        }

        if (events | EPOLLHUP)
        {
            pThis->_set_socket_status(SOCK_STATUS::SOCK_STATUS_RD_CLOSED);
        }


        if (events | EPOLLIN)
        {
            if (events | EPOLLRDHUP)
            {
                pThis->_set_socket_status(SOCK_STATUS::SOCK_STATUS_RD_CLOSED);
            }

            do
            {
                const int MAX_RECV_SIZE = 0x1000;
                static char data[MAX_RECV_SIZE];

                int ret = recv(_socket, data, MAX_RECV_SIZE, 0);
                if (ret > 0)
                {

                }
                else if (ret == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        break;
                    }
                    else if (errno == EINTR)
                    {
                        continue;
                    }
                    else
                    {
                        int err = g_net_socket_error(pThis->_socket);
                        _on_recv_error(err);
                        pThis->_set_socket_status(SOCK_STATUS::SOCK_STATUS_ERROR);
                        break;
                    }
                }
                else
                {
                    // connection be closed by peer.
                    pThis->_status = SOCK_STATUS::SOCK_STATUS_RD_CLOSED;
                }


            } while (true);
        }

        if (events | EPOLLOUT)
        {
            pThis->_send_pending = false;
        }

        

    }


    CSession::CSession() :
        _status(SOCK_STATUS::SOCK_STATUS_NONE),
        _header(sizeof(uint32))
    {
    }


    CSession::~CSession()
    {
        SAFE_DELETE(_key);
    }


    void CSession::Attach(SOCKET_HANDER socket, void* key)
    {
        _header.Reset(4);

        _socket = socket;

        linger ln = { 1, 0 };
        setsockopt(_socket, SOL_SOCKET, SO_LINGER, (char*)&ln, sizeof(linger));

        if (key)
        {
            _key = (Poll::CompletionKey*)key;
            _key->obj = this;
            _key->func = &CSession::session_cb;
        }
        else
        {
            _key = new Poll::CompletionKey{ this, &CSession::session_cb };
            if (INSTANCE(Poll::CPoller)->RegisterHandler(_socket, _key, EPOLLIN))
            {
                return;
            }
        }

      
    
        _status = SOCK_STATUS::SOCK_STATUS_ALIVE;
        _post_recv();
    }


    void CSession::Send(const char* data, uint32 size)
    {
        if (!Alive())
            return;

        while (size)
        {
            CMessage* msg_writer = nullptr;
            if (_q_send.empty() || _q_send.back()->Full())
            {
                CMessage* msg = INSTANCE_2(CMessageQueue)->ApplyMessage();
                msg->Reset(0x1000);
                _q_send.push(msg);
            }
            msg_writer = _q_send.back();
            char* pdata = const_cast<char*>(data);
            msg_writer->Fill(pdata, size);
        }
    }


    bool CSession::Update()
    {
        if (Alive())
        {
            _post_send();
            return false;
        }
        else
        {
            _close();
            return true;
        }
    }


    void CSession::_post_send()
    {
        if (!Alive())
            return;
        
        if (_send_pending)
            return;

        if (_b_send)
            return;

        

        if (_q_send.empty())
            return;

        _b_send = _q_send.front();

        if (!_b_send->DataLength())
            return;

        

        _q_send.pop();

       

__SEND__:

        int ret = send(_socket, _b_send->Data(), _b_send->DataLength(), MSG_NOSIGNAL);
        if (ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 注册事件 INSTANCE();
                _send_pending = true;
            }
            else if( errno == EINTR)
            {
                goto __SEND__;
            }
            else
            {
                _on_send_error(errno);
            }
        }

    }


    void CSession::_post_recv()
    {

        if (!Alive())
            return;

      

    }


    void CSession::_on_recv_error(uint32 err)
    {
        _recv_error = err;
    }


    void CSession::_on_send_error(uint32 err)
    {
        _send_error = err;
    }


    void CSession::_close()
    {
        if (_status == SOCK_STATUS::SOCK_STATUS_DEADED)
        {
            _status = SOCK_STATUS::SOCK_STATUS_DECAY;
            g_net_close_socket(_socket);
            on_closed();
        }
    }


    void CSession::Disconnect()
    {
        if (!Alive())
            return;

    }


    void CSession::_on_recv(char* pdata, uint32 size)
    {
        do
        {
            while (size && !_header.Full())
            {
                _header.Fill(pdata, size);
            }

            if (!size) return;

            if (!_msg)
            {
                uint32 data_len = *(uint32*)_header.Data();
                if (data_len >= max_packet_size() || data_len < 2)
                {
                    INSTANCE(CLogger)->Error("packet size exceed max_packet_size !!!");
                    this->Disconnect();
                    return;
                }

                _msg = INSTANCE(CMessageQueue)->ApplyMessage();
                _msg->Reset(data_len);
            }

            while (size && !_msg->Full())
            {
                _msg->Fill(pdata, size);
            }

            if (_msg->Full())
            {
                _msg->_param = 0;
                _msg->_ptr = nullptr;
                INSTANCE(CMessageQueue)->PushMessage(_msg);
                _msg = nullptr;
                _header.Reset(4);
            }

        } while (size);
    }


    void CSession::_set_socket_status(SOCK_STATUS s)
    {
        INSTANCE(CLogger)->Info("CSession::_status changed! prev = %d, curr = %d", _status, s);
        _status = s;
    }

    
    void CSession::_on_send(char* pdata, uint32 size)
    {

    }


    void CSession::on_closed()
    {
        INSTANCE(CLogger)->Info("CSession::on_closed send_err=%u, recv_err=%u", _send_error, _recv_error);
    }
    
    

#endif
}

#include "stdafx.h"
#include "xml_function.h"

#include "xml.h"
#include "xml_exception.h"

#ifdef PLAT_LINUX
#define _strnicmp strncasecmp
#endif

namespace XML
{

    size_t g_align_4(size_t t)
    {
        if ( t == 0 ) 
        {
            return 4;
        }

        size_t len = t;

        if ( len % 4 != 0 )
        {
            len = len + ( 4 - ( len % 4 ) );
        }

        return len;
    }


    bool g_is_blank( char* pCh )
    {
        if ( pCh[0] == ' ' || pCh[0] == '\t' )
        {
            return true;
        }

        if ( pCh[0] == 0x0D && pCh[1] == 0x0A )
        {
            return true;
        }

        return false;
    }


    bool g_is_node_name_end_1(char*pch)
    {
        if ( pch[0] == '>' )
        {
            return true;
        }
        return false;
    }

    bool g_is_node_name_end_2(char* pch)
    {
        if ( pch[0] == '/' && pch[1] == '>' )
        {
            return true;
        }
        return false;
    }

    bool g_is_node_name_end(char* pch)
    {
        if ( g_is_node_name_end_1(pch) )
        {
            return true;
        }
        return g_is_node_name_end_2(pch);
    }

    char* g_get_node_name( char*& pData, size_t& dwLen)
    {
        if ( !dwLen )
        {
            return nullptr;
        }

        char* pStart = pData;

        g_add_pointer(pData, dwLen);

        while( dwLen )
        {
            if ( g_is_blank(pData) || g_is_node_name_end(pData) )
            {
                break;
            }
            g_add_pointer(pData, dwLen);
        }
        return pStart;

    }

    bool g_is_head_node_end(char*pch)
    {
        if( pch[0] == '?' && pch[1] == '>' )
        {
            return true;
        }
        return false;
    }

    // aasfs = "sd"
    char* g_get_attrib_name(char*&pData, size_t& dwLen)
    {
        char* pStart = pData;
        g_add_pointer(pData, dwLen);

        while(dwLen)
        {
            if (  pData[0] == '=' || g_is_blank(pData) )
            {
                break;
            }
            g_add_pointer(pData, dwLen);
        }
        return pStart;
    }

    void g_skip_blank(char*&pData, size_t& dwLen)
    {
        char* pTemp = pData;
        while(static_cast<size_t>(pTemp - pData) < dwLen)
        {
            if ( pTemp[0] == ' ' || pTemp[0] == '\t' )
            {
                pTemp++;
                continue;
            }
            if ( pTemp[0] == 0x0D && pTemp[1] == 0x0A )
            {
                pTemp = pTemp + 2;
                continue;
            }
            break;
        }
        dwLen = dwLen - ( pTemp - pData );
        pData = pTemp;
    }


    void g_add_pointer( char*& pData, size_t& dwLen, size_t size )
    {
        if ( !pData || dwLen < size )
        {
            throw CXmlException("move pointer error;", "xml parse failed;");
            return;
        }
        pData = pData + size;
        dwLen = dwLen - size;
    }

    // pData start with "<!--"，ending with "-->"
    bool g_skip_comment( char*& pData, size_t& dwLen )
    {
        if ( !dwLen )
        {
            return true;
        }

        while( dwLen )
        {
            g_skip_blank(pData, dwLen);
            if ( 0 != _strnicmp("<!--", pData, 4 ) )
            {
                return true;
            }

            bool bfind = false;
            g_add_pointer(pData,dwLen, 4);
            while(dwLen)
            {
                if ( 0 == _strnicmp("-->", pData, 3 ) )
                {
                    bfind = true;
                    g_add_pointer(pData, dwLen, 3);
                    break;
                }
                g_add_pointer(pData,dwLen);
            }
            if ( !bfind )
            {
                throw CXmlException("xml parse failed;", "xml comment not completed;");
                return false;
            }
        }
        return true;
    }


    // (</node > </node> );
    char*   g_get_node_end_name(char*& pData, size_t& dwLen)
    {
        if ( !dwLen )
        {
            return nullptr;
        }

        char* pStart = pData;

        while ( dwLen > 0 )
        {
            if ( pData[0] == ' ' || pData[0] == '\t' || pData[0] == '>' )
            {
                break;
            }
            if ( pData[0] == 0x0D && pData[1] == 0x0A )
            {
                break;
            }
            g_add_pointer(pData, dwLen);
        }

        return pStart;
    }


    char* g_get_attrib_value(char*& pData, size_t& dwLen)
    {
        if ( dwLen == 0 )
        {
            return nullptr;
        }

        char* pStart = pData;

        char ch = pStart[0];
        if ( ch != '\'' && ch != '\"' )
        {
            throw CXmlException("Must start with ' for value;", pStart );
            return nullptr;
        }

        g_add_pointer(pData, dwLen);

        pStart = pData;

        while( dwLen )
        {// a= "sdfasfa"
            if ( pData[0] == '<' )
            {
                throw CXmlException("invalid char;", pData);
                return nullptr;
            }
            if ( pData[0] == '&' )
            {
                throw CXmlException("unspoort &;", pData);
                return nullptr;
            }
            if ( pData[0] == ch )
            {
                pData[0] = '\0';
                g_add_pointer(pData, dwLen);
                return pStart;
                break;
            }
            g_add_pointer(pData, dwLen);
        }
        return nullptr;
    }


    bool g_insert_attrib_list(std::list<CXmlAttrib*>& lstAttrib, CXmlAttrib* pAttrib)
    {
        auto it     = lstAttrib.end();
        auto itor   = lstAttrib.begin();

        while( itor != it )
        {
            if ( strcmp( (*itor)->GetName(), pAttrib->GetName() ) == 0 )
            {
                throw CXmlException("repeated attr name;", pAttrib->GetName());
                return false;
            }
            itor++;
        }

        lstAttrib.push_back(pAttrib);
        return true;
    }


    bool g_is_ascii(unsigned char ch)
    {
        if ( ch > 0x7f )
            return false;

        return true;
    }


    bool g_is_char(unsigned char ch)
    {
        if ( ch >= 65 && ch <= 90  ) // A~Z
        {
            return true;
        }
        if ( ch >= 97 && ch <= 122 )
        {
            return true;
        }
        return false;
    }

    bool g_is_number(unsigned char ch)
    {
        if ( ch >= 48 && ch <= 57 )
        {
            return true;
        }
        return false;
    }


    bool g_is_connect(unsigned char ch)
    {
        if ( ch == '-' || ch == '_' || ch == ':'|| ch == '.' )
            return true;

        return false;
    }


    bool g_is_ascii_valid(unsigned char ch)
    {
        return false;
    }

}
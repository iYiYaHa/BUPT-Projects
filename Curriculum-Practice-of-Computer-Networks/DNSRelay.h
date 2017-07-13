#pragma once
// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")

#include<iostream>
#include<string>
#include<list>
#include<unordered_map>
#include<fstream>
#include<ctime>
#include<limits.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<iomanip>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

const char defaultServer[] = "10.3.9.5";
const char defaultFile[] = "C:\\Users\\张有杰\\Source\\Repos\\DNS-Relay\\DNSRelay\\dnsrelay.txt";
const char PORT[] = "53";
const int MAXBUFLEN = 2048;
const int maxResponseTime = 4;//最长响应时间

namespace dnsNS {

    struct dnsPacket {
        struct _header {
            unsigned short ID;
            unsigned short QR;
            unsigned short OPCODE;
            unsigned short AA;
            unsigned short TC;
            unsigned short RD;
            unsigned short RA;
            unsigned short Z;
            unsigned short RCODE;
            unsigned short QDCOUNT;
            unsigned short ANCOUNT;
            unsigned short NSCOUNT;
            unsigned short ARCOUNT;
        };
        struct _query {
            std::string QNAME;
            unsigned short QTYPE;
            unsigned short QCLASS;
        };
        struct _RR {
            std::string NAME;
            unsigned short TYPE;
            unsigned short CLASS;
            unsigned int TTL;
            unsigned short RELENGTH;
            std::string RDATA;
        };
        _header header;
        std::list<_query> query;
        std::list<_RR> answer;
        std::list<_RR> authority;
        std::list<_RR> additional;
        int length;
    };
    struct innerID {
        unsigned short inID;//DNSRelay程序收到的查询ID
        //unsigned short outID;//DNSRelay程序发出的DNS请求ID
        sockaddr_storage addr;
        clock_t timeStamp;//时间戳
    };
    
    //作为使用 Socket 通信的基类，负责初始化及清理环境等
    class mySocket {
    public:
        mySocket()
        {
            bool flag = initialize();
            if (flag == false) {//如果初始化失败，抛出异常
                throw std::runtime_error("Failed to initialize the environment!\nPlease rerun the program...\n");
            }
        }
        bool initialize();
    };

    //内部及外部 DNS 分组的 ID 转换表
    class convertTable {
    private:
        unsigned short nextOutID;
        std::unordered_map<unsigned short, struct innerID> idTable;//(外部 ID ，内部 ID ，socket 地址，时间戳)
    public:
        convertTable() { nextOutID = 1; }
        //维护转换表，删除过时条目
        void maintainTable();

        //向转换表中加入信息
        void  addTable(unsigned short & outID, struct innerID & inner);

        //根据外部 ID 获取内部ID及其存储地址等信息
        bool fetchInnerID(const unsigned short & outID, struct innerID & inner);
    };

    //DNS 中继器
    class dnsRelayer :mySocket {
    private:
        std::unordered_map<std::string, std::string> dnsTable;//为了提高查询效率，使用关联容器存储域名-IP映射表,
                                                                                          //相同域名不再重复映射
        convertTable idTable;//内外消息 ID 转换表
        SOCKET sock;//通信 socket
        int debug = 0;//Debug 等级
        sockaddr servAddr;//外部 DNS 服务器地址
        size_t servAddrLen;//外部 DNS 服务器地址长度
        int count = 0;//已处理的 DNS 查询请求数目

        bool loadTable(const std::string file);//从外部文件载入域名 - IP映射表
        dnsPacket parsePacket(const char * buf, const int bytes);//解析 DNS 分组
        void displayPacket(const dnsPacket & packet);//显示 DNS 分组
        void response(char * recvBuf, const dnsPacket & packet, sockaddr_storage & theirAddr);//响应处理 DNS 分组
        std::string addrToIP(const sockaddr_storage & addr);
    public:
        dnsRelayer(int level = 0, const std::string ip = defaultServer, const std::string file = defaultFile);
        void relay();//中继函数
    };

    //根据参数创建一个 DNS 中继器
    dnsRelayer * getRelayer(int argc, char * argv[]);

}
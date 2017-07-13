/*
    Function:DNS Relay 程序
    Date:2017/3/11
    Programmer:张有杰
    Thoughts about the program:
            1.监听53号端口
            2.分析来自53号的数据包
                2.1 提取域名等信息
            3.检查所要查询的域名是否在dns.txt中
                3.1 如果在
                    返回对应的IP地址
                3.2 否则
                        将请求转发给外部的DNS服务器，并返回结果
*/
#include "DNSRelay.h"
using namespace dnsNS;

//初始化windows中的socket环境
bool mySocket::initialize()
{
    WSADATA wsaData;

    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed:" << iResult << std::endl;
        return false;
    }
    return true;
}

//完成载入域名IP对照表、获取socket、bind至53号端口等功能
dnsRelayer::dnsRelayer(int level, const std::string ip, const std::string file) :debug(level), mySocket()
{
    int flag;
    struct addrinfo *serv, *p, hints;
    std::cout << "DNS Relay, Version 1.0, Build: May 14th 2017" << std::endl;
    std::cout << "Usage: dnsrelay [-d | -dd] [<dns-server>] [<db-file>]\n";
    std::cout << "Name Server : " << ip << ":" << PORT << std::endl;
    std::cout << "Debug Level :" << debug << std::endl;
    std::cout << std::endl;
    std::cout << "Try to load the dns table\n";
    //if (loadTable(file) == false) {
    //    throw std::runtime_error("Failed to load dns relay table.\n");
    //}
    //原本打算导入失败就抛出异常
    //后来改为导入失败打印错误信息，继续工作。即中继所有请求。

    if (loadTable(file) == false) {
        std::cout << "Failed to load dns relay table.\n";
    }
    else {
        std::cout << "DNS table loaded.\n";
        if (debug == 1 || debug == 2) {
            std::cout << dnsTable.size() << " pieces of DNS table items in total." << std::endl;
        }
    }

    //获取外部 DNS 服务器地址信息
    //获取地址信息时填充 hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    flag = getaddrinfo(ip.c_str(), PORT, &hints, &serv);
    if (flag != 0) {
        throw std::runtime_error("Failed to get outside DNS server's address information.");
    }
    servAddr = *(serv->ai_addr);//保存外部服务器地址
    servAddrLen = serv->ai_addrlen;
    freeaddrinfo(serv);

    //获取本机地址信息
    //获取地址信息时填充 hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    flag = getaddrinfo(NULL, PORT, &hints, &serv);
    if (flag != 0) {
        std::cout << "Failed to get the address information." << std::endl;
        std::cout << gai_strerror(flag) << std::endl;
        throw std::runtime_error("Failed to get address information.");
    }

    //获取 socket 并完成绑定
    sock = INVALID_SOCKET;
    for (p = serv; p != NULL; p = p->ai_next) {//遍历所有可用的地址信息
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == SOCKET_ERROR) {
            sock = INVALID_SOCKET;
            continue;
        }

        //使用被占用的端口
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int));

        //绑定 socket
        if (bind(sock, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sock);
            continue;
        }
        break;//成功获取一个socket并将其绑定在端口上。
    }
    if (p == NULL) {//绑定失败
        //std::cout << "Failed to bind." << std::endl;
        WSACleanup();
        freeaddrinfo(serv);
        throw std::runtime_error("Failed to acquire and bind the socket.Maybe a bad name server ip.\n");
    }

    std::cout << "Acquire a socket.\n";
    std::cout << "Bind it to the specified port 53.\n";
    freeaddrinfo(serv);
}

bool dnsRelayer::loadTable(const std::string file)
{
    std::ifstream tableFile;
    std::string name, ip;
    tableFile.open(file);
    int count = 0;
    if (tableFile.is_open() == false) {//未能打开DNS转换文件
        return false;
    }
    while (tableFile >> ip) {//按顺序读入ip和域名
        tableFile >> name;
        dnsTable.insert(std::make_pair(name, ip));
        count += 1;
        if (debug == 2)
            std::cout << std::setw(8) << count << ": " << std::setw(16) << ip << std::setw(28) << std::setiosflags(std::ios::right) << name << std::endl;
    }
    tableFile.close();
    return true;
}

dnsPacket dnsNS::dnsRelayer::parsePacket(const char * buf, const int bytes)
{
    int len = 0, index = 0;
    dnsPacket::_query tmpQuery;
    dnsPacket result;

    result.header.ID = (buf[0] << 8 | (0x00ff & buf[1]));
    result.header.QR = buf[2] >> 7 & 0x01;
    result.header.OPCODE = (buf[2] >> 3 & 0x0f);
    result.header.AA = buf[2] & 0x04;
    result.header.TC = buf[2] & 0x02;
    result.header.RD = buf[2] & 0x01;
    result.header.RA = buf[3] & 0x80;
    result.header.RCODE = buf[3] & 0x0f;
    result.header.QDCOUNT = (buf[4] << 8) | buf[5];
    result.header.ANCOUNT = (buf[6] << 8) | buf[7];
    result.header.NSCOUNT = (buf[8] << 8) | buf[9];
    result.header.ARCOUNT = (buf[10] << 8) | buf[11];
    result.length = bytes;

    if (result.header.QR == 0) {//如果是请求分组，则进一步解析查询的域名等信息
        count += 1; //查询请求分组数目加一

        //解析查询部分的域名
        index = 12;
        len = buf[index];
        std::string name = "";
        while (len != 0) {
            for (int i = 0;i < len;++i) {
                ++index;
                name += buf[index];
            }
            ++index;
            name += '.';
            len = buf[index];
        }
        name.pop_back();//弹出最后一个 '.'
        tmpQuery.QNAME = name;

        ++index;//index = 13
        tmpQuery.QTYPE = buf[index] << 8 | buf[index + 1];
        index += 2;//index = 15
        tmpQuery.QCLASS = buf[index] << 8 | buf[index + 1];
        result.query.push_back(tmpQuery);
    }
    return std::move(result);
}

//将dns packet 的内容显示出来
void dnsNS::dnsRelayer::displayPacket(const dnsPacket & packet)
{
    if (debug == 0)
        return;
    else if (debug == 1) {
        if (packet.header.QR == 0) {
            for (auto i = packet.query.begin();i != packet.query.end();++i) {
                std::cout << std::setw(8) << (clock() / CLOCKS_PER_SEC)<< "： " << "NAME: ";
                std::cout << std::setw(25) << std::setiosflags(std::ios::right) << i->QNAME;
                std::cout << "  TYPE: " << std::setw(2) << std::setiosflags(std::ios::right) << i->QTYPE;
                std::cout << " CLASS: " << std::setw(2) << std::setiosflags(std::ios::right) << i->QCLASS << std::endl;
            }
        }
    }
    else if (debug == 2) {
        std::cout << "ID:" << packet.header.ID << " QR:" << packet.header.QR << " OPCODE:" << packet.header.OPCODE;
        std::cout << " AA:" << packet.header.AA << " TC:" << packet.header.TC << " RD:" << packet.header.RD;
        std::cout << " RA:" << packet.header.RA << " QDCOUNT:" << packet.header.QDCOUNT;
        std::cout << " ANCOUNT:" << packet.header.ANCOUNT << " NSCOUNT:" << packet.header.NSCOUNT;
        std::cout << "ARCOUNT:" << packet.header.ARCOUNT << std::endl;
        std::cout << std::endl;
    }
}

//开始中继处理DNS请求
void dnsRelayer::relay()
{
    int numbytes = 0, theirlen = 0;
    char * recvBuf = new char[MAXBUFLEN];
    struct sockaddr_storage theirAddr;
    theirlen = sizeof(theirAddr);
    while (true) {
        //time_t a = time(NULL);

        numbytes = recvfrom(sock, recvBuf, MAXBUFLEN - 1, 0,
            (struct sockaddr *)&theirAddr, &theirlen);
        if (numbytes == -1) {
            int t = WSAGetLastError();
            if(debug != 0)
                std::cout << "Error in recvfrom : # " << t << std::endl;
            continue;
            //throw std::runtime_error("Error when receving...\n");
        }
        if (debug == 2) {
            std::cout << "Receive from " << addrToIP(theirAddr) << " (" << numbytes << " bytes)\n";
            std::cout << "PACKET:";
            for (int i = 0;i < numbytes;++i)
                std::cout << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)(unsigned char)recvBuf[i] << " ";
            std::cout << std::endl;
        }
        recvBuf[numbytes] = '\0';
        dnsPacket recvPacket = parsePacket(recvBuf, numbytes);
        displayPacket(recvPacket);
        response(recvBuf, recvPacket, theirAddr);
        //time_t b = time(NULL);
        //std::cout << "wanle" << (b - a) << std::endl;
    }
    delete recvBuf;
}

//修改缓冲区中的相关信息，对DNS分组进行响应
void dnsRelayer::response(char * recvBuf, const dnsPacket & packet, sockaddr_storage & theirAddr)
{
    unsigned short outID;
    innerID tmpInner;
    if (packet.header.QR == 0) {//收到的DNS分组为请求
                                //说明是DNS请求，则应该查询本机域名-IP对照表(使用小写)
                                //如果存在，则构造响应包//修改Buffer
                                //否则，保存发送方的地址信息及会话ID
        for (auto i = packet.query.begin(); i != packet.query.end(); ++i) {
            std::string tmp = i->QNAME;
            auto pos = dnsTable.find(tmp);
            if (pos == dnsTable.end() || i->QTYPE == 0x001c) {//DNS中不存在该域名 或者 IPV6
                tmpInner.addr = theirAddr;
                tmpInner.inID = packet.header.ID;
                tmpInner.timeStamp = clock() / CLOCKS_PER_SEC;

                idTable.addTable(outID, tmpInner);//加入内外 ID 映射

                recvBuf[0] = outID >> 8;
                recvBuf[1] = outID;
                int t = sendto(sock, recvBuf, packet.length, 0, (sockaddr *)&servAddr, servAddrLen);
                if (debug == 2) {
                    std::cout << "Send to " << addrToIP(theirAddr) << " (" << packet.length << " bytes)\n";
                    std::cout << "[ID:" << tmpInner.inID << " -> " << outID << "]" << std::endl;
                    std::cout << std::endl;
                }
                if (t != packet.length && debug != 0) {
                    t = WSAGetLastError();
                    std::cout << "sento() Error # " << t << std::endl;
                }
            }
            else {
                recvBuf[2] |= 0x80;
                recvBuf[7] = 1;

                int index = packet.length;
                recvBuf[index++] = 0xc0;//数据压缩，指向 query 中的 NAME
                recvBuf[index++] = 0x0c;

                recvBuf[index++] = 0x00;//表示 TYPE 为 1 ，主机地址
                recvBuf[index++] = 0x01;

                recvBuf[index++] = 0x00;//CLASS 为 1
                recvBuf[index++] = 0x01;

                recvBuf[index++] = 0x00;//TTL 为 395
                recvBuf[index++] = 0x00;
                recvBuf[index++] = 0x01;
                recvBuf[index++] = 0x8b;

                recvBuf[index++] = 0x00;//IP地址长度
                recvBuf[index++] = 0x04;

                //IP地址
                std::string ip = pos->second;
                if (ip == "0.0.0.0") {//屏蔽域名
                    recvBuf[3] |= 0x03;
                }
                else {
                    int ipSec = 0;
                    for (size_t i = 0;i < ip.length();++i) {
                        if (ip[i] != '.' && i != ip.length() - 1) {
                            ipSec = ipSec * 10 + (ip[i] - '0');
                        }
                        else {
                            recvBuf[index++] = ipSec;
                            ipSec = 0;
                        }
                    }
                }
                recvBuf[index++] = 0x00;//结尾标志
               int t = sendto(sock, recvBuf, index, 0, (sockaddr *)&theirAddr, sizeof(theirAddr));
                if (debug == 2) {
                    std::cout << "Send to " << addrToIP(theirAddr) << " (" << packet.length << " bytes)" << std::endl;
                    std::cout << std::endl;
                }
                if (t != packet.length&& debug == 2) {
                    t = WSAGetLastError();
                    std::cout << "sento() Error # " << t << std::endl;
                }
            }
        }
    }
    else {//收到的DNS分组为响应
          //说明是从外部服务器收到的
        innerID tmp;
        if (idTable.fetchInnerID(packet.header.ID, tmp) == true) {
            idTable.maintainTable();
            recvBuf[0] = (tmp.inID >> 8);
            recvBuf[1] = (tmp.inID);
            int t = sendto(sock, recvBuf, packet.length, 0, (sockaddr *)&tmp.addr, sizeof(tmp.addr));
            if (debug == 2) {
                std::cout << "Send to " << addrToIP(tmp.addr) << " (" << packet.length << " bytes)\n";
                std::cout << "[ID:" << packet.header.ID << " -> " << tmp.inID << "]" << std::endl;
                std::cout << std::endl;
            }
            if (t != packet.length && debug != 0) {
                t = WSAGetLastError();
                std::cout << "sento() Error # " << t << std::endl;
            }
        }
        //不存在与内部消息ID 转换表中，说明超时，直接丢弃。
        //else {
        //}
    }
}

//从 sockaddr_storage 中提取 IP 和 端口
std::string dnsNS::dnsRelayer::addrToIP(const sockaddr_storage & addr)
{
    std::string res;
    char * tmp = nullptr;
    unsigned short port = ((sockaddr_in *)&addr)->sin_port;
    int p = 100000;
    tmp = inet_ntoa(((sockaddr_in *)&addr)->sin_addr);
    res = res + std::string(tmp) + ":";
    while (port / p == 0)
        p /= 10;
    while (port != 0) {
        res += (port / p + '0');
        port = port % p;
        p /= 10;
    }
    return res;
}

void dnsNS::convertTable::maintainTable()
{
    for (auto i = idTable.begin(); i != idTable.end();++i) {
        if ((i->second.timeStamp - (clock() / CLOCKS_PER_SEC)) >= maxResponseTime)
            idTable.erase(i);
    }
}

void  dnsNS::convertTable::addTable(unsigned short & outID, innerID & inner)
{
    //最大序号值
    while (idTable.find(nextOutID) != idTable.end()) {
        nextOutID = (nextOutID + 1) % USHRT_MAX;
    }
    idTable.insert(std::make_pair(nextOutID, inner));
    outID = nextOutID;
    nextOutID = (nextOutID + 1) % USHRT_MAX;
}

bool dnsNS::convertTable::fetchInnerID(const unsigned short & outID, innerID & inner)
{
    auto i = idTable.find(outID);
        if (i != idTable.end()) {
            inner = i->second;
            idTable.erase(i);
            return true;
        }
    return false;
}

dnsNS::dnsRelayer * dnsNS::getRelayer(int argc, char * argv[]) {
    dnsRelayer * res;
    if (argc == 1) {
        res = new dnsRelayer();
    }
    else if (argc == 2) {//debug等级
        if (strcmp(argv[1], "-d") == 0)
            res = new dnsRelayer(1);
        else if (strcmp(argv[1], "-dd") == 0)
            res = new dnsRelayer(2);
        else
            res = new dnsRelayer(0, argv[1]);
    }
    else if (argc == 3) {//debug等级，IP地址/或者默认debug等级，IP地址，dnsrelay.txt
        if (strcmp(argv[1], "-d") == 0)
            res = new dnsRelayer(1, argv[2]);
        else if (strcmp(argv[1], "-dd") == 0)
            res = new dnsRelayer(2, argv[2]);
        else
            res = new dnsRelayer(0, argv[1],argv[2]);
    }
    else if (argc == 4) {
        if (strcmp(argv[1], "-d") == 0)
            res = new dnsRelayer(1, argv[2],argv[3]);
        else if (strcmp(argv[1], "-dd") == 0)
            res = new dnsRelayer(2, argv[2],argv[3]);
        else
            res = new dnsRelayer(0, argv[2],argv[3]);
    }
    return res;
}
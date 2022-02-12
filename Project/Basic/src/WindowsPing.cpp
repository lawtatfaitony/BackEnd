#include "WindowsPing.h"
#include <stdio.h>
#include <stdlib.h>

WindowsPing::WindowsPing() 
    :m_szICMPData(nullptr)
    , m_bIsInitSucc(false)
    , m_usPacketSeq(0)
{
    init_network();
}

WindowsPing::~WindowsPing()
{
    WSACleanup();
    if (nullptr != m_szICMPData)
    {
        free(m_szICMPData);
        m_szICMPData = nullptr;
    }
}

bool WindowsPing::Ping(char *pDstIp, bool bNeedReplay, DWORD dwTimeout, bool bPrintResult)
{
    if (!m_bIsInitSucc) return false;

    // create a socktet
    sockaddr_in sockaddrDest;
    sockaddrDest.sin_family = AF_INET;
    sockaddrDest.sin_addr.s_addr = inet_addr(pDstIp);
    int nSockaddrDestSize = sizeof(sockaddrDest);

    // construct an icmp packet
    int nICMPDataSize = DEF_PACKET_SIZE + sizeof(ICMPHeader);
    ULONG ulSendTimestamp = get_tickcount_calibrate();
    USHORT usSeq = ++m_usPacketSeq;
    memset(m_szICMPData, 0, nICMPDataSize);
    ICMPHeader *pICMPHeader = (ICMPHeader*)m_szICMPData;
    pICMPHeader->m_byType = ECHO_REQUEST;
    pICMPHeader->m_byCode = 0;
    pICMPHeader->m_usID = m_usCurrentProcID;
    pICMPHeader->m_usSeq = usSeq;
    pICMPHeader->m_ulTimeStamp = ulSendTimestamp;
    pICMPHeader->m_usChecksum = cal_check_sum((USHORT*)m_szICMPData, nICMPDataSize);

    //send ICMP
    if (SOCKET_ERROR == sendto(m_sockRaw, m_szICMPData, nICMPDataSize, 0, (struct sockaddr*)&sockaddrDest, nSockaddrDestSize))
        return false;

    // need response
    if (!bNeedReplay) return true;

    char recvbuf[256] = { "\0" };
    while (true)
    {
        // handle the response
        if (WSAWaitForMultipleEvents(1, &m_event, false, 100, false) != WSA_WAIT_TIMEOUT)
        {
            WSANETWORKEVENTS netEvent;
            WSAEnumNetworkEvents(m_sockRaw, m_event, &netEvent);
            if (netEvent.lNetworkEvents & FD_READ)
            {
                ULONG nRecvTimestamp = get_tickcount_calibrate();
                int nPacketSize = recvfrom(m_sockRaw, recvbuf, 256, 0, (struct sockaddr*)&sockaddrDest, &nSockaddrDestSize);
                if (nPacketSize != SOCKET_ERROR)
                {
                    IPHeader *pIPHeader = (IPHeader*)recvbuf;
                    USHORT usIPHeaderLen = (USHORT)((pIPHeader->m_byVerHLen & 0x0f) * 4);
                    ICMPHeader *pICMPHeader = (ICMPHeader*)(recvbuf + usIPHeaderLen);
                    if (pICMPHeader->m_usID == m_usCurrentProcID //current process
                        && ECHO_REPLY == pICMPHeader->m_byType //icmp
                        && pICMPHeader->m_usSeq == usSeq) // current request
                    {
                        PingReply repPing;
                        repPing.m_usSeq = usSeq;
                        repPing.m_dwRoundTripTime = nRecvTimestamp - pICMPHeader->m_ulTimeStamp;
                        repPing.m_dwBytes = nPacketSize - usIPHeaderLen - sizeof(ICMPHeader);
                        repPing.m_dwTTL = pIPHeader->m_byTTL;
                        if (bPrintResult)
                            print_result(pDstIp, repPing);
                        return true;
                    }
                }
            }
        }
        // timeout
        if (get_tickcount_calibrate() - ulSendTimestamp >= dwTimeout)
            return false;
    }
    return true;
}

void WindowsPing::init_network()
{
    WSADATA WSAData;
    if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
    {
        printf("WSAStartup() failed: %d\n", GetLastError());
        return;
    }
    m_event = WSACreateEvent();
    m_usCurrentProcID = (USHORT)GetCurrentProcessId();
    m_sockRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, nullptr, 0, 0);
    if (m_sockRaw == INVALID_SOCKET)
    {
        // 10013£¬ not enough rights
        printf("WSASocket() failed:%d\n", WSAGetLastError());
    }
    else
    {
        WSAEventSelect(m_sockRaw, m_event, FD_READ);
        m_bIsInitSucc = true;
        m_szICMPData = (char*)malloc(DEF_PACKET_SIZE + sizeof(ICMPHeader));
        if (nullptr == m_szICMPData)
            m_bIsInitSucc = false;
    }
}

USHORT WindowsPing::cal_check_sum(USHORT *pBuffer, int nSize)
{
    unsigned long ulCheckSum = 0;
    while (nSize > 1)
    {
        ulCheckSum += *pBuffer++;
        nSize -= sizeof(USHORT);
    }
    if (nSize)
        ulCheckSum += *(UCHAR*)pBuffer;

    ulCheckSum = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
    ulCheckSum += (ulCheckSum >> 16);

    return (USHORT)(~ulCheckSum);
}

ULONG WindowsPing::get_tickcount_calibrate()
{
    ULONG s_ulFirstCallTick = 0;
    LONGLONG s_ullFirstCallTickMS = 0;

    SYSTEMTIME systemtime;
    FILETIME filetime;
    GetLocalTime(&systemtime);
    SystemTimeToFileTime(&systemtime, &filetime);
    LARGE_INTEGER liCurrentTime;
    liCurrentTime.HighPart = filetime.dwHighDateTime;
    liCurrentTime.LowPart = filetime.dwLowDateTime;
    LONGLONG llCurrentTimeMS = liCurrentTime.QuadPart / 10000;

    if (s_ulFirstCallTick == 0)
        s_ulFirstCallTick = GetTickCount();
    if (s_ullFirstCallTickMS == 0)
        s_ullFirstCallTickMS = llCurrentTimeMS;
    return s_ulFirstCallTick + (ULONG)(llCurrentTimeMS - s_ullFirstCallTickMS);
}

void WindowsPing::print_result(const char* pDstIp, const PingReply& repPing)
{
    if (nullptr == pDstIp) return;
    printf("Reply from %s: bytes=%d time=%ldms TTL=%ld\n", pDstIp,
        repPing.m_dwBytes, repPing.m_dwRoundTripTime, repPing.m_dwTTL);
}
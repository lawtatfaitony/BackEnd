#pragma once
#include <winsock2.h>
#pragma comment(lib, "WS2_32")


#define DEF_PACKET_SIZE 32
#define ECHO_REQUEST 8
#define ECHO_REPLY 0

struct IPHeader
{
    BYTE m_byVerHLen; //4λ�汾+4λ�ײ�����
    BYTE m_byTOS; //��������
    USHORT m_usTotalLen; //�ܳ���
    USHORT m_usID; //��ʶ
    USHORT m_usFlagFragOffset; //3λ��־+13λƬƫ��
    BYTE m_byTTL; //TTL
    BYTE m_byProtocol; //Э��
    USHORT m_usHChecksum; //�ײ������
    ULONG m_ulSrcIP; //ԴIP��ַ
    ULONG m_ulDestIP; //Ŀ��IP��ַ
};

struct ICMPHeader
{
    BYTE m_byType; //����
    BYTE m_byCode; //����
    USHORT m_usChecksum; //����� 
    USHORT m_usID; //��ʶ��
    USHORT m_usSeq; //���
    ULONG m_ulTimeStamp; //ʱ������Ǳ�׼ICMPͷ����
};

struct PingReply
{
    USHORT m_usSeq;
    DWORD m_dwRoundTripTime;
    DWORD m_dwBytes;
    DWORD m_dwTTL;
};

class WindowsPing
{
public:
    WindowsPing();
    ~WindowsPing();
    bool Ping(char *szDestIP, bool bNeedReplay = true, DWORD dwTimeout = 2000, bool bPrintResult = false);

private:
    void init_network();
    USHORT cal_check_sum(USHORT *pBuffer, int nSize);
    ULONG get_tickcount_calibrate();
    void print_result(const char* pDstIp, const PingReply& repPing);

private:
    SOCKET m_sockRaw;
    WSAEVENT m_event;
    USHORT m_usCurrentProcID;
    char *m_szICMPData;
    bool m_bIsInitSucc;
    USHORT m_usPacketSeq;

};
#ifndef WIN32
#include "LinuxPing.h"
#include <stdio.h>



Linuxping::Linuxping()
{
    m_nSend = 0;
    m_nRecv = 0;

    m_dTotalResponseTimes = 0;
}

Linuxping::~Linuxping()
{
    if (!close_socket())
        printf("CloseSocket failed!\n");
}

bool Linuxping::Ping(const std::string& strIp, int times)
{
    int nTimeout = 3;
    if (!create_socket())
    {
        printf("CreateSocket failed!!!\n");
        return false;
    }
    printf("PING %s(%s): %d bytes data in ICMP packets.\n", strIp.c_str(),
        strIp.c_str(), SEND_DATA_LEN);
    while (nTimeout--)
    {
        send_packet();
        recv_packet();
        sleep(1);
    }
    statistics(SIGINT);
    return true;
}

bool Linuxping::create_socket(const std::string& strIp)
{
    char buf[2048];
    int errnop = 0;
    unsigned long inaddr;
    struct hostent hostinfo, * dest_phost;
    struct protoent* protocol = NULL;

    if ((protocol = getprotobyname("icmp")) == NULL)
    {
        perror("getprotobyname()");
        printf("CreateSocket : getprotobyname failed:%d\n", errnop);
        return false;
    }
    if (-1 == (m_nSocketfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)))
    {
        perror("socket");
        printf("CreateSocket: create socket failed:%d\n", m_nSocketfd);
        return false;
    }
    setuid(getuid());
    m_dest_addr.sin_family = AF_INET;

    bzero(&(m_dest_addr.sin_zero), 8);

    if ((inaddr = inet_addr(strIp.c_str())) == INADDR_NONE)
    {
        //if(getprotobyname_r(m_strIp.c_str(), &hostinfo, buf, sizeof(buf), &dest_phost, &errnop))
        //{
        //	printf("CreateSocket: getprotobyname error %s failed:%d\n", m_strIp.c_str(),errnop);
        //	return false;
        //}
        //else
        //{
        m_dest_addr.sin_addr = *((struct in_addr*)dest_phost->h_addr);
        //}
    }
    else
    {
        m_dest_addr.sin_addr.s_addr = inaddr;
    }

    return true;
}

bool Linuxping::close_socket()
{
    bool flag = false;
    if (m_nSocketfd)
    {
        close(m_nSocketfd);
        flag = true;
    }
    return flag;
}

void Linuxping::send_packet(void)
{
    int packetsize;
    packetsize = pack(m_nSend);

    if ((sendto(m_nSocketfd, m_sendpacket, packetsize, 0, (const struct sockaddr*)&m_dest_addr, sizeof(m_dest_addr))) < 0)
    {
        printf("send_packet: send error :\n");
    }
    m_nSend++;
}

void Linuxping::recv_packet(void)
{
    int fromlen, packetsize, n;
    while (m_nRecv < m_nSend)
    {
        struct timeval timeout;
        fd_set readfd;
        FD_ZERO(&readfd);

        FD_SET(m_nSocketfd, &readfd);

        int maxfd = m_nSocketfd + 1;
        timeout.tv_sec = m_nMaxTimeWait;

        timeout.tv_usec = 0;

        n = select(maxfd, &readfd, NULL, NULL, &timeout);

        switch (n)
        {
        case 0:
            printf("recv_packet: select time out: \n");
            break;
        case -1:
            printf("recv_packet: select error: \n");
            break;
        default:
            if (FD_ISSET(m_nSocketfd, &readfd))
            {
                if ((packetsize = recvfrom(m_nSocketfd, m_recvpacket, sizeof(m_recvpacket), 0, (struct sockaddr*)&m_from_addr, (socklen_t*)&fromlen)) < 0)
                {
                    printf("packetsize = %d\n", packetsize);
                    //printf("recv_packet: recv error: %d\n", errno);
                    return;
                }
                gettimeofday(&m_tvrecv, NULL);
                m_end_tvrecv.tv_usec = m_tvrecv.tv_usec;
                m_end_tvrecv.tv_sec = m_tvrecv.tv_sec;

                if (unpack(m_recvpacket, packetsize) == -1)
                {
                    continue;
                }
                m_nRecv++;
            }
            break;
        }
    }
}

int Linuxping::pack(int pack_number)
{
    int packsize;
    struct icmp* pIcmp;
    struct timeval* pTime;

    pIcmp = (struct icmp*)m_sendpacket;

    pIcmp->icmp_type = ICMP_ECHO;
    pIcmp->icmp_code = 0;
    pIcmp->icmp_cksum = 0;
    pIcmp->icmp_seq = pack_number;
    pIcmp->icmp_id = getpid();
    packsize = 8 + SEND_DATA_LEN;
    pTime = (struct timeval*)pIcmp->icmp_data;
    gettimeofday(pTime, NULL);
    if (m_nSend == 0)
    {
        m_begin_tvrecv.tv_usec = pTime->tv_usec;
        m_begin_tvrecv.tv_sec = pTime->tv_sec;
    }
    pIcmp->icmp_cksum = cal_chksum((unsigned short*)pIcmp, packsize);
    return packsize;
}

int Linuxping::unpack(char* buf, int len)
{
    int i, iphdrlen;
    struct icmp* pIcmp;
    struct timeval* tvsend;
    struct ip* recv_ip = (struct ip*)buf;
    double rtt;

    iphdrlen = recv_ip->ip_hl << 2;
    pIcmp = (struct icmp*)(buf + iphdrlen);
    len -= iphdrlen;
    if (len < 8)
    {
        printf("ICMP packets's length is less than 8'");
        return -1;
    }

    if ((pIcmp->icmp_type == ICMP_ECHOREPLY) && (m_copy_Ip == inet_ntoa(m_from_addr.sin_addr)) && (pIcmp->icmp_id = getpid()))
    {
        tvsend = (struct timeval*)pIcmp->icmp_data;
        tv_sub(&m_tvrecv, tvsend);
        rtt = m_tvrecv.tv_sec * 1000 + (double)m_tvrecv.tv_usec / 1000;

        printf("%d byte from %s : icmp_seq=%u ttl=%d time=%.3fms\n",
            len,
            inet_ntoa(m_from_addr.sin_addr),
            pIcmp->icmp_seq,
            recv_ip->ip_ttl,
            rtt);
    }
    else
    {
        printf("throw away the old package %d\tbyte from %s\t: icmp_seq=%u\t: icmp_seq=%u\t ttl=%u\trtt=%.3f\tms",
            len,
            inet_ntoa(m_from_addr.sin_addr),
            pIcmp->icmp_seq,
            recv_ip->ip_ttl,
            rtt);
        return -1;
    }
    return 1;
}

void Linuxping::tv_sub(struct timeval* out, struct timeval* in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 10000000;
    }
    out->tv_sec -= in->tv_sec;
}

void Linuxping::statistics(int sig)
{
    tv_sub(&m_end_tvrecv, &m_begin_tvrecv);
    m_dTotalResponseTimes = m_end_tvrecv.tv_sec * 1000 + (double)m_end_tvrecv.tv_usec / 1000;
    printf("--------statistics----------\n");
    printf("%d packets transmitted, %d received, %d%% lost, time: %.3lfms\n", m_nSend, m_nRecv, (m_nSend - m_nRecv) / m_nSend * 100, m_dTotalResponseTimes);
    close(m_nSocketfd);
}

unsigned short Linuxping::cal_chksum(unsigned short* addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short* w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}
#endif
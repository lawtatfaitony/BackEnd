#ifndef WIN32
#include <string>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>


#define PACKET_SIZE 4096
#define SEND_DATA_LEN 56
#define ERROR    -1
#define SUCCESS   1
#define MAX_WAIT_TIME 3
#define MAX_NO_PACKETS 4
#define NULL 0


class Linuxping
{
public:
    Linuxping();
    Linuxping(const Linuxping& org);
    virtual ~Linuxping();

private:

    int m_nSend;
    int m_nRecv;
    struct sockaddr_in m_dest_addr;
    struct sockaddr_in m_from_addr;

    char m_sendpacket[PACKET_SIZE];
    char m_recvpacket[PACKET_SIZE];

    struct timeval m_tvrecv;
    struct timeval m_begin_tvrecv;
    struct timeval m_end_tvrecv;
    double m_dTotalResponseTimes;
    int m_nSocketfd;

public:
    bool Ping(const std::string& strIp);
    bool create_socket(const std::string& strIp);
    bool close_socket();

    void send_packet(void);
    void recv_packet(void);

    int pack(int pack_no);
    int unpack(char* buf, int len);

    void tv_sub(struct timeval* out, struct timeval* in);
    void statistics(int sig);

    static unsigned short cal_chksum(unsigned short* addr, int len);
};

#endif
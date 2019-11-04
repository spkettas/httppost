/********************************************
 *
 * @brief pcap�ļ���д��,������PCAPͷ�ļ�����
 * @author kettas
 * @date 2016/3/25
 *
 *******************************************/

#ifndef _PCAP_OPER_H
#define _PCAP_OPER_H


#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


#define  PACKET_SIZE                    (140)
#define  COOKED_CAPTURE_SIZE            (16)
#define  IP_HEADER_SIZE             	(20)
#define  UDP_HEADER_SIZE          		(8)
#define  RTP_HEADER_SIZE           		(12)
#define  RTP_DATA_SIZE                  (84)
#define  MAX_FILENAME_LEN               (64)


// pcap�ļ�ͷ
typedef struct pcap_file_header_t{
    unsigned int magic;
    unsigned short int version_major;
    unsigned short int version_minor;
    unsigned int thiszone;
    unsigned int sigfigs;
    unsigned int snaplen;
    unsigned int linktype;
}PcapFileHeader_t;	//24bytes


// pcap��ͷ
typedef struct pcap_pkt_header{
    unsigned int unGmtTime;
    unsigned int unMicroTime;
    unsigned int unCapLen;
    unsigned int unLen;
}PcapPktHeader_t;	//16bytes


// IP
typedef struct linux_ip_protocol_header
{
    unsigned char ucVersionAndHeaderLen;
    unsigned char ucServiceField;
    unsigned short usTotalLen;//124
    unsigned short usIdentification;
    unsigned short usFlagsAndFragment;
    unsigned char ucTimeToLive;
    unsigned char ucProtocol;
    unsigned short usHeaderChecksum;
    unsigned int  unSource;
    unsigned int  unDest;
}IpProtocolHeader_t; 	//20bytes

// UDP
typedef struct linux_udp_header{
    unsigned short usSrcPort;
    unsigned short usDestPort;
    unsigned short usLen;  //104
    unsigned short usChecksum;
}UdpHeader_t;         //8 bytes

// RTP
typedef struct rtp_header{
    unsigned char ucVersionPaddingExtendIdCount;
    unsigned char ucMarkerAndPayLoadType;
    unsigned short usSeq;
    unsigned int unTimeStamp;
    unsigned int unSyncSrcIdent;
}RtpHeader_t; //12bytes


// PCAP
typedef struct _pcap_obj
{
    FILE *fpOutput;
    char cFilenameArray[MAX_FILENAME_LEN];
}PcapObj;



// pcap�ļ���д����
static int PcapOpen( const char *filename,PcapObj **ppcap );
static int PcapClose( PcapObj *pcap );
static int PcapWritePkt( PcapObj *pcap,char *ppkt,int len );

static void PcapInitFileHeader( PcapFileHeader_t *rpFileHeader );
static void PcapInitPktHeader( PcapPktHeader_t *rpPktHeader,int pktlen );

// ��ӡ��Ϣ
static void PrintFileHeader(PcapFileHeader_t* rpFileHeader);
static void PrintPktHeader(PcapPktHeader_t* rpPktHeader);
static void PrintUdpHeader(UdpHeader_t* rpUdpHeader);
static void PrintIpHeader(IpProtocolHeader_t* rpIpHeader);

// д���ݰ�
bool AppendPkt( char *file,int count,char *buff,int len );


/**
 * ��ȡpcap�ļ�����UDP����
 * @param nPkts ��������0��ʾ������
 * @param dstip Ŀ�ķ�����
 */
int SendPcap( const char *fileName,uint32_t nPkts,char *dstip );


/**
 * @brief ׷дpcap�ļ�
 * @param file �ļ����ƣ���kettas.pcap
 * @param count �ɼ�����Ŀ
 * @param buff ��̫��֡
 * @param len ֡����
 * ����:("kettas.pcap",(char*)pdata,(unsigned int)(htons(iphead->Length)+14));
 */
bool AppendPkt( char *file,int count,char *buff,int len );



/// ��pcap�ļ� 
int PcapOpen( const char *filename,PcapObj **ppcap )
{
    PcapObj *pcap = NULL;
    FILE *fop = NULL;
    if(ppcap == NULL || filename == NULL)
        return -1;

    *ppcap = NULL;
    pcap = (PcapObj*)calloc(1,sizeof(PcapObj));
    if(pcap == NULL)
        return -2;

    if( (fop=fopen(filename,"wb")) )
    {
        pcap->fpOutput = fop;
        strncpy(pcap->cFilenameArray,filename,sizeof(pcap->cFilenameArray));
        *ppcap = pcap;

        // ��ʼ���ļ�ͷ
        PcapFileHeader_t fileHeader;
        PcapInitFileHeader( &fileHeader );
        fwrite( &fileHeader,sizeof(PcapFileHeader_t),1,pcap->fpOutput );

        return 0;
    }

    // ����
    free(pcap);
    return -3;
}


// �ر�pcap�ļ�
int PcapClose( PcapObj *pcap )
{
    if( pcap != NULL )
    {
        if(pcap->fpOutput)
        {
            fclose(pcap->fpOutput);
        }

        free(pcap);
        pcap = NULL;
    }

    return 0;
}


// д�����ݰ���֡���ݰ�
int PcapWritePkt( PcapObj *pcap,char *ppkt,int len )
{
    // ��ͷ
    PcapPktHeader_t pktHeader;
    PcapInitPktHeader( &pktHeader,len );
    fwrite( &pktHeader,sizeof(PcapPktHeader_t),1,pcap->fpOutput);

    // ������
    fwrite( ppkt,len,1,pcap->fpOutput );
    fflush( pcap->fpOutput );
    return 0;
}


// pcap�ļ�ͷ��ʼ��
void PcapInitFileHeader(PcapFileHeader_t* rpFileHeader)
{
    char* pmagic = (char*) (&rpFileHeader->magic);
    pmagic[0] = 0xd4;
    pmagic[1] = 0xc3;
    pmagic[2] = 0xb2;
    pmagic[3] = 0xa1;

    rpFileHeader->version_major = 2;
    rpFileHeader->version_minor = 4;
    rpFileHeader->thiszone = 0;
    rpFileHeader->sigfigs = 0;
    rpFileHeader->snaplen = UINT_MAX;
    rpFileHeader->linktype = 1;
}

// pcap��ͷ��ʼ��
void PcapInitPktHeader( PcapPktHeader_t *rpPktHeader,int pktlen )
{
    struct timeval ts;
    rpPktHeader->unCapLen = pktlen;
    rpPktHeader->unLen      = pktlen;
    gettimeofday(&ts,NULL);
    rpPktHeader->unGmtTime = (unsigned int)ts.tv_sec;
    rpPktHeader->unMicroTime = (unsigned int)ts.tv_usec;
}


// ��ӡ�ļ�ͷ
void PrintFileHeader( PcapFileHeader_t* rpFileHeader )
{
    assert( rpFileHeader );
    printf( "magic:%x\n"
            "version_major:%d\n"
            "version_minor:%d\n"
            "timezone:%d\n"
            "snaplen:%d\n"
            "linktype:%d\n",
            rpFileHeader->magic,
            rpFileHeader->version_major,
            rpFileHeader->version_minor,
            rpFileHeader->thiszone,
            rpFileHeader->snaplen,
            rpFileHeader->linktype );
}


// ��ӡ��ͷ
void PrintPktHeader( PcapPktHeader_t* rpPktHeader )
{
    assert( rpPktHeader );
    printf( "gmt time:%lf\n"
            "caplen:%d\n"
            "len:%d\n",
            rpPktHeader->unGmtTime%60 + rpPktHeader->unMicroTime/1000000.0,
            rpPktHeader->unCapLen,
            rpPktHeader->unLen );
}


// ��ӡIPͷ
void PrintIpHeader(IpProtocolHeader_t* rpIpHeader)
{
    assert(rpIpHeader);
    printf( "total_len:0x%x\n"
            "protocol:0x%x\n",
            ntohs(rpIpHeader->usTotalLen),
            rpIpHeader->ucProtocol);
}


// ��ӡUDPͷ
void PrintUdpHeader(UdpHeader_t* rpUdpHeader)
{
    assert(rpUdpHeader);
    printf( "src_port:%d\n"
            "dest_port:%d\n"
            "len:%d\n",
            ntohs(rpUdpHeader->usSrcPort),
            ntohs(rpUdpHeader->usDestPort),
            ntohs(rpUdpHeader->usLen) );
}


// ��ȡpcap�ļ�������ԭʼ�׽��ַ�����������
int SendPcap( const char *fileName,uint32_t nPkts,char *dstip )
{
    PcapFileHeader_t file_header;
    PcapPktHeader_t pkt_header;

    unsigned long ulTotalCnt = 0;
    unsigned long ulTotalBytes  = 0;
    unsigned int count = 0;

    char buffer[2048];
    struct ip *iph;

    // UDP
    int m_sock = socket( AF_INET,SOCK_DGRAM,0 );

    struct sockaddr_in m_addr;
    memset( &m_addr, 0, sizeof(struct sockaddr_in) );
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(1813);
    inet_pton( AF_INET,dstip,&m_addr.sin_addr );

    // ��ȡPCAP�ļ�
    FILE *fp = fopen(fileName,"rb");
    assert( fp != NULL );

    // PCAPͷ��
    int cnts = fread( &file_header,sizeof(PcapFileHeader_t),1,fp );

    while( !feof(fp) )
    {
        if( nPkts >0 && ulTotalCnt >= nPkts )
            break;

        // ��ͷ
        memset(buffer,0,sizeof(buffer));
        cnts = fread( &pkt_header,sizeof(PcapPktHeader_t),1,fp );

        if( cnts == 1 )
        {
            // ������
            unsigned int bytes = fread( &buffer[0],sizeof(char),pkt_header.unCapLen,fp );
            if( bytes == pkt_header.unCapLen )
            {
                iph = (struct ip*)( buffer + sizeof(struct ethhdr) );
                char *data = NULL;
                int sLen = 0;

                // TCP/UDP��Data����
                if( iph->ip_p == IPPROTO_TCP )
                {
                    data = (char*)iph + sizeof(struct ip) + sizeof(struct tcphdr);
                    sLen = htons( iph->ip_len ) - sizeof(struct ip) - sizeof(struct tcphdr); // tcp���������
                }
                else if( iph->ip_p == IPPROTO_UDP )
                {
                    struct udphdr *udp = (struct udphdr*)((char*)iph + sizeof(struct ip));
                    data  = (char*)udp + sizeof(struct udphdr);
                    sLen = htons( udp->len ) - 8;	// udp���Ȱ���ͷ��
                }
                else
                {
                    continue;
                }

                // ��UDP��ʽ���ͣ��������
                int len = sendto( m_sock,
                                  data,
                                  sLen,
                                  0,
                                  (struct sockaddr *)&m_addr,
                                  sizeof(struct sockaddr_in) );

                printf( "Seq:%d len:%d\n",++count,len );
                if( len > 0 )
                {
                    ulTotalCnt++;
                    ulTotalBytes += sizeof(PcapPktHeader_t) + pkt_header.unCapLen;
                }
            }
        }//if(cnts;)

        usleep(100);
    }//while(;)

    printf( "Total:%ldp Bytes:%ldB\n",
            ulTotalCnt,
            ulTotalBytes );

    fclose( fp );
    close( m_sock );
    return 0;
}


/**
 * @brief дpcap�ļ�
 * @param file �ļ����ƣ���kettas.pcap
 * @param count �ɼ�����Ŀ
 * @param buff ��̫��֡
 * @param len ֡����
 * ����:("kettas.pcap",(char*)pdata,(unsigned int)(htons(iphead->Length)+14))
 */
bool AppendPkt( char *file,int count,char *buff,int len )
{
    static PcapObj *pcap = NULL;
    static int pCount = 0;

    // ����
    if( pCount && (pCount%count == 0) )
        return true;

    // ��ʼ��
    if( pcap == NULL )
    {
        char path[100];
        sprintf( path,"/home/%s",file );
        PcapOpen( path,&pcap );
    }

    // д����
    if( len > 0 )
    {
        PcapWritePkt( pcap,buff,len );
        pCount++;
    }

    // �ر�
    if( pCount%count == 0 )
    {
        PcapClose( pcap );
        exit(1);
    }

    return true;
}

#endif	// _PCAP_OPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void printBin(const unsigned char * buff, int len){
	for(int i=0; i<len; ++i){
		printf("%02X ", buff[i]);
		if((i+1)%16==0){
			printf("\n");
		}
	}
}

void writeShort(char * buff, int val)
{
	buff[0] = (val>>8)&0xff;
	buff[1] = val&0xff;
}

void write3Byte(char * buff, int val)
{
	buff[0] = (val>>16)&0xff;
	buff[1] = (val>>8)&0xff;
	buff[2] = val&0xff;
}

void writeInt(char * buff, int val)
{
	buff[0] = (val>>24)&0xff;
	buff[1] = (val>>16)&0xff;
	buff[2] = (val>>8)&0xff;
	buff[3] = val&0xff;
}

int setFileHeader(char * buff, int len)
{
	memcpy(buff, "FLV", 3);
	buff[3] = 1;
	buff[4] = 1; // 1 video only 4 audio only 5 video and audio
	buff[5] = 0;
	buff[6] = 0;
	buff[7] = 0;
	buff[8] = 9;
	return 9;
}

int setTag(char *buff, int len, 
	int previousLen, char tagtype, 
	int taglen,
	unsigned int timestamp, int streamid)
{
	char * temp = (char*)&previousLen;
	buff[0] = temp[3];
	buff[1] = temp[2];
	buff[2] = temp[1];
	buff[3] = temp[0];
	buff[4] = tagtype;
	temp = (char*)&taglen;
	buff[5] = temp[2];
	buff[6] = temp[1];
	buff[7] = temp[0];
	if(tagtype == 0x12)
	{
		buff[8] = 0;
		buff[9] = 0;
		buff[10] = 0;
		buff[11] = 0;
	}else{
		temp = (char*)&timestamp;
		buff[8] = temp[2];
		buff[9] = temp[1];
		buff[10] = temp[0];
		buff[11] = temp[3];
	}
	temp = (char*)&streamid;
	buff[12] = temp[2];
	buff[13] = temp[1];
	buff[14] = temp[0];
	return 15;
}

int setAMFFirstDataPackage(char *buff, int len)
{
	buff[0] = 0x02;
	buff[1] = 0x00;
	buff[2] = 0x0A;
	memcpy(buff+3, "onMetaData", 10);
	return 13;
}

int setAMFDouble(char *buff, int len, const char *name , double value)
{
	int pos = 0;
	short namelen = strlen(name);
	char * tempchr = (char *)&namelen;
	buff[pos++] = tempchr[1];
	buff[pos++] = tempchr[0];
	memcpy(buff+pos, name, strlen(name));
	pos+=strlen(name);
	buff[pos++] = 0;
	tempchr = (char *)&value;
	buff[pos++] = tempchr[7];
	buff[pos++] = tempchr[6];
	buff[pos++] = tempchr[5];
	buff[pos++] = tempchr[4];
	buff[pos++] = tempchr[3];
	buff[pos++] = tempchr[2];
	buff[pos++] = tempchr[1];
	buff[pos++] = tempchr[0];
	return pos;
}

int setAMFBoolean(char *buff, int len, const char *name , char value)
{
	int pos = 0;
	short namelen = strlen(name);
	char * tempchr = (char *)&namelen;
	buff[pos++] = tempchr[1];
	buff[pos++] = tempchr[0];
	memcpy(buff+pos, name, strlen(name));
	pos+=strlen(name);
	buff[pos++] = 1;
	buff[pos++] = value;
	return pos;
}

int setAMFString(char *buff, int len, const char *name , const char * value)
{
	int pos = 0;
	short namelen = strlen(name);
	char * tempchr = (char *)&namelen;
	buff[pos++] = tempchr[1];
	buff[pos++] = tempchr[0];
	memcpy(buff+pos, name, strlen(name));
	pos+=strlen(name);
	buff[pos++] = 2;
	namelen = strlen(value);
	tempchr = (char *)&namelen;
	buff[pos++] = tempchr[1];
	buff[pos++] = tempchr[0];
	memcpy(buff+pos, value, namelen);
	pos += namelen;
	return pos;
}

int setAMFSecondPackage(char *buff, int len)
{
	int pos = 0;
	buff[pos++] = 0x08;
	buff[pos++] = 0x00;
	buff[pos++] = 0x00;
	buff[pos++] = 0x00;
	buff[pos++] = 0x08;
	pos+=setAMFDouble(buff+pos, len-pos, "duration" , 0);
	pos+=setAMFDouble(buff+pos, len-pos, "width" , 704);
	pos+=setAMFDouble(buff+pos, len-pos, "height" , 576);
	pos+=setAMFBoolean(buff+pos, len-pos, "canSeekToEnd" , 0);
	pos+=setAMFDouble(buff+pos, len-pos, "framerate" , 25);
	pos+=setAMFDouble(buff+pos, len-pos, "filesize" , 0);
	pos+=setAMFDouble(buff+pos, len-pos, "videocodecid" , 7);
	pos+=setAMFDouble(buff+pos, len-pos, "videodatarate" , 0);
	buff[pos++] = 0x00;
	buff[pos++] = 0x00;
	buff[pos++] = 0x09;

	return pos;
}

int setVideoPackageHeader(char *buff, int len, char isI, char isNalu, int timestamp)
{
	char * temp = (char*) &timestamp;
	buff[0] = isI ? 0x17 : 0x27;
	buff[1] = isNalu;
	buff[2] = temp[2];
	buff[3] = temp[1];
	buff[4] = temp[0];
	return 5;
}

int setAVCConfigPackage(char *buff, int len, char * sps, int spslen, char * pps, int ppslen)
{
	char * tempchr = NULL;
	int pos = 0;
	buff[pos++] = 01;
	buff[pos++] = 0x4d; //0x42; //0x7a;
	buff[pos++] = 0x00; //0xe0; //0x00;
	buff[pos++] = 0x1e; //0x14; //0x1f;
	buff[pos++] = 0xff;//ff
	buff[pos++] = 0xe1;
	tempchr = (char *)&spslen;
	buff[pos++] = tempchr[1]; buff[pos++] = tempchr[0];//08;
	memcpy(buff+pos, sps, spslen ) ;//67 42 E0 14 DA 05 82 51  // sps
	pos+=spslen;
	buff[pos++] = 0x01;
	tempchr = (char *)&ppslen;
	buff[pos++] = tempchr[1]; buff[pos++] = tempchr[0];//05;
	memcpy(buff+pos, pps, ppslen ) ;//68 ce 30 a4 80  // pps
	pos+=ppslen;
	return pos;
}

#pragma pack(1)
//PG��ͷ�ṹ
typedef struct tagPGPackHead
{
	unsigned char Version;							//�汾��
	unsigned short nCodeType;						//��������
	unsigned char cHeadLen;							//��ͷ����
	unsigned short nAttrCount;						//���Ը���
	unsigned short nSeqNum;							//�����
	unsigned short nSessionId;						//�ỰID
	unsigned short nTotalLength;					//�ܳ���
}PGPACK_HEAD;

//PG����ֵ�ṹ
typedef struct tagAttribHead
{
	unsigned char nAttrType;						//����
	unsigned char nReserved;						//����
	unsigned short nLength;							//���Գ���
}ATTRIB_HEAD;
#pragma  pack()

#pragma pack(4)
typedef struct tagCptypeVideoData
{
	unsigned char ucVideoType;						//��Ƶ���ݵı����ʽ��0-MPEG1 1-MPEG2 4-MPEG 8-MJPEG 16-H264
	unsigned char ucFrameType;						//֡���͡�0, I ֡ 1, P ֡ 2, B֡ 
	unsigned char ucAckFlag;						//Ack �ذ���־���������־Ϊ��0 ʱ��Ҫ��ͻ��˸�����������һ��ack��RPU һ����һ֡��Ƶ�����һ������Ack ��־��
	unsigned char ucReserved;						//�����ֽ�
	unsigned short ucPacketNum;						//һ������������֡��Ҫͨ�����UDP ���Ľ��д��䡣Packet number ָ���䵱ǰһ֡���ݵ�UDP ���ĵĸ�����
	unsigned short ucSeqNum;						//�˰���һ֡�еı�ţ���0 ��ʼ��ţ�0��1��2��3����
	unsigned char ucFramePerSec;					//��RPU ������֡���ʡ�ÿ���ӵĲ�����֡��Ŀ��
	unsigned char ucIFreq;							//I֡���ֵ�Ƶ�ʡ�ÿ��I Freq����֡�а���һ��I ֡������Ķ���P ֡��
	unsigned char ucReserved_2[2];					//�����ֶ�,��ǰֵΪ0                 
	unsigned char ucVideoId;						//��Ƶͨ��,��1��ʼ
	unsigned char ucVideoFlag;						//��Ƶ������������
//	unsigned char ucDataBegin1;						//���һ֡����ѹ����С��64K Byte����ô���ֶ�Ϊ0�����һ֡����ѹ���󳬹�64K Byte���������ݰ�����ȷ����ֵ.
	//unsigned short usDataBegin2;					//ָ�����ڵ�ǰ����Ƶ֡�п�ʼ��λ�á���֡ͷ��ʼ���㣬���ֽ�Ϊ��λ�������ڵ�ǰ��Ƶ֡�Ŀ�ʼλ�ã�Data Begin (1)*64K +Data Begin (2).
	unsigned short usDataLength;					//�ڴ˰�����Ƶ���ݵĳ��ȡ����ֶκ�Data Begin ��ͬ�����˰��е�������һ֡������Ƶ����һ�������ݡ�Ϊ�˷�ֹ·����·�����򽻻�����VSP ���Ľ��з�Ƭ��DataLength ���ô���1400byte��
	unsigned int usDataBegin;						//ָ�����ڵ�ǰ����Ƶ֡�п�ʼ��λ�á���֡ͷ��ʼ���㣬���ֽ�Ϊ��λ�������ڵ�ǰ��Ƶ֡�Ŀ�ʼλ�ã�Data Begin (1)*64K +Data Begin (2).
	unsigned int uiFrameID;							//֡�ı��, ��0 ��ʼ��ţ�RPU ÿ�²���һ֡��֡ID ��1��
	unsigned int uiFrameLength;						//֡���ܳ��ȣ����ֽ�Ϊ��λ��
	unsigned int uiTimeStamp;						//��1970 ��1 ��1 ��00:00:00��coordinated universal time��������������
	unsigned int uiTimeStampEx;						//ʱ���΢�벿�ݡ�
//	char *pszVideoData;								//���������������ݲ��֡�
} CPTYPE_VIDEODATA;

//��Ƶ������Ϣ�Ľṹ�嶨�壺35
typedef struct tagCptypeAudioData
{
	unsigned char ucAudioType;						//��Ƶ�����ʽ����ǰֻ֧��һ��ֵ,1:G.711
	unsigned char ucFramePerSec;					//RPU ÿ���Ӳ�����֡��Ŀ��
	unsigned char ucAckFlag;						//0-��Ҫ���Ack ����1-Ҫ���Ack ����RPU һ����һ֡��Ƶ�����һ������Ack ��־����ʱ��Client ��Ҫ���ݴ˱�־�������Ƿ��Ack ����
	unsigned char ucPacketNum;						//һ������������֡��Ҫͨ�����UDP ���Ľ��д��䡣Packet number ָ���䵱ǰһ֡���ݵ�UDP ���ĵĸ�����
	unsigned char ucSeqNum;							//ָ�˰���һ֡�еı�ţ���0 ��ʼ��ţ�0��1��2��3����
	unsigned char ucAdudioId;						//��Ƶͨ��ID,һ��ֻ��һ��,��1��ʼ
	unsigned char szReserve[2];						//����������չ�ֶ��á�
	unsigned short usDataBegin;						//ָ�����ڵ�ǰ����Ƶ֡�п�ʼ��λ�á���֡ͷ��ʼ���㣬���ֽ�Ϊ��λ��
	unsigned short usDataLength;					//�ڴ˰�����Ƶ���ݵĳ��ȡ����ֶκ�Data Begin ��ͬ�����˰��е�������һ֡������Ƶ����һ�������ݡ�Ϊ�˷�ֹ·����·�����򽻻�����VSP ���Ľ��з�Ƭ��Data Length ���ô���1400byte��
	unsigned int uiFrameID;							//֡�ı��, ��0 ��ʼ��ţ�RPU ÿ�²���һ֡��֡ID ��1��
	unsigned int uiFrameLength;						//֡���ܳ���,���ֽ�Ϊ��λ
	unsigned int uiTimeStamp;						//��1970 ��1 ��1 ��00:00:00��coordinated universal time������������
	unsigned int uiTimeStampEx;						//ʱ���΢�벿�ݡ�
//	char *pszAudioData;								//������������Ƶ���ݿ�ʼ�Ĳ��֡�
} CPTYPE_AUDIODATA;

typedef struct tagCptypeDataAck
{
	unsigned char ucAckType;						//Ack ���ͣ�1-��Ƶ,��ʾ����Ƶ���ݻ�ACK 2-��Ƶ,��ʾ����Ƶ���ݻ�ACK
	unsigned char ucReserved;						//�������ڽ�����չ�ֶ���
	unsigned char ucRecvPackets;					//�յ��İ��ĸ�����ָ�����յ���һ֡�еİ��ĸ�����
	unsigned char ucDropPackets;					//�����İ��ĸ���������Ϊ�����ԭ����ɰ��ڴ�������б�������Client û���յ�
	unsigned char ucVAID;
	unsigned char ucVAFlag;
	unsigned char ucReserved_2[2];
	unsigned int uiFrameID;							//֡��ID���ո��յ���֡��ID��
	//unsigned char ucDropPktID[];					//��������ID�����ﶪ������ID �Ƕ�Ӧ��ǰ���SequenceNum ��һһ��Ӧ�ģ� ���������ЩSequence Num �İ�û�б��յ���������RPU �������Ա�RPU ��������������
} CPTYPE_DATAACK;

#pragma pack()

int convertPSToRawH264(unsigned char *src, int srclen, unsigned char * dst, int dstlen)
{
	char psStart[4] = {0,0,0,1};
	char psInfo[10] = {0};
	char pesMark[2] = {0};
	int pos = 0;
	int packmark = 0;
	int dstTotal = 0;
	int psDataLen = 0;
	int pesDataLen = 0;
	
	int i, j;
	for(; pos<srclen; )
	{
		if(memcmp(src+pos, psStart+1, 3)==0)
		{
			pos+=3;
			packmark = src[pos];
			pos++;
			//printf("----packmark %02X \n", packmark);
			if(packmark == 0xba) // ps header
			{
				memcpy(psInfo, src+pos, 9);
				pos+=9;
				pos+=(src[pos]&7)+1;
				continue;
			}
			psDataLen = (((int)src[pos])<<8)+src[pos+1];
			pos+=2;
			//printf("----psDataLen %d\n", packmark, psDataLen);
			if(packmark == 0xbb || packmark == 0xbc )
			{
				pos += psDataLen;
				continue;
			}
			//if( (packmark&0xc0)!=0xc0 && (packmark&0xe0)!=0xe0 )
			if( (packmark&0xe0)!=0xe0 )
			{
				pos += psDataLen;
				continue;
			}
			if((src[pos]&0x80)==0)
			{
				pos += 6+psDataLen;
				continue;
			}
			memcpy(pesMark, src+pos,2);
			pos+=2;
			if( (pesMark[1]&0x80) == 0x80 )
			{
			}
			pesDataLen = psDataLen - src[pos] - 3;
			pos+=src[pos]+1;
			//printf("pesDataLen offset %02X\n", pos);
			//printf("pesDataLen %d\n", pesDataLen);
			
			memcpy(dst+dstTotal, src+pos, pesDataLen);
			dstTotal += pesDataLen;
			//printf("----- dstTotal %d\n", dstTotal);
			
			pos+=pesDataLen;

		}else{
			++pos;
		}
	}
	//printf("----- end dstTotal %d\n", dstTotal);
	//exit(0);
	return dstTotal;
}


int GetSPS(char * buff, int len, char * sps, int spsBuffLen)
{
	char nalu[4] = {0,0,0,1};
	int i = 0;
	int j = 0;
	for(; i<len-3;++i)
	{
		if(memcmp(buff+i, nalu, 4)==0)
		{
			if((buff[i+4]&0x1f)==7)
			{
				j = i+5;
				for(; j<len-3;++j)
				{
					if(memcmp(buff+j, nalu, 4)==0)
					{
						memcpy(sps, buff+i+4, j-i-3);
						return j-i-4;
					}
				}
				memcpy(sps, buff+i+4, len-i-4);
				return len-i-4;
			}
		}
	}
	return 0;
}


int GetPPS(char * buff, int len, char * pps, int ppsBuffLen)
{
	char nalu[4] = {0,0,0,1};
	int i = 0;
	int j = 0;
	for(; i<len-3;++i)
	{
		if(memcmp(buff+i, nalu, 4)==0)
		{
			if((buff[i+4]&0x1f)==8)
			{
				j = i+4;
				for(; j<len-3;++j)
				{
					if(memcmp(buff+j, nalu, 4)==0)
					{
						memcpy(pps, buff+i+4, j-i-4);
						return j-i-4;
					}
				}
				memcpy(pps, buff+i+4, len-i-4);
				return len-i-4;
			}
		}
	}
	return 0;
}

#define PGVIDEOHEADERSLEN (sizeof(PGPACK_HEAD)+sizeof(ATTRIB_HEAD)+sizeof(CPTYPE_VIDEODATA))

#define BUFFER_SIZE 4*1024*1024

int testPgspStream()
{
		char sps[128] = {0};//{0x67, 0x4D, 0x00, 0x1E, 0x95, 0xA8, 0x2C, 0x04, 0x9A, 0x6E, 0x02, 0x02, 0x02, 0x04};

	char pps[128] = {0};//{0x68, 0xEE, 0x3C, 0x80};
	//int spslen =14, ppslen = 4;
	int spslen =0, ppslen = 0;

	char nalu[4] = {0, 0, 0, 1};
	char inbuff[2048];
	char * outBuff = (char*)malloc(BUFFER_SIZE);
	char * frameBuff = (char*)malloc(BUFFER_SIZE);
	int outLen = 0;
	int packegTotal = 0;
	int frameSize = 0;
	int tempLen = 0;
	int previousLen = 0;
	int timestamp = 0;
	int startTimeStamp = 0;
	char isSetAVCConfig = 0;
	FILE * inFile = NULL, * outFile = NULL, * h264File = NULL;

	PGPACK_HEAD * pgHeader = NULL;
	ATTRIB_HEAD * attrHead = NULL;
	CPTYPE_VIDEODATA * videoDataCFG = NULL;

	memset(outBuff, 0, BUFFER_SIZE );

	inFile = fopen("G:\\testdata\\pgProtocol.data", "rb");
	if(inFile!=NULL)
	{
		outFile = fopen("test001.flv", "wb");
		if(outFile!=NULL)
		{
			outLen += setFileHeader(outBuff, BUFFER_SIZE);
			tempLen = setAMFFirstDataPackage(outBuff+outLen+15, BUFFER_SIZE-outLen-15);
			tempLen += setAMFSecondPackage(outBuff+outLen+tempLen+15, BUFFER_SIZE-outLen-tempLen-15);
			outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x12, tempLen, timestamp, 0);
			outLen += tempLen;
			previousLen = tempLen+11;
			
			/*
			tempLen = setVideoPackageHeader(outBuff+outLen+15, BUFFER_SIZE-outLen-15, 1, 0, 0);
			tempLen += setAVCConfigPackage(outBuff+outLen+tempLen+15, BUFFER_SIZE-outLen-tempLen-15, sps, spslen, pps, ppslen);
			outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, tempLen, timestamp, 0);
			previousLen = tempLen+11;
			outLen += tempLen;
			*/

			//h264File = fopen("test01.h264", "wb");

			while(!feof(inFile))
			{
				fread(inbuff, 1, sizeof(PGPACK_HEAD), inFile);
				pgHeader = (PGPACK_HEAD*)inbuff;
				packegTotal = pgHeader->nTotalLength;
				fread(inbuff+sizeof(PGPACK_HEAD), 1, packegTotal-sizeof(PGPACK_HEAD), inFile);
				attrHead = (ATTRIB_HEAD*)(inbuff+sizeof(PGPACK_HEAD));
				videoDataCFG = (CPTYPE_VIDEODATA *)(inbuff+sizeof(PGPACK_HEAD)+sizeof(ATTRIB_HEAD) );
				if(attrHead->nAttrType == 34)
				{

						if(spslen==0)
						{
							spslen = GetSPS(inbuff+PGVIDEOHEADERSLEN, videoDataCFG->usDataLength, sps, 128);
							if(spslen>0)
							{
								printf("sps:\n");
							printBin((const unsigned char*)sps, spslen);
								printf("\n");
							}
						}
						if(ppslen==0)
						{
							ppslen = GetPPS(inbuff+PGVIDEOHEADERSLEN, videoDataCFG->usDataLength, pps, 128);
							if(ppslen>0)
							{
								printf("pps:\n");
							printBin((const unsigned char*)pps, ppslen);
								printf("\n");
							}
						}

					//memcpy(frameBuff+videoDataCFG->usDataBegin, inbuff+PGVIDEOHEADERSLEN, videoDataCFG->usDataLength);
					memcpy(frameBuff+frameSize, inbuff+PGVIDEOHEADERSLEN, videoDataCFG->usDataLength);
					frameSize+=videoDataCFG->usDataLength;
					//	fwrite(inbuff+PGVIDEOHEADERSLEN,1,videoDataCFG->usDataLength,h264File);
					//if(videoDataCFG->ucPacketNum==videoDataCFG->ucSeqNum+1)
					if(videoDataCFG->ucAckFlag>0)
					{
						if(spslen>0 && ppslen>0)
						{
							if(BUFFER_SIZE-outLen<frameSize+25)
							{
								fwrite(outBuff, 1,outLen, outFile );
								memset(outBuff, 0, BUFFER_SIZE);
								outLen = 0;
							}
							
							if(!isSetAVCConfig)
							{
								outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, 5+11+spslen+ppslen, timestamp, 0);
								tempLen = setVideoPackageHeader(outBuff+outLen, BUFFER_SIZE-outLen, 1, 0, 0);
								tempLen += setAVCConfigPackage(outBuff+outLen+tempLen, BUFFER_SIZE-outLen-tempLen, sps, spslen, pps, ppslen);
								previousLen = tempLen+11;
								outLen += tempLen;

								outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, 5+spslen+ppslen+6, timestamp, 0);
								tempLen = setVideoPackageHeader(outBuff+outLen, BUFFER_SIZE-outLen, 1, 1, 0);
								memcpy(outBuff+outLen+tempLen, nalu+1, 3);
								tempLen+=3;
								memcpy(outBuff+outLen+tempLen, sps, spslen);
								tempLen+=spslen;
								memcpy(outBuff+outLen+tempLen, nalu+1, 3);
								tempLen+=3;
								memcpy(outBuff+outLen+tempLen, pps, ppslen);
								tempLen+=ppslen;
								previousLen = tempLen+11;
								outLen += tempLen;
								isSetAVCConfig = 1;
							}
							{
								if(startTimeStamp==0)
								{
									startTimeStamp = videoDataCFG->uiTimeStamp*1000+videoDataCFG->uiTimeStampEx/1000;
								}
								outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, 5+frameSize, videoDataCFG->uiTimeStamp*1000+videoDataCFG->uiTimeStampEx/1000 - startTimeStamp, 0);
								tempLen = setVideoPackageHeader(outBuff+outLen, BUFFER_SIZE-outLen, 1, 1, videoDataCFG->uiTimeStamp*1000+videoDataCFG->uiTimeStampEx/1000 - startTimeStamp);
								memcpy(outBuff+outLen+tempLen, frameBuff, frameSize);
								tempLen += frameSize;
								previousLen = tempLen+11;
								outLen += tempLen;
							}
						}
						frameSize = 0;
					}
				}
			}
			if(outLen>0)
			{
				char * tempP = (char*)&previousLen;
				fwrite(outBuff, 1,outLen, outFile );
				outBuff[0] = tempP[2];
				outBuff[1] = tempP[1];
				outBuff[2] = tempP[0];
				fwrite(outBuff, 1,3, outFile );
			}
			fclose(outFile);
		}
		fclose(inFile);
	}
	//fclose(h264File);

	
	
	return 0;
}

int getNaluData(char * buff, int len)
{
	char nalu[4] = {0,0,0,1};
	int i = 0;
	int j = 0;
	for(; i<len-2;++i)
	{
		if(memcmp(buff+i, nalu, 4)==0)
		{
			j = i+4;
			for(; j<len-2;++j)
			{
				if(memcmp(buff+j, nalu, 4)==0)
				{
					return j-i;
				}
			}
			return len;
		}
	}
	return 0;
}

int fixNaluData(char * buff, int len)
{
	char nalu[4] = {0,0,0,3};
	int i = 0;
	for(; i<len-2;++i)
	{
		if(memcmp(buff+i, nalu, 4)==0)
		{
			memcpy(buff+i+2, buff+i+4, len-i-4);
			len --;
			i+=2;
		}
	}
	return len;
}

int findNalStart(char * buff, int start, int end)
{
	char nalu[4] = {0,0,0,1};
	for(; start<end;++start)
	{
		if(memcmp(buff+start, nalu, 4)==0)
		{
			return start;
		}
	}
	return end;
}

int findFrameStart(char * buff, int len)
{
	char startframe[8] = {0, 0, 0, 1, 9, 0xf0,0,0};
	int start = 0;
	int end = 0;

	int tempStart = 0, tempEnd = 0;

	char hasSps = 0, hasPps = 0, hasIdr;

	int dstTotal = 0;

	for(; start<len;++start)
	{
		start = findNalStart(buff, start, len);
		if(start<len)
		{
			if(memcmp(buff+start, startframe, 8)==0)
			{
				//printf("start %d \n", start);
				end = start + 8;
				for(; end<len; ++end)
				{
					end = findNalStart(buff, end, len);
					if(memcmp(buff+end, startframe, 8)==0)
					{
						break;
					}
				}
				tempStart = start;
				for(;;++tempStart)
				{
					tempStart = findNalStart(buff, tempStart+4, len);
					if(tempStart>=end)
					{
						break;
					}

					//printf("tempStart %d end %d \n", tempStart, end);
					//printf("buff[tempStart+4] %02X \n", buff[tempStart+4]);
					switch(buff[tempStart+4]&0x1f)
					{
						case 5:
							//printf("--- has idr\n");
							hasIdr = 1;
						break;
						case 7:
							//printf("--- has sps\n");
							hasSps = 1;
						break;
						case 8:
							//printf("--- has pps\n");
							hasPps = 1;
						break;
					}
				}
				if(hasIdr && hasSps && hasPps)
				{
					return start;
				}
				start = end - 1;
			}
		}
	}
	return 0;
}

int convertToMp4Raw(char * src, int srclen, char * dst, int dstlen)
{
	int start = 0;
	int end = 0;

	int dstTotal = 0;

	start = findFrameStart(src, srclen);
	//printf("start %d \n", start);
	//printBin( (unsigned char *)src+start, 16 );
	for(; start<srclen; )
	{
		end = findNalStart(src, start+4, srclen);
		if( (src[start+4]&0x80)==0)
		{
			writeInt(dst+dstTotal, end - start - 4 );
			dstTotal += 4;
			memcpy(dst+dstTotal, src+start+4, end - start - 4);
			dstTotal += end - start - 4;
		}
		start = end;
	}
	return dstTotal;
}

int getMp4FrameSize(char * buff, int len)
{
	char framestart[8] = {0,0,0,4,9,0xf0,0,0};
	int i = 0;
	int j = 0;
	for(; i<len;++i)
	{
		if(memcmp(buff+i, framestart, 8)==0)
		{
			j = i+8;
			for(; j<len;++j)
			{
				if(memcmp(buff+j, framestart, 8)==0)
				{
					return j-i;
				}
			}
			return len;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{

	char sps[128] = {0};//{0x67, 0x4D, 0x00, 0x1E, 0x95, 0xA8, 0x2C, 0x04, 0x9A, 0x6E, 0x02, 0x02, 0x02, 0x04};

	char pps[128] = {0};//{0x68, 0xEE, 0x3C, 0x80};
	//int spslen =14, ppslen = 4;
	int spslen =0, ppslen = 0;

	char nalu[4] = {0, 0, 0, 1};
	char * inbuff = (char*)malloc(BUFFER_SIZE);
	char * outBuff = (char*)malloc(BUFFER_SIZE);
	char * mp4Buff = (char*)malloc(BUFFER_SIZE);
	int outLen = 0;
	int packegTotal = 0;
	int frameSize = 0;
	int rawSize = 0;
	int tempLen = 0;
	int previousLen = 0;
	int timestamp = 0;
	char isSetAVCConfig = 0;
	char isSetfirst = 0;
	unsigned char * tempChr = NULL;
	FILE * inFile = NULL, * outFile = NULL, * h264File = NULL;

	memset(outBuff, 0, BUFFER_SIZE );

	inFile = fopen("G:\\testdata\\pgdata.ps", "rb");
	if(inFile!=NULL)
	{
		outFile = fopen("test001.flv", "wb");
		if(outFile!=NULL)
		{
			//h264File = fopen("test001.h264", "wb");
			outLen += setFileHeader(outBuff, BUFFER_SIZE);
			tempLen = setAMFFirstDataPackage(outBuff+outLen+15, BUFFER_SIZE-outLen-15);
			tempLen += setAMFSecondPackage(outBuff+outLen+tempLen+15, BUFFER_SIZE-outLen-tempLen-15);
			outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x12, tempLen, timestamp, 0);
			outLen += tempLen;
			previousLen = tempLen+11;
			
			/*
			tempLen = setVideoPackageHeader(outBuff+outLen+15, BUFFER_SIZE-outLen-15, 1, 0, 0);
			tempLen += setAVCConfigPackage(outBuff+outLen+tempLen+15, BUFFER_SIZE-outLen-tempLen-15, sps, spslen, pps, ppslen);
			outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, tempLen, timestamp, 0);
			previousLen = tempLen+11;
			outLen += tempLen;
			*/

			do
			{
				if(!feof(inFile) && packegTotal<sizeof(inbuff))
				{
					int rlen = fread(inbuff, 1, BUFFER_SIZE-packegTotal, inFile);
					if(rlen>0)
					{
						packegTotal += rlen;
					}
				}

				//if(spslen>0 && ppslen>0)
				{
					if(BUFFER_SIZE-outLen<frameSize+25)
					{
						fwrite(outBuff, 1,outLen, outFile );
						memset(outBuff, 0, BUFFER_SIZE);
						outLen = 0;
					}
					
					packegTotal = convertPSToRawH264((unsigned char *)inbuff, packegTotal, (unsigned char *)mp4Buff, BUFFER_SIZE);
					//printf("------------------- packegTotal %d\n", packegTotal);
					//fwrite(mp4Buff, 1,packegTotal, h264File );
					//printBin((unsigned char *)mp4Buff, 16);
					memcpy(inbuff, mp4Buff, packegTotal);

					if(!isSetAVCConfig)
					{
						if(spslen==0)
						{
							spslen = GetSPS(inbuff, packegTotal, sps, 128);
							if(spslen>0)
							{
								printf("sps:\n");
								printBin((const unsigned char*)sps, spslen);
								printf("\n");
							}
						}
						if(ppslen==0)
						{
							ppslen = GetPPS(inbuff, packegTotal, pps, 128);
							if(ppslen>0)
							{
								printf("pps:\n");
								printBin((const unsigned char*)pps, ppslen);
								printf("\n");
							}
						}
						outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, 5+11+spslen+ppslen, timestamp, 0);
						tempLen = setVideoPackageHeader(outBuff+outLen, BUFFER_SIZE-outLen, 1, 0, 0);
						tempLen += setAVCConfigPackage(outBuff+outLen+tempLen, BUFFER_SIZE-outLen-tempLen, sps, spslen, pps, ppslen);
						previousLen = tempLen+11;
						outLen += tempLen;

						isSetAVCConfig = 1;
					}

					packegTotal = convertToMp4Raw(inbuff, packegTotal, mp4Buff, BUFFER_SIZE);
					//fwrite(mp4Buff, 1,packegTotal, h264File );
					//fflush(h264File);
					//printf("--- packegTotal %d\n", packegTotal);
					memcpy(inbuff, mp4Buff, packegTotal);
					while(packegTotal>0)
					{
						char isIframe = 0;
						frameSize = getMp4FrameSize( inbuff, packegTotal );
						//printf("frameSize %d\n", frameSize);
						if(0==frameSize)
						{
							printBin((unsigned char *)inbuff, 16);
						}

						if(!isSetfirst)
						{
							isIframe =1;
							isSetfirst = 1;
						}

						timestamp += 40;

						outLen += setTag(outBuff+outLen, BUFFER_SIZE-outLen, previousLen, 0x9, 5+frameSize, timestamp, 0);
						tempLen = setVideoPackageHeader(outBuff+outLen, BUFFER_SIZE-outLen, isIframe, 1, timestamp);
						memcpy(outBuff+outLen+tempLen, inbuff, frameSize);

						tempLen += frameSize;
						previousLen = tempLen+11;
						outLen += tempLen;
											
						packegTotal -= frameSize;
						if(packegTotal>0)
						{
							memcpy(inbuff, inbuff+frameSize, packegTotal);
						}
						frameSize = 0;
						//printf("packegTotal %d\n", packegTotal);
					}
				}
			}while(!feof(inFile) || packegTotal>0);
			if(outLen>0)
			{
				tempChr = (unsigned char *)&previousLen;
				fwrite(outBuff, 1,outLen, outFile );
				//printBin( (unsigned char *)outBuff+outLen-16, 16);
				outBuff[0] = tempChr[2];
				outBuff[1] = tempChr[1];
				outBuff[2] = tempChr[0];
				fwrite(outBuff, 1,3, outFile );
			}
			fclose(outFile);
		}
		fclose(inFile);
		//fclose(h264File);
	}

	return 0;
}

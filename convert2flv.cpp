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
//PG包头结构
typedef struct tagPGPackHead
{
	unsigned char Version;							//版本号
	unsigned short nCodeType;						//报文类型
	unsigned char cHeadLen;							//包头长度
	unsigned short nAttrCount;						//属性个数
	unsigned short nSeqNum;							//包序号
	unsigned short nSessionId;						//会话ID
	unsigned short nTotalLength;					//总长度
}PGPACK_HEAD;

//PG属性值结构
typedef struct tagAttribHead
{
	unsigned char nAttrType;						//类型
	unsigned char nReserved;						//保留
	unsigned short nLength;							//属性长度
}ATTRIB_HEAD;
#pragma  pack()

#pragma pack(4)
typedef struct tagCptypeVideoData
{
	unsigned char ucVideoType;						//视频数据的编码格式：0-MPEG1 1-MPEG2 4-MPEG 8-MJPEG 16-H264
	unsigned char ucFrameType;						//帧类型。0, I 帧 1, P 帧 2, B帧 
	unsigned char ucAckFlag;						//Ack 回包标志。当这个标志为非0 时，要求客户端给服务器返回一个ack。RPU 一般在一帧视频的最后一个包置Ack 标志。
	unsigned char ucReserved;						//保留字节
	unsigned short ucPacketNum;						//一个完整的数据帧需要通过多个UDP 报文进行传输。Packet number 指传输当前一帧数据的UDP 报文的个数。
	unsigned short ucSeqNum;						//此包在一帧中的编号，从0 开始编号，0，1，2，3，…
	unsigned char ucFramePerSec;					//由RPU 产生的帧速率。每秒钟的产生的帧数目。
	unsigned char ucIFreq;							//I帧出现的频率。每“I Freq”个帧中包含一个I 帧，其余的都是P 帧。
	unsigned char ucReserved_2[2];					//保留字段,当前值为0                 
	unsigned char ucVideoId;						//视频通道,从1开始
	unsigned char ucVideoFlag;						//视频主辅码流类型
//	unsigned char ucDataBegin1;						//如果一帧数据压缩后小于64K Byte，那么该字段为0，如果一帧数据压缩后超过64K Byte，根据数据包长度确定该值.
	//unsigned short usDataBegin2;					//指数据在当前的视频帧中开始的位置。从帧头开始计算，以字节为单位。数据在当前视频帧的开始位置：Data Begin (1)*64K +Data Begin (2).
	unsigned short usDataLength;					//在此包中视频数据的长度。此字段和Data Begin 共同决定此包中的数据是一帧完整视频的哪一部分数据。为了防止路经的路由器或交换机对VSP 报文进行分片。DataLength 不得大于1400byte。
	unsigned int usDataBegin;						//指数据在当前的视频帧中开始的位置。从帧头开始计算，以字节为单位。数据在当前视频帧的开始位置：Data Begin (1)*64K +Data Begin (2).
	unsigned int uiFrameID;							//帧的编号, 从0 开始编号，RPU 每新产生一帧，帧ID 加1。
	unsigned int uiFrameLength;						//帧的总长度，以字节为单位。
	unsigned int uiTimeStamp;						//自1970 年1 月1 日00:00:00（coordinated universal time）以来的秒数。
	unsigned int uiTimeStampEx;						//时间的微秒部份。
//	char *pszVideoData;								//这里是真正的数据部分。
} CPTYPE_VIDEODATA;

//音频数据信息的结构体定义：35
typedef struct tagCptypeAudioData
{
	unsigned char ucAudioType;						//音频编码格式。当前只支持一个值,1:G.711
	unsigned char ucFramePerSec;					//RPU 每秒钟产生的帧数目。
	unsigned char ucAckFlag;						//0-不要求回Ack 包。1-要求回Ack 包。RPU 一般在一帧视频的最后一个包置Ack 标志，此时，Client 需要根据此标志，决定是否回Ack 包。
	unsigned char ucPacketNum;						//一个完整的数据帧需要通过多个UDP 报文进行传输。Packet number 指传输当前一帧数据的UDP 报文的个数。
	unsigned char ucSeqNum;							//指此包在一帧中的编号，从0 开始编号，0，1，2，3，…
	unsigned char ucAdudioId;						//音频通道ID,一般只有一个,从1开始
	unsigned char szReserve[2];						//用来将来扩展字段用。
	unsigned short usDataBegin;						//指数据在当前的音频帧中开始的位置。从帧头开始计算，以字节为单位。
	unsigned short usDataLength;					//在此包中音频数据的长度。此字段和Data Begin 共同决定此包中的数据是一帧完整视频的哪一部分数据。为了防止路经的路由器或交换机对VSP 报文进行分片。Data Length 不得大于1400byte。
	unsigned int uiFrameID;							//帧的编号, 从0 开始编号，RPU 每新产生一帧，帧ID 加1。
	unsigned int uiFrameLength;						//帧的总长度,以字节为单位
	unsigned int uiTimeStamp;						//自1970 年1 月1 日00:00:00（coordinated universal time）以来的秒数
	unsigned int uiTimeStampEx;						//时间的微秒部份。
//	char *pszAudioData;								//这里是真正音频数据开始的部分。
} CPTYPE_AUDIODATA;

typedef struct tagCptypeDataAck
{
	unsigned char ucAckType;						//Ack 类型：1-音频,表示对音频数据回ACK 2-视频,表示对视频数据回ACK
	unsigned char ucReserved;						//保留用于将来扩展字段用
	unsigned char ucRecvPackets;					//收到的包的个数。指的是收到的一帧中的包的个数。
	unsigned char ucDropPackets;					//丢弃的包的个数。是因为网络的原因造成包在传输过程中被丢弃，Client 没有收到
	unsigned char ucVAID;
	unsigned char ucVAFlag;
	unsigned char ucReserved_2[2];
	unsigned int uiFrameID;							//帧的ID。刚刚收到的帧的ID。
	//unsigned char ucDropPktID[];					//丢弃包的ID。这里丢弃包的ID 是对应与前面的SequenceNum 是一一对应的， 如果发现哪些Sequence Num 的包没有被收到，就需向RPU 反馈，以便RPU 调整发包的码流
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

//��׼�������ͷ�ļ�
#include <stdio.h>
#include <stdint.h>
//�ļ���������ͷ�ļ�
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/file.h>

#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include 		"DataFileManager.h"

static int create_multi_dir(const char *path);

/****************************
���ڣ�2017��11��7��   �ɼ������ļ�����취��
1.�����ļ����·�����ļ���������/��/��_SampleRecord.dat
2.�����ļ��洢ʱ���:00:00~24:00,
3.�����ļ����ǹ���:
		1�긲��,�����ڴ洢һ������ݣ����ս��������ļ�����;
		10�긲�ǣ����ݴ洢��10�������������ո��ǣ�
4.��ʷ���ݲ�������,�����Զ�������һ��������ǰ�£���ǰʱ��ڵ㡣
5.���ݼ�¼�С�����RecordReportFlah ��־�������ϴ����޸Ĵ˱�ʶ��ע��
6.��������ѯ��ʷ���ݣ����ݲ�ѯ���ڼ���ʱ��ε��������ݣ�����������Ƿ��Ѿ��ϱ���ʶ��
****************************/

/*********************************************************************************************************
** ��������: GetImageFile
** ��������: read file context
** ��ڲ���: char	*FilePath
**          uint32_t fileline
** ���ڲ����� uint8_t **r_data, int *r_len
** ����ֵ  : �ɹ� true  ʧ��  false
** ˵    ��:

**********************************************************************************************************/


uint8_t GetImageFile(char	*FilePathName,uint32_t fileline,uint8_t **r_data, int *r_len)
{
    FILE *fd;
	struct stat buf; 
    ssize_t     length_r;
    uint16_t    readLen;
    int32_t     st;
    char    type[80];
    char 	mStr[FileTransmitSize];
    *r_len = 10;  //Error raise
    //check forld,none create forlder
    create_multi_dir(FilePathName);

    //fd = open(FilePathName,O_RDONLY,0777);
    if( (fd = open(FilePathName,O_RDONLY,0777)) <= 0){
       fprintf(stderr, "open %s failed,%s%d \n",FilePathName,__FILE__,__LINE__);
       return false;
    }
    length_r = lseek (fd,fileline*sizeof(mStr),SEEK_CUR);
    readLen = sizeof(mStr);
    readLen = read(fd,&mStr,readLen);
    if(readLen <= 0){
        fprintf(stderr, "read err len:%ld,%s line%d \n",length_r,__FILE__,__LINE__);
        close(fd);
        return false;
    }

    length_r += readLen;
//    st = fstat(fd, &buf);
//    if( st <= 0)
//	{
//        fprintf(stderr, "fstat err%s line%d \n",__FILE__,__LINE__);
//        close(fd);
//        return false;
//	}
//    if(buf.st_size == length_r){
//        *r_len = readLen;
//        memcpy(r_data,mStr,*r_len);
//        close(fd);
//        return 2;
//    }
    *r_len = readLen;
    memcpy(r_data,mStr,*r_len);
    close(fd);
    return true;
}


/*********************************************************************************************************
** ��������: GetDeviceSampleInfor
** ��������: write sample information to flash
** ��ڲ���: CommunicationState	*pCommState   - 	CommunicationState
**           DeviceRunInfor 		*pInfor				-   DeviceRunInfor
** ���ڲ�����*pCommState	 --		ָ��ָ��Ľṹ�����ݱ�ˢ�� ,*pInfor memset(,0,)
** ����ֵ  : �ɹ� true  ʧ��  false
** ˵�� :
**********************************************************************************************************/


struct DevRunInfor  SampleRecord_Array[SampleRecordMaxLine];

uint8_t GetDeviceSampleInfor(CommunicationState	*pCommState,DeviceRunInfor *pInfor)
{
    FILE * fd;
    ssize_t length_r;
	ssize_t offset = pCommState->pGWIptr->StoreInforRDNumber;
    char *Path ;
    strncpy(Path,pCommState->sys_time,sizeof(pCommState->sys_time));

    char *FilePath = "../SampleRecord.dat";
    char mStr[sizeof(struct DevRunInfor)];
    struct DevRunInfor *PSampleRecord;
    PSampleRecord = (struct DevRunInfor *)mStr;
	if(offset > SampleRecordMaxLine)
		return false;
    if((fd = open(FilePath,O_RDWR|O_CREAT,0777)) < 0){
		fprintf(stderr, "open %s failed,%s%d \n",FilePath,__FILE__,__LINE__);
		return false;
    }
    length_r = lseek (fd,offset*sizeof(struct DevRunInfor),SEEK_CUR);

    if((length_r = read(fd,(void*)mStr,sizeof(struct DevRunInfor))) < 0){
        fprintf(stderr, "read failed,%s%d \n",__FILE__,__LINE__);
		close(fd);
		return false;
    }
    if(length_r != sizeof(struct DevRunInfor)){
		fprintf(stderr, "read length none enough,%s%d \n",__FILE__,__LINE__);
		close(fd);
		return false;
    }
	//printf("ManufacturerID is %d,DeviceId is %d \n",PSampleRecord->ManufacturerID,PSampleRecord->DeviceId );
    close(fd);
    memcpy((void*)pInfor,mStr,sizeof(struct DevRunInfor));
    return true;
}


/*********************************************************************************************************
** ��������: SetDeviceSampleInfor
** ��������: write sample information to flash
** ��ڲ���: CommunicationState	*pCommState   - 	CommunicationState
**           DeviceRunInfor 		*pInfor				-   DeviceRunInfor
** ���ڲ�����*pCommState	 --		ָ��ָ��Ľṹ�����ݱ�ˢ�� ,*pInfor memset(,0,)
** ����ֵ  : �ɹ� true  ʧ��  false
** ˵�� :
 
**********************************************************************************************************/

uint8_t SetDeviceSampleInfor(CommunicationState	*pCommState,DeviceRunInfor *pInfor)
{
	FILE * fd;
    ssize_t length_w,offset=1;
    char *FilePath ;
	
    int ret;
    DIR *dir;
    struct dirent *catlog;
    FilePath = "./2016/10/";
//check forld,none create forlder
    create_multi_dir(FilePath);

//ʹ��opendir������Ŀ¼
    dir = opendir(FilePath);
    if(dir==NULL){
        printf("opendir %s failed!\n",FilePath);
    }
    printf("opendir %s suceces��\n",FilePath);
//ʹ��readdir��ȡĿ¼argv[1]
    catlog = readdir(dir);
    if(catlog == NULL){
        printf("readdir %s failed!\n",FilePath);
    }
    printf("%s d_ino is %ld\n ",FilePath,catlog->d_ino);
//ʹ��closedir�����ر�Ŀ¼
    closedir(dir);

    char mStr[sizeof(struct DevRunInfor)];
    struct DevRunInfor *PSampleRecord;
    PSampleRecord = (struct DevRunInfor *)mStr;
    *FilePath = "../SampleRecord.dat";
	if(offset > SampleRecordMaxLine)
		return false;
	memcpy(PSampleRecord,(const void*)pInfor,sizeof(mStr));
  if((fd = open(FilePath,O_RDWR|O_CREAT|O_APPEND,0777))<0){
       fprintf(stderr, "open %s failed,%s%d \n",FilePath,__FILE__,__LINE__);
	   return false;
    }

//	length_r = lseek (fd,offset*sizeof(struct SampleRecord),SEEK_CUR);

    length_w = write(fd,mStr,sizeof(struct DevRunInfor));
    if((length_w <= 0)||(length_w != sizeof(mStr))){
        fprintf(stderr, "write none length,%s%d \n",__FILE__,__LINE__);
		return false;
    }
    else{
        printf("mStr Write OK!\n");
    }
    close(fd);
    return true;
}


/*********************************************************************************************************
** ��������: GetDeviceConfigInfor
** ��������: ��ȡ�豸eeprom ��Ϣ����
** ��ڲ���: void  *parg        - 	DeviceConfigInfor*	
**           uint16 CurNumber - �豸��� 

** ���ڲ�����parg --��ȡ�豸��Ϣ����
** ����ֵ  : ��ȡ�ɹ� true  ʧ��  false
** ע��    ��
**********************************************************************************************************/

uint8_t  GetDeviceConfigInfor(void *parg,uint16_t CurNumber)
{
		uint8_t	st;
		uint16_t val,NumberLine;
		uint32_t	iEmemAddr;
		DeviceConfigInfor*	pDevInfor;
		pDevInfor 	=	(DeviceConfigInfor *)parg;
		if(CurNumber >0){
			NumberLine	= CurNumber - 1;   //eeprom address from 0 to DeviceMaxTotalCnt-1
		}
		else
			return false;
		iEmemAddr	=	NumberLine*DeviceStructSize+dev1ManufacturerID;

		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;
		pDevInfor->ManufacturerID = val;
		pDevInfor->ManufacturerID <<= 8; 
		st=	GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;
		pDevInfor->ManufacturerID |= val;

		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
				return	false;	
		pDevInfor->GroupId = val;
		
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->DeviceType = (uint8_t)val;
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->DeviceId = (uint8_t)val;	
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->CollectType = val;	
		
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->CollectPeriod = val;	
		
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->InforCode = val;	
		pDevInfor->InforCode <<= 16; 
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->InforCode |= val;	
		
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->ItemId = val;	
		
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->Unit = val;		
		
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->RegisterState = val;
		st=GetRegValue(NULL,&val,iEmemAddr++);
		if(st == false)
			return	false;	
		pDevInfor->SampleState = val;		
		return	true;
}







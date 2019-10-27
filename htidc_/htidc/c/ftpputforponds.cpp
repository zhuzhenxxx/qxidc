#include "_public.h"
#include "ftp.h"

void CallQuit(int sig);

FILE           *timefp,*listfp;
Cftp           ftp;
CLogFile       logfile;
CProgramActive ProgramActive;

char strremoteip[21];        // Զ�̷�������IP
UINT uport;                  // Զ�̷�����FTP�Ķ˿�
char strmode[11];            // ����ģʽ��port��pasv
char strusername[31];        // Զ�̷�����FTP���û���
char strpassword[31];        // Զ�̷�����FTP������
char strlocalpath[201];      // �����ļ���ŵ�Ŀ¼
char strlocalpathbak[201];   // �����ļ����ͳɹ��󣬴�ŵı���Ŀ¼�������Ŀ¼Ϊ�գ��ļ����ͳɹ���ֱ��ɾ��
char strlistfilename[301];   // ����Ѵ����ļ��б���xml�ļ�
char strtimefilename[301];   // ʱ���ǩ�ļ�
char strremotepath[201];     // Զ�̷���������ļ�������Ŀ¼
char strmatchname[4096];     // �������ļ�ƥ����ļ���
UINT utimeout;               // FTP�����ļ��ĳ�ʱʱ��
int  itimetvl;
char strtrlog[11];           // �Ƿ��л���־
char strdeleteremote[21];
char strFullFileName[501];   // �ļ�ȫ��������·��

 
// ������Է������������ӣ�����¼
BOOL ftplogin();

// ���ļ����͵��Է�������Ŀ¼
BOOL ftpputforponds();

// ���Ѿ����͹����ļ���Ϣд��List�ļ�
BOOL  WriteToXML();

// �ļ��б����ݽṹ
struct st_filelist
{
  char localfilename[301];
  char modtime[21];
  int  filesize;
  int  ftpsts;   // 1-δ����,2-�����͡�
};

// �ļ���Ϣ����ʱ���ݽṹ�����κκ����ж�����ֱ����
struct st_filelist stfilelist;

// ���η������ļ��嵥������
vector<struct st_filelist> v_newfilelist;

// �ϴη������ļ��嵥������
vector<struct st_filelist> v_oldfilelist;

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpputforponds logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/ftpputforponds /tmp/htidc/log/ftpputforponds_192.168.201.14_oracle_gzradpic.log \"<remoteip>192.168.201.14</remoteip><port>2138</port><mode>pasv</mode><username>data</username><password>Data#1620</password><localpath>/mnt/data9733/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</localpath><localpathbak>/mnt/data9733/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</localpathbak><remotepath>/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</remotepath><timefilename>/mnt/data9733/OpticalFlowDataUrs/NewTREC/trecPonds.over</timefilename><matchname>TREC_Wind_{YYYY}��{MM}��{DD}��{HH}ʱ{MI}��00��_.png,Meta_TREC_GD_{YYYY}��{MM}��{DD}��{HH}ʱ{MI}��00��_138����.png,Meta_TREC_GD_{YYYY}��{MM}��{DD}��{HH}ʱ{MI}��00��_144����.png</matchname><listfilename>/tmp/htidc/list/ftpputforponds_192.168.201.14_gzradpic.xml</listfilename><timeout>180</timeout><trlog>TRUE</trlog><deleteremote>TRUE</deleteremote>\"\n\n");

    printf("��������ר������ponds�����ϴ�����������ponds�ǽṹ���ļ����͵�Զ�̵�FTP��������\n");
    printf("logfilename�Ǳ��������е���־�ļ���һ�����/tmp/htidc/logĿ¼�£�"\
           "����ftpputforponds_Զ�̵�ַ_ftp�û���.log������\n");
    printf("xmlbufferΪ�ļ�����Ĳ��������£�\n");
    printf("<remoteip>192.168.201.14</remoteip> Զ�̷�������IP��\n");
    printf("<port>21</port> Զ�̷�����FTP�Ķ˿ڡ�\n");
    printf("<mode>pasv</mode> ����ģʽ��pasv��port����ѡ�ֶΣ�ȱʡ����pasvģʽ��portģʽһ�㲻�á�\n");
    printf("<username>oracle</username> Զ�̷�����FTP���û�����\n");
    printf("<password>oracle1234HOTA</password> Զ�̷�����FTP�����롣\n");
    printf("<localpath>/tmp/htidc/ftpget/gzrad</localpath> �����ļ���ŵ�Ŀ¼��\n");
    printf("<localpathbak>/tmp/htidc/ftpget/gzradbak</localpathbak> �����ļ����ͳɹ��󣬴�ŵı���Ŀ¼��"\
           "�����Ŀ¼Ϊ�գ��ļ����ͳɹ���ֱ��ɾ�������localpath����localpathbak��"\
           "�ļ����ͳɹ��󽫱���Դ�ļ����䡣��������£������ļ������Ӧ��ɾ���򱸷ݣ�"\
           "��ɾ��������ֻ�����ڴ�����־�ļ��������\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> Զ�̷���������ļ���Ŀ¼��\n");
    printf("timefilename>/mnt/data9733/DualPolarimetricRadar/Radar/time/dbz_time.txt</timefilename> ʱ���ǩ�ļ���"\
           "ÿ�����ݶ���һ��ʱ���ǩ�ļ�����¼���ȶ�ȡʱ���ǩ���ݣ��Ϳ���֪�����µ��ļ������ĸ�Ŀ¼��ʲôʱ��ġ�\n");
    printf("<matchname>WX_AWS_dbz_ShenZhen_{yyyy}{mm}{dd}{hh24}{mi}00.png,WX_Mate_dbz_GD_{yyyy}{mm}{dd}{hh24}{mi}00.png</matchname> �������ļ�ƥ����ļ�����ֱ�Ӿ�׼ƥ�䣬���ִ�Сд��"\
           "Ŀǰ���Դ�������ʱ�������{YYYY}��4λ���꣩��{YYY}������λ���꣩��"\
           "{YY}������λ���꣩��{MM}�����£���{DD}�����գ���{HH}��ʱʱ����{MI}���ַ֣���{SS}�����룩��\n");
    printf("<listfilename>/tmp/htidc/list/ftpputforponds_192.168.201.14_gzradpic.xml</listfilename> ��"\
           "���������ļ��б���xml�ļ���һ�����/tmp/htidc/listĿ¼�£�"\
           "����ftpgetfiles_Զ�̵�ַ_ftp�û���_�ļ�����.xml��������ʽ��\n");
    printf("<timeout>1800</timeout> FTP�����ļ��ĳ�ʱʱ�䣬��λ���룬ע�⣬����ȷ������ĳ"\
           "���ļ���ʱ��С��timeout��ȡֵ���������ɴ���ʧ�ܵ������\n");
    printf("<trlog>TRUE</trlog> ����־�ļ�����100Mʱ���Ƿ��л���־�ļ���TRUE-�л���FALSE-���л���\n");
    printf("<deleteremote>TRUE</deleteremote> ���Զ��Ŀ¼���ڸ��ļ����Ƿ�ɾ��Զ��Ŀ¼����ʱ�ļ�����ʽ�ļ�\n");
    printf("<timetvl>-8</timetvl> Ϊʱ�������ƫ��ʱ�䣬��ʱ�����á�\n");
    printf("���ϵĲ���ֻ��mode��timetvl��localpathbak��trlogΪ��ѡ�����������Ķ����\n\n");

    printf("port ģʽ�ǽ����ӷ������߶˿������ͻ���20�˿��������ӡ�\n");
    printf("pasv ģʽ�ǽ����ͻ��߶˿��������������ص����ݶ˿ڵ��������ӡ�\n\n");

    printf("port����������ʽ�����ӹ����ǣ��ͻ������������FTP�˿ڣ�Ĭ����21��������������"\
           "�������������ӣ�����һ��������·��\n");
    printf("����Ҫ��������ʱ����������20�˿���ͻ��˵Ŀ��ж˿ڷ����������󣬽���һ��������·���������ݡ�\n\n");

    printf("pasv����������ʽ�����ӹ����ǣ��ͻ������������FTP�˿ڣ�Ĭ����21��������������"\
           "�������������ӣ�����һ��������·��\n");
    printf("����Ҫ��������ʱ���ͻ�����������Ŀ��ж˿ڷ����������󣬽���һ��������·���������ݡ�\n\n");

    return -1;
  }

  // �ر�ȫ�����źź��������
  // �����ź�,��shell״̬�¿��� "kill + ���̺�" ������ֹЩ����
  // ���벻Ҫ�� "kill -9 +���̺�" ǿ����ֹ
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001]; 

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // �ж��Ƿ��л���־
  BOOL btrlog=TRUE;

  if (strcmp(strtrlog,"FALSE")==0) btrlog=FALSE;

  // ����־
  if (logfile.Open(argv[1],"a+",btrlog) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //�򿪸澯
  logfile.SetAlarmOpt("ftpputforponds");

  memset(strremoteip,0,sizeof(strremoteip));          // Զ�̷�������IP
  memset(strmode,0,sizeof(strmode));                  // ����ģʽ��port��pasv
  memset(strusername,0,sizeof(strusername));          // Զ�̷�����FTP���û���
  memset(strpassword,0,sizeof(strpassword));          // Զ�̷�����FTP������
  memset(strlocalpath,0,sizeof(strlocalpath));        // �����ļ���ŵ�Ŀ¼
  memset(strlocalpathbak,0,sizeof(strlocalpathbak));  // ������Ŀ¼Ϊ�գ��ļ����ͳɹ���ֱ��ɾ��
  memset(strtimefilename,0,sizeof(strtimefilename));  // ʱ���ǩ�ļ�
  memset(strremotepath,0,sizeof(strremotepath));      // Զ�̷���������ļ�������Ŀ¼
  memset(strmatchname,0,sizeof(strmatchname));        // �������ļ�ƥ����ļ���
  memset(strtrlog,0,sizeof(strtrlog));        
  memset(strdeleteremote,0,sizeof(strdeleteremote));
  memset(strlistfilename,0,sizeof(strlistfilename));
  uport=21;                                           // Զ�̷�����FTP�Ķ˿�
  utimeout=0;                                         // FTP�����ļ��ĳ�ʱʱ��
  itimetvl=0;

  GetXMLBuffer(strXmlBuffer,"timetvl",&itimetvl);
  GetXMLBuffer(strXmlBuffer,"password",strpassword,30);
  GetXMLBuffer(strXmlBuffer,"remoteip",strremoteip,20);
  GetXMLBuffer(strXmlBuffer,"port",&uport);
  GetXMLBuffer(strXmlBuffer,"mode",strmode,4);
  GetXMLBuffer(strXmlBuffer,"username",strusername,30);
  GetXMLBuffer(strXmlBuffer,"timeout",&utimeout);
  GetXMLBuffer(strXmlBuffer,"trlog",strtrlog,8);
  GetXMLBuffer(strXmlBuffer,"deleteremote",strdeleteremote,8);
  GetXMLBuffer(strXmlBuffer,"listfilename",strlistfilename,300);
  GetXMLBuffer(strXmlBuffer,"timefilename",strtimefilename,300);

  if (strcmp(strmode,"port") != 0) strncpy(strmode,"pasv",4);

  if (strlen(strremoteip) == 0) { logfile.Write("remoteip is null.\n"); return -1; }
  if (strlen(strusername) == 0) { logfile.Write("username is null.\n"); return -1; }
  if (strlen(strpassword) == 0) { logfile.Write("password is null.\n"); return -1; }
  if (uport == 0)               { logfile.Write("port is null.\n"); return -1;     }
  if (utimeout == 0)            { logfile.Write("timeout is null.\n"); return -1;  }

  // ע�⣬����ʱ��utimeout��
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",utimeout);

  ProgramActive.WriteToFile();

  char strLine[501];

  // ��ȡʱ���ǩ�ļ�����,������xmlbuffer�е�ʱ�������
  if ( (timefp=FOPEN(strtimefilename,"r")) != NULL)
  {
    memset(strLine,0,sizeof(strLine));

    // ���ļ��л�ȡһ��
    FGETS(strLine,100,timefp);
    fclose(timefp);

    // �չ�14λ��������ʱ����
    strncat(strLine,"000000",14-strlen(strLine));

    char YYYY[5],YYY[4],YY[3],MM[3],DD[3],HH[3],MI[3],SS[3];

    memset(YYYY,0,sizeof(YYYY));
    memset(YYY,0,sizeof(YYY));
    memset(YY,0,sizeof(YY));
    memset(MM,0,sizeof(MM));
    memset(DD,0,sizeof(DD));
    memset(HH,0,sizeof(HH));
    memset(MI,0,sizeof(MI));
    memset(SS,0,sizeof(SS));

    strncpy(YYYY,strLine,4);
    strncpy(YYY,strLine+1,3);
    strncpy(YY,strLine+2,2);
    strncpy(MM,strLine+4,2);
    strncpy(DD,strLine+6,2);
    strncpy(HH,strLine+8,2);
    strncpy(MI,strLine+10,2);
    strncpy(SS,strLine+12,2);

    UpdateStr(strXmlBuffer,"{YYYY}",YYYY);
    UpdateStr(strXmlBuffer,"{YYY}",YYY);
    UpdateStr(strXmlBuffer,"{YY}",YY);
    UpdateStr(strXmlBuffer,"{MM}",MM);
    UpdateStr(strXmlBuffer,"{DD}",DD);
    UpdateStr(strXmlBuffer,"{HH}",HH);
    UpdateStr(strXmlBuffer,"{MI}",MI);
    UpdateStr(strXmlBuffer,"{SS}",SS);

    UpdateStr(strXmlBuffer,"{yyyy}",YYYY);
    UpdateStr(strXmlBuffer,"{yyy}",YYY);
    UpdateStr(strXmlBuffer,"{yy}",YY);
    UpdateStr(strXmlBuffer,"{mm}",MM);
    UpdateStr(strXmlBuffer,"{dd}",DD);
    UpdateStr(strXmlBuffer,"{hh}",HH);
    UpdateStr(strXmlBuffer,"{mi}",MI);
    UpdateStr(strXmlBuffer,"{ss}",SS);
  }

  // �����ڴ���xmlbuffer�е�ʱ�����֮���ȡֵ
  GetXMLBuffer(strXmlBuffer,"localpath",strlocalpath,200);
  GetXMLBuffer(strXmlBuffer,"localpathbak",strlocalpathbak,200);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,200);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,4000);

  if (strlen(strlocalpath) == 0)  { logfile.Write("localpath is null.\n"); return -1; }
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1;}
  if (strlen(strmatchname) == 0)  { logfile.Write("matchname is null.\n"); return -1; }

  v_oldfilelist.clear();

  // �ϴη������ļ��嵥���ص�v_oldfilelist
  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // ���ļ��л�ȡһ��
      if (FGETS(strLine,500,listfp,"<endl/>") == FALSE) break;

      memset(&stfilelist,0,sizeof(stfilelist));

      GetXMLBuffer(strLine,"filename", stfilelist.localfilename,300);
      GetXMLBuffer(strLine,"modtime",  stfilelist.modtime,14);
      GetXMLBuffer(strLine,"filesize",&stfilelist.filesize);

      v_oldfilelist.push_back(stfilelist);
    }

    fclose(listfp);
  }

  v_newfilelist.clear();

  CCmdStr CmdStr;
  CmdStr.SplitToCmd(strmatchname,",");

  for (UINT ii=0;ii<CmdStr.CmdCount();ii++)
  { 
    memset(strFullFileName,0,sizeof(strFullFileName));
  
    snprintf(strFullFileName,500,"%s/%s",strlocalpath,CmdStr.m_vCmdStr[ii].c_str());

    // ����Ƿ��������ļ������û�У��򷵻ء�
    if (access(strFullFileName,F_OK) != 0) continue;

    memset(&stfilelist,0,sizeof(stfilelist));

    struct stat st_filestat;      // �ļ���Ϣ�ṹ��
    stat(strFullFileName,&st_filestat); // ��ȡ�ļ���Ϣ

    stfilelist.filesize = st_filestat.st_size; // �ļ���С

    struct tm nowtimer;

    // �ļ����һ�α��޸ĵ�ʱ��
    nowtimer = *localtime(&st_filestat.st_mtime); nowtimer.tm_mon++;
    snprintf(stfilelist.modtime,20,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    strncpy(stfilelist.localfilename,CmdStr.m_vCmdStr[ii].c_str(),300); // �ļ���
    stfilelist.ftpsts = 1; // Ĭ���ϴ����ϴ�-1�����ϴ�-2��

    v_newfilelist.push_back(stfilelist);

  }

  // �ѱ����ļ����ϴ��ļ����Աȣ��ó���Ҫ�ϴ����ļ���
  for (UINT ii=0;ii<v_newfilelist.size();ii++)
  {
    if (v_newfilelist.size() == 0) continue;

    // ��Զ��Ŀ¼���ļ�������С�����ںͱ��ص��嵥�ļ��Ƚ�һ�£����ȫ����ͬ���Ͱ�ftpsts����Ϊ2�����ٲɼ�
    for (UINT jj=0;jj<v_oldfilelist.size();jj++)
    {
      if (strcmp(v_newfilelist[ii].localfilename,v_oldfilelist[jj].localfilename) == 0)
      {
        if ( (strcmp(v_newfilelist[ii].modtime,v_oldfilelist[jj].modtime) == 0) &&
             (v_newfilelist[ii].filesize==v_oldfilelist[jj].filesize) )
        {
          v_newfilelist[ii].ftpsts=2;
        }

        break;
      }
    }
  }

  // �ϴ��ļ�
  ftpputforponds();

  // ���Ѿ����͹����ļ���Ϣд��List�ļ�
  WriteToXML();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  // ���Ѿ����͹����ļ���Ϣд��List�ļ�
  WriteToXML();

  logfile.Write("ftpputforponds exit.\n");

  exit(0);
}


// ���Ѿ����͹����ļ���Ϣд��List�ļ�
BOOL WriteToXML()
{
  if ( (listfp=FOPEN(strlistfilename,"w")) == NULL)
  {
    logfile.Write("FOPEN %s failed.\n",strlistfilename); return FALSE;
  }

  fprintf(listfp,"<data>\n");

  for (UINT nn=0;nn<v_newfilelist.size();nn++)
  {
    if (v_newfilelist[nn].ftpsts==2)
    {
      fprintf(listfp,"<filename>%s</filename><modtime>%s</modtime><filesize>%d</filesize><endl/>\n",v_newfilelist[nn].localfilename,v_newfilelist[nn].modtime,v_newfilelist[nn].filesize);
    }
  }

  fprintf(listfp,"</data>\n");
  
  return TRUE;
}

// ������Է������������ӣ�����¼
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // ���ô���ģʽ
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP���Ӻ͵�¼�ĳ�ʱʱ����Ϊ80�͹��ˡ�
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) failed.\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // ����ĳ�ʱʱ������Ϊutimeout�롣
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",utimeout);

  // �ڶԷ��������ϴ���Ŀ¼
  ftp.mkdir(strremotepath);

  return TRUE;
}

// ���ļ����͵��Է�������Ŀ¼
BOOL ftpputforponds()
{
  char strFullLocalFileName[301];
  char strFullLocalFileNameBak[301];
  char strFullRemoteFileName[301];
  char strFullRemoteFileNameTmp[301];
  BOOL bConnected=FALSE;

  for (UINT kk=0;kk<v_newfilelist.size();kk++)
  {
    // ���ļ��ɼ������ط�����Ŀ¼
    if (v_newfilelist[kk].ftpsts==1)
    {
      memcpy(&stfilelist,&v_newfilelist[kk],sizeof(stfilelist));

      // ���û�����϶Է���FTP��������������
      if (bConnected==FALSE)
      {
        // ���ӷ�����
        if (ftplogin() == FALSE) return FALSE;

        ProgramActive.WriteToFile();

        bConnected=TRUE;
      }

      // ����ļ���СΪ0����������
      if (stfilelist.filesize == 0) continue;

      memset(strFullLocalFileName,0,sizeof(strFullLocalFileName));
      memset(strFullLocalFileNameBak,0,sizeof(strFullLocalFileNameBak));
      memset(strFullRemoteFileName,0,sizeof(strFullRemoteFileName));
      memset(strFullRemoteFileNameTmp,0,sizeof(strFullRemoteFileNameTmp));

      snprintf(strFullLocalFileName,300,"%s/%s",strlocalpath,stfilelist.localfilename);
      snprintf(strFullLocalFileNameBak,300,"%s/%s",strlocalpathbak,stfilelist.localfilename);
      snprintf(strFullRemoteFileName,300,"%s/%s",strremotepath,stfilelist.localfilename);
      snprintf(strFullRemoteFileNameTmp,300,"%s/%s.tmp",strremotepath,stfilelist.localfilename);

      // ɾ��Զ��Ŀ¼����ʱ�ļ�����ʽ�ļ�����Ϊ����Է�Ŀ¼�Ѵ��ڸ��ļ������ܻᵼ���ļ����䲻�ɹ������
      // ���ǣ��µ���ⲻ֪�����᲻������
      if (strcmp(strdeleteremote,"TRUE")==0) { ftp.ftpdelete(strFullRemoteFileNameTmp); ftp.ftpdelete(strFullRemoteFileName); }

      CTimer Timer;

      Timer.Beginning();

      logfile.Write("put %s(size=%ld,mtime=%s)...",strFullLocalFileName,stfilelist.filesize,stfilelist.modtime);

      BOOL bIsLogFile=MatchFileName(stfilelist.localfilename,"*.LOG");

      // �����ļ�
      if (ftp.put(strFullLocalFileName,strFullRemoteFileName,bIsLogFile) == FALSE)
      {
        logfile.WriteEx("failed.\n%s",ftp.response()); continue;
      }
      else
      {
        // ���Ϊ���ϴ�
        v_newfilelist[kk].ftpsts=2;

        logfile.WriteEx("ok(time=%d).\n",Timer.Elapsed());
      }

      if (strlen(strlocalpathbak) == 0)
      {
        // ɾ������Ŀ¼�ĸ��ļ�
        if (REMOVE(strFullLocalFileName) == FALSE) 
        { 
          logfile.WriteEx("\nREMOVE(%s) failed.\n",strFullLocalFileName); continue; 
        }
      }
      else
      {
        // �ѱ���Ŀ¼�ĸ��ļ��Ƶ����ݵ�Ŀ¼ȥ�����localpath����localpathbak������Դ�ļ�����
        if (strcmp(strlocalpath,strlocalpathbak) != 0)
        {
          if (RENAME(strFullLocalFileName,strFullLocalFileNameBak) == FALSE) 
          { 
            logfile.WriteEx("\nRENAME(%s,%s) failed.\n",strFullLocalFileName,strFullLocalFileNameBak); 

            continue; 
          }
        }
      }
    }
  }

  ftp.logout();

  return TRUE;
}
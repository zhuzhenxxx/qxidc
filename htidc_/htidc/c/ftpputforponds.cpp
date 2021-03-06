#include "_public.h"
#include "ftp.h"

void CallQuit(int sig);

FILE           *timefp,*listfp;
Cftp           ftp;
CLogFile       logfile;
CProgramActive ProgramActive;

char strremoteip[21];        // 远程服务器的IP
UINT uport;                  // 远程服务器FTP的端口
char strmode[11];            // 传输模式，port和pasv
char strusername[31];        // 远程服务器FTP的用户名
char strpassword[31];        // 远程服务器FTP的密码
char strlocalpath[201];      // 本地文件存放的目录
char strlocalpathbak[201];   // 本地文件发送成功后，存放的备份目录，如果该目录为空，文件发送成功后直接删除
char strlistfilename[301];   // 存放已传输募斜淼膞ml文件
char strtimefilename[301];   // 时间标签文件
char strremotepath[201];     // 远程服务器存放文件的最终目录
char strmatchname[4096];     // 待发送文件匹配的文件名
UINT utimeout;               // FTP发送文件的超时时间
int  itimetvl;
char strtrlog[11];           // 是否切换日志
char strdeleteremote[21];
char strFullFileName[501];   // 文件全名，包括路径

 
// 建立与对方服务器的连接，并登录
BOOL ftplogin();

// 把文件发送到对方服务器目录
BOOL ftpputforponds();

// 把已经推送过的文件信息写入List文件
BOOL  WriteToXML();

// 文件列表数据结构
struct st_filelist
{
  char localfilename[301];
  char modtime[21];
  int  filesize;
  int  ftpsts;   // 1-未推送,2-已推送。
};

// 文件信息的临时数据结构，在任何函数中都可以直接用
struct st_filelist stfilelist;

// 本次服务器文件清单的容器
vector<struct st_filelist> v_newfilelist;

// 上次服务器文件清单的容器
vector<struct st_filelist> v_oldfilelist;

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/ftpputforponds logfilename xmlbuffer\n\n");

    printf("Sample:/htidc/htidc/bin/ftpputforponds /tmp/htidc/log/ftpputforponds_192.168.201.14_oracle_gzradpic.log \"<remoteip>192.168.201.14</remoteip><port>2138</port><mode>pasv</mode><username>data</username><password>Data#1620</password><localpath>/mnt/data9733/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</localpath><localpathbak>/mnt/data9733/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</localpathbak><remotepath>/OpticalFlowDataUrs/NewTREC/{YYYY}{MM}{DD}</remotepath><timefilename>/mnt/data9733/OpticalFlowDataUrs/NewTREC/trecPonds.over</timefilename><matchname>TREC_Wind_{YYYY}年{MM}月{DD}日{HH}时{MI}分00秒_.png,Meta_TREC_GD_{YYYY}年{MM}月{DD}日{HH}时{MI}分00秒_138分钟.png,Meta_TREC_GD_{YYYY}年{MM}月{DD}日{HH}时{MI}分00秒_144分钟.png</matchname><listfilename>/tmp/htidc/list/ftpputforponds_192.168.201.14_gzradpic.xml</listfilename><timeout>180</timeout><trlog>TRUE</trlog><deleteremote>TRUE</deleteremote>\"\n\n");

    printf("本程序是专门用于ponds数据上传，可增量把ponds非结构化文件发送到远程的FTP服务器。\n");
    printf("logfilename是本程序运行的日志文件，一般放在/tmp/htidc/log目录下，"\
           "采用ftpputforponds_远程地址_ftp用户名.log命名。\n");
    printf("xmlbuffer为文件传输的参数，如下：\n");
    printf("<remoteip>192.168.201.14</remoteip> 远程服务器的IP。\n");
    printf("<port>21</port> 远程服务器FTP的端口。\n");
    printf("<mode>pasv</mode> 传输模式，pasv和port，可选字段，缺省采用pasv模式，port模式一般不用。\n");
    printf("<username>oracle</username> 远程服务器FTP的用户名。\n");
    printf("<password>oracle1234HOTA</password> 远程服务器FTP的密码。\n");
    printf("<localpath>/tmp/htidc/ftpget/gzrad</localpath> 本地文件存放的目录。\n");
    printf("<localpathbak>/tmp/htidc/ftpget/gzradbak</localpathbak> 本地文件发送成功后，存放的备份目录，"\
           "如果该目录为空，文件发送成功后直接删除，如果localpath等于localpathbak，"\
           "文件发送成功后将保留源文件不变。正常情况下，本地文件传输后应该删除或备份，"\
           "不删除不备份只适用于传输日志文件的情况。\n");
    printf("<remotepath>/tmp/radpic/gzrad</remotepath> 远程服务器存放文件的目录。\n");
    printf("timefilename>/mnt/data9733/DualPolarimetricRadar/Radar/time/dbz_time.txt</timefilename> 时间标签文件，"\
           "每种数据都有一个时间标签文件做记录，先读取时间标签内容，就可以知道最新的文件是在哪个目录、什么时候的。\n");
    printf("<matchname>WX_AWS_dbz_ShenZhen_{yyyy}{mm}{dd}{hh24}{mi}00.png,WX_Mate_dbz_GD_{yyyy}{mm}{dd}{hh24}{mi}00.png</matchname> 待发送文件匹配的文件名，直接精准匹配，区分大小写，"\
           "目前可以处理以下时间变量：{YYYY}（4位的年）、{YYY}（后三位的年）、"\
           "{YY}（后两位的年）、{MM}（月月）、{DD}（日日）、{HH}（时时）、{MI}（分分）、{SS}（秒秒）。\n");
    printf("<listfilename>/tmp/htidc/list/ftpputforponds_192.168.201.14_gzradpic.xml</listfilename> 存"\
           "放已推送文件列表的xml文件，一般放在/tmp/htidc/list目录下，"\
           "采用ftpgetfiles_远程地址_ftp用户名_文件类型.xml的命名方式。\n");
    printf("<timeout>1800</timeout> FTP发送文件的超时时间，单位：秒，注意，必须确保传输某"\
           "个文件的时间小于timeout的取值，否则会造成传输失败的情况。\n");
    printf("<trlog>TRUE</trlog> 当日志文件大于100M时，是否切换日志文件，TRUE-切换；FALSE-不切换。\n");
    printf("<deleteremote>TRUE</deleteremote> 如果远程目录存在该文件，是否删除远程目录的临时文件和正式文件\n");
    printf("<timetvl>-8</timetvl> 为时间变量的偏移时间，暂时不启用。\n");
    printf("以上的参数只有mode、timetvl、localpathbak、trlog为可选参数，其它的都必填。\n\n");

    printf("port 模式是建立从服务器高端口连到客户端20端口数据连接。\n");
    printf("pasv 模式是建立客户高端口连到服务器返回的数据端口的数据连接。\n\n");

    printf("port（主动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，"\
           "服务器接受连接，建立一条命令链路。\n");
    printf("当需要传送数据时，服务器从20端口向客户端的空闲端口发送连接请求，建立一条数据链路来传送数据。\n\n");

    printf("pasv（被动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，"\
           "服务器接受连接，建立一条命令链路。\n");
    printf("当需要传送数据时，客户端向服务器的空闲端口发送连接请求，建立一条数据链路来传送数据。\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  char strXmlBuffer[4001]; 

  memset(strXmlBuffer,0,sizeof(strXmlBuffer));

  strncpy(strXmlBuffer,argv[2],4000);

  // 判断是否切换日志
  BOOL btrlog=TRUE;

  if (strcmp(strtrlog,"FALSE")==0) btrlog=FALSE;

  // 打开日志
  if (logfile.Open(argv[1],"a+",btrlog) == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",argv[1]); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("ftpputforponds");

  memset(strremoteip,0,sizeof(strremoteip));          // 远程服务器的IP
  memset(strmode,0,sizeof(strmode));                  // 传输模式，port和pasv
  memset(strusername,0,sizeof(strusername));          // 远程服务器FTP的用户名
  memset(strpassword,0,sizeof(strpassword));          // 远程服务器FTP的密码
  memset(strlocalpath,0,sizeof(strlocalpath));        // 本地文件存放的目录
  memset(strlocalpathbak,0,sizeof(strlocalpathbak));  // 绻媚柯嘉眨募⑺统晒笾苯由境�
  memset(strtimefilename,0,sizeof(strtimefilename));  // 时间标签文件
  memset(strremotepath,0,sizeof(strremotepath));      // 远程服务器存放文件的最终目录
  memset(strmatchname,0,sizeof(strmatchname));        // 待发送文件匹配的文件名
  memset(strtrlog,0,sizeof(strtrlog));        
  memset(strdeleteremote,0,sizeof(strdeleteremote));
  memset(strlistfilename,0,sizeof(strlistfilename));
  uport=21;                                           // 远程服务器FTP的端口
  utimeout=0;                                         // FTP发送文件的超时时间
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

  // 注意，程序超时是utimeout秒
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",utimeout);

  ProgramActive.WriteToFile();

  char strLine[501];

  // 读取时间标签文件内容,并处理xmlbuffer中的时间变量。
  if ( (timefp=FOPEN(strtimefilename,"r")) != NULL)
  {
    memset(strLine,0,sizeof(strLine));

    // 从文件中获取一行
    FGETS(strLine,100,timefp);
    fclose(timefp);

    // 凑够14位的年月日时分秒
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

  // 必须在处理xmlbuffer中的时间变量之后才取值
  GetXMLBuffer(strXmlBuffer,"localpath",strlocalpath,200);
  GetXMLBuffer(strXmlBuffer,"localpathbak",strlocalpathbak,200);
  GetXMLBuffer(strXmlBuffer,"remotepath",strremotepath,200);
  GetXMLBuffer(strXmlBuffer,"matchname",strmatchname,4000);

  if (strlen(strlocalpath) == 0)  { logfile.Write("localpath is null.\n"); return -1; }
  if (strlen(strremotepath) == 0) { logfile.Write("remotepath is null.\n"); return -1;}
  if (strlen(strmatchname) == 0)  { logfile.Write("matchname is null.\n"); return -1; }

  v_oldfilelist.clear();

  // 上次服务器文件清单加载到v_oldfilelist
  if ( (listfp=FOPEN(strlistfilename,"r")) != NULL)
  {
    while (TRUE)
    {
      memset(strLine,0,sizeof(strLine));

      // 从文件中获取一行
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

    // 检测是否存在这个文件，如果没有，则返回。
    if (access(strFullFileName,F_OK) != 0) continue;

    memset(&stfilelist,0,sizeof(stfilelist));

    struct stat st_filestat;      // 文件信息结构体
    stat(strFullFileName,&st_filestat); // 获取文件信息

    stfilelist.filesize = st_filestat.st_size; // 文件大小

    struct tm nowtimer;

    // 文件最后一次被修改的时间
    nowtimer = *localtime(&st_filestat.st_mtime); nowtimer.tm_mon++;
    snprintf(stfilelist.modtime,20,"%04u%02u%02u%02u%02u%02u",\
             nowtimer.tm_year+1900,nowtimer.tm_mon,nowtimer.tm_mday,\
             nowtimer.tm_hour,nowtimer.tm_min,nowtimer.tm_sec);

    strncpy(stfilelist.localfilename,CmdStr.m_vCmdStr[ii].c_str(),300); // 文件名
    stfilelist.ftpsts = 1; // 默认上传，上传-1，不上传-2！

    v_newfilelist.push_back(stfilelist);

  }

  // 把本次文件和上次文件做对比，得出需要上传的文件。
  for (UINT ii=0;ii<v_newfilelist.size();ii++)
  {
    if (v_newfilelist.size() == 0) continue;

    // 把远程目录的文件名、大小和日期和本地的清单文件比较一下，如果全部相同，就把ftpsts设置为2，不再采集
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

  // 上传文件
  ftpputforponds();

  // 把已经推送过的文件信息写入List文件
  WriteToXML();

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  ftp.logout();

  // 把已经推送过的文件信息写入List文件
  WriteToXML();

  logfile.Write("ftpputforponds exit.\n");

  exit(0);
}


// 把已经推送过的文件信息写入List文件
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

// 建立与对方服务器的连接，并登录
BOOL ftplogin()
{
  int imode=FTPLIB_PASSIVE;

  // 设置传输模式
  if (strcmp(strmode,"port") == 0) imode=FTPLIB_PORT;

  // FTP连接和登录的超时时间设为80就够了。
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",80);

  if (ftp.login(strremoteip,uport,strusername,strpassword,imode) == FALSE)
  {
    logfile.Write("ftp.login(%s,%lu,%s,%s) failed.\n",strremoteip,uport,strusername,strpassword); return FALSE;
  }

  // 程序的超时时间再设为utimeout秒。
  ProgramActive.SetProgramInfo(&logfile,"ftpputforponds",utimeout);

  // 在对方服务器上创建目录
  ftp.mkdir(strremotepath);

  return TRUE;
}

// 把文件发送到对方服务器目录
BOOL ftpputforponds()
{
  char strFullLocalFileName[301];
  char strFullLocalFileNameBak[301];
  char strFullRemoteFileName[301];
  char strFullRemoteFileNameTmp[301];
  BOOL bConnected=FALSE;

  for (UINT kk=0;kk<v_newfilelist.size();kk++)
  {
    // 把文件采集到本地服务器目录
    if (v_newfilelist[kk].ftpsts==1)
    {
      memcpy(&stfilelist,&v_newfilelist[kk],sizeof(stfilelist));

      // 如果没有连上对方的FTP服务器，就连接
      if (bConnected==FALSE)
      {
        // 连接服务器
        if (ftplogin() == FALSE) return FALSE;

        ProgramActive.WriteToFile();

        bConnected=TRUE;
      }

      // 如果文件大小为0，则跳过。
      if (stfilelist.filesize == 0) continue;

      memset(strFullLocalFileName,0,sizeof(strFullLocalFileName));
      memset(strFullLocalFileNameBak,0,sizeof(strFullLocalFileNameBak));
      memset(strFullRemoteFileName,0,sizeof(strFullRemoteFileName));
      memset(strFullRemoteFileNameTmp,0,sizeof(strFullRemoteFileNameTmp));

      snprintf(strFullLocalFileName,300,"%s/%s",strlocalpath,stfilelist.localfilename);
      snprintf(strFullLocalFileNameBak,300,"%s/%s",strlocalpathbak,stfilelist.localfilename);
      snprintf(strFullRemoteFileName,300,"%s/%s",strremotepath,stfilelist.localfilename);
      snprintf(strFullRemoteFileNameTmp,300,"%s/%s.tmp",strremotepath,stfilelist.localfilename);

      // 删除远程目录的临时文件和正式文件，因为如果对方目录已存在该文件，可能会导致文件传输不成功能情况
      // 但是，新的类库不知道还会不会这样
      if (strcmp(strdeleteremote,"TRUE")==0) { ftp.ftpdelete(strFullRemoteFileNameTmp); ftp.ftpdelete(strFullRemoteFileName); }

      CTimer Timer;

      Timer.Beginning();

      logfile.Write("put %s(size=%ld,mtime=%s)...",strFullLocalFileName,stfilelist.filesize,stfilelist.modtime);

      BOOL bIsLogFile=MatchFileName(stfilelist.localfilename,"*.LOG");

      // 发送文件
      if (ftp.put(strFullLocalFileName,strFullRemoteFileName,bIsLogFile) == FALSE)
      {
        logfile.WriteEx("failed.\n%s",ftp.response()); continue;
      }
      else
      {
        // 标记为已上传
        v_newfilelist[kk].ftpsts=2;

        logfile.WriteEx("ok(time=%d).\n",Timer.Elapsed());
      }

      if (strlen(strlocalpathbak) == 0)
      {
        // 删除本地目录的该文件
        if (REMOVE(strFullLocalFileName) == FALSE) 
        { 
          logfile.WriteEx("\nREMOVE(%s) failed.\n",strFullLocalFileName); continue; 
        }
      }
      else
      {
        // 把本地目录的该文件移到备份的目录去，如果localpath等于localpathbak，保留源文件不变
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

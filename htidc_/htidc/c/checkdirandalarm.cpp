#include "_public.h"
#include "_oracle.h"

// 程序异常中断处理函数
void EXIT(int sig);

CLogFile       logfile;                // 日志文件类
CProgramActive ProgramActive;          // 后台进程监控类
CFile          File;
CDir           Dir;
connection     conn;

char pathname[301];
char andchild[11];
char mobilenostr[1001];
char strconnstr[101];
char strdwid[3];
char strywid[3];
char strclientname[21];
char strsendport[21];
char alarmcontent[301];
char strid[51];
long alarmcount;

struct st_DISKSTAT
{
  char filesystem[101];
  char total[31];
  char used[31];
  char available[31];
  char usep[31];
  char mount[31];
}m_stDISKSTAT;

// 把告警插入到短信平台
BOOL PalarmToSMS();

int main(int argc,char *argv[])
{
  if (argc != 3)
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/checkdirandalarm checkdirandalarm.xml ipaddr\n\n");

    printf("Example:/htidc/htidc/bin/procctl 300 /htidc/htidc/bin/checkdirandalarm /htidc/szqx/c/checkdirandalarm.xml 10.153.130.58\n\n");

    printf("程序读取checkdirandalarm.xml参数文件，分析待监控目录下的文件数、文件大小、磁盘空间是否超出阀值，如果超出，就发出短信告警。\n");
    printf("文件大小设定为超过200MB，磁盘空间设定为超过50%%就告警。\n");
    printf("本程序的罩疚募�/tmp/htidc/log/checkdirandalarm.log。\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,EXIT); signal(SIGTERM,EXIT);

  // 打开程序运行日志
  if (logfile.Open("/tmp/htidc/log/checkdirandalarm.log","a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n","/tmp/htidc/log/checkdirandalarm.log"); return -1;
  }

  logfile.SetAlarmOpt("checkdirandalarm");

  // 注意，程序超时是300秒
  ProgramActive.SetProgramInfo(&logfile,"checkdirandalarm",300);

  logfile.Write("开始分析.\n");

  if (File.OpenForRead(argv[1],"r") == FALSE)
  {
    logfile.Write("File.OpenForRead(%s) failed.\n",argv[2]); EXIT(-1);
  }

  char strLocalTime[21];
  char strxmlbuffer[1024];
  UINT ii =0;

  while (TRUE)
  {
    memset(strxmlbuffer,0,sizeof(strxmlbuffer));

    if (File.FFGETS(strxmlbuffer,1000) == FALSE) break;

    if (strstr(strxmlbuffer,"#") != 0) continue;

    if (strstr(strxmlbuffer,"sendport") != 0)
    {
      memset(mobilenostr,0,sizeof(mobilenostr));
      memset(strconnstr,0,sizeof(strconnstr));
      memset(strdwid,0,sizeof(strdwid));
      memset(strywid,0,sizeof(strywid));
      memset(strclientname,0,sizeof(strclientname));
      memset(strsendport,0,sizeof(strsendport));

      GetXMLBuffer(strxmlbuffer,"connstr",strconnstr,100);
      GetXMLBuffer(strxmlbuffer,"dwid",strdwid,2);
      GetXMLBuffer(strxmlbuffer,"ywid",strywid,2);
      GetXMLBuffer(strxmlbuffer,"clientname",strclientname,20);
      GetXMLBuffer(strxmlbuffer,"sendport",strsendport,20);
      GetXMLBuffer(strxmlbuffer,"mobilenostr",mobilenostr,1000);
     
      logfile.Write("connstr=%s\ndwid=%s\nywid=%s\nclientname=%s\nsendport=%s\nmobilenostr=%s\n",strconnstr,strdwid,strywid,strclientname,strsendport,mobilenostr);
    }
    else
    {
      memset(pathname,0,sizeof(pathname));
      memset(andchild,0,sizeof(andchild));
      alarmcount=0;

      GetXMLBuffer(strxmlbuffer,"pathname",pathname,300);
      GetXMLBuffer(strxmlbuffer,"andchild",andchild,5);
      GetXMLBuffer(strxmlbuffer,"alarmcount",&alarmcount);
    }

    if ( (strlen(pathname)==0) || (strlen(andchild)==0) ) continue;

    BOOL bAndChild=FALSE;

    if ( (strcmp(andchild,"true")==0) || (strcmp(andchild,"TRUE")==0) ) bAndChild=TRUE;

    if (Dir.OpenDir(pathname,bAndChild) == FALSE)
    {
      logfile.Write("Dir.OpenDir(%s) failed.\n",pathname); continue;
    }

    logfile.Write("pathname=%s,andchild=%s,alarmcount=%ld,fileexist=%lu.\n",pathname,andchild,alarmcount,Dir.m_vFileName.size());

    if (Dir.m_vFileName.size() >= (UINT)alarmcount)
    {
      logfile.Write("目录%s（是否包括子目录：%s）中的文件数为%lu，超过了监控的阀值%ld。\n",pathname,andchild,Dir.m_vFileName.size(),alarmcount);
      
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyymmddhh24miss");
      snprintf(strid,30,"%s%ld",strLocalTime,++ii);

      memset(alarmcontent,0,sizeof(alarmcontent));
      snprintf(alarmcontent,300,"(%s)目录%s中的文件数为%lu，超过了监控的阀值%ld。\n",argv[2],pathname,Dir.m_vFileName.size(),alarmcount);
     
      if (PalarmToSMS() == FALSE) logfile.Write("PalarmToSMS() failed.\n");

    }

    // 20190111 新增监控目录下的文件大小，超过200MB就告警。
    while (TRUE)
    {
      if (Dir.ReadDir() == FALSE) break;
   
      if (Dir.m_FileSize >= 200*1024*1024)
      {
        logfile.Write("文件：%s 的大小为%ldMB，超过了监控的阀值100MB。\n",Dir.m_FullFileName,Dir.m_FileSize/1024/1024);
      
        memset(strLocalTime,0,sizeof(strLocalTime));
        LocalTime(strLocalTime,"yyyymmddhh24miss");
        snprintf(strid,30,"%s%ld",strLocalTime,++ii);

        memset(alarmcontent,0,sizeof(alarmcontent));
        snprintf(alarmcontent,300,"(%s)文件：%s 的大小为%ldMB，超过了监控的阀值100MB。\n",argv[2],Dir.m_FullFileName,Dir.m_FileSize/1024/1024);
     
        if (PalarmToSMS() == FALSE) logfile.Write("PalarmToSMS() failed.\n");
      }
    }

  }

  logfile.Write("分析目录中的文件数及文件大小完成.\n");

  // 20190111 新增监控服务器磁盘空间，超过50% 就告警。
  FILE *fp=0;

  if ( (fp=popen("df -k --block-size=1M","r")) == NULL )
  {
    logfile.Write("exec /bin/df -k failed.\n"); return FALSE;
  }

  CCmdStr CmdStr;

  char strLine[512],strLine1[512];

  while (TRUE)
  {
    memset(strLine,0,sizeof(strLine));

    memset(&m_stDISKSTAT,0,sizeof(m_stDISKSTAT));

    if (FGETS(strLine,500,fp) == FALSE) break;

    // 如果没有找到“%”，就再读取一行
    if (strstr(strLine,"%") == 0)
    {
      memset(strLine1,0,sizeof(strLine1));
      if (FGETS(strLine1,500,fp) == FALSE)  break;
      strcat(strLine," "); strcat(strLine,strLine1);
    }

    // 如果找不到“/”，就越过该行，它很可能是第一行的标题
    if (strstr(strLine,"/dev") == 0) continue;

    // 在有些服务器上,内存也是作为文件系统mount上去的,以下两行代码删除内存文件系统
    if (strstr(strLine,"none")     != 0) continue;
    if (strstr(strLine,"/boot")    != 0) continue;
    if (strstr(strLine,"/dev/shm") != 0) continue;

    // 挂载目录也排除(带IP地址的就是挂载目录)
    if (MatchFileName(strLine,"*.*.*.*")==TRUE) continue;

    // 删除字符串前后的空格
    Trim(strLine);

    // 把字符串中间的多个空格全部转换为一个空格
    UpdateStr(strLine,"  "," ");

    CmdStr.SplitToCmd(strLine," ");

    CmdStr.GetValue(0,m_stDISKSTAT.filesystem,100);
    CmdStr.GetValue(1,m_stDISKSTAT.total,30);
    CmdStr.GetValue(2,m_stDISKSTAT.used,30);
    CmdStr.GetValue(3,m_stDISKSTAT.available,30);
    CmdStr.GetValue(4,m_stDISKSTAT.usep,30);
    CmdStr.GetValue(5,m_stDISKSTAT.mount,30);

    snprintf(m_stDISKSTAT.usep,30,"%d",atoi(m_stDISKSTAT.usep));

    if (atoi(m_stDISKSTAT.usep) >= 50)
    {
      logfile.Write("目录：%s 的使用率为%s%%，超过了监控的阀值50%。\n",m_stDISKSTAT.mount,m_stDISKSTAT.usep);
      
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyymmddhh24miss");
      snprintf(strid,30,"%s%ld",strLocalTime,++ii);

      memset(alarmcontent,0,sizeof(alarmcontent));
      snprintf(alarmcontent,300,"(%s)目录：%s 的使用率为%s%%，超过了监控的阀值50%%。\n",argv[2],m_stDISKSTAT.mount,m_stDISKSTAT.usep);
     
      if (PalarmToSMS() == FALSE) logfile.Write("PalarmToSMS() failed.\n");
    }
  }

  logfile.Write("分析服务器磁盘空间完成.\n");

  return 0;
}

void EXIT(int sig)
{
  if (sig > 0) 
  {
    signal(sig,SIG_IGN); logfile.Write("catching the signal(%d).\n",sig);
  }

  logfile.Write("checkdirandalarm EXIT.\n");

  exit(0);
}

// 把告警插入到短信平台
BOOL PalarmToSMS()
{
  // 连接短信平台
  if (conn.connecttodb(strconnstr,TRUE) != 0)
  {
     logfile.Write("conn.connecttodb(%s) failed.\n",strconnstr); return FALSE;
  }
  
  sqlstatement stmtins;
  stmtins.connect(&conn);
  stmtins.prepare("\
    insert into T_SMSSENDLIST(id,dwid,ywid,clientname,sendport,mobileno,content,crttime)\
          values(:1,:2,:3,:4,:5,:6,:7,sysdate)");
  stmtins.bindin(1,strid,30);
  stmtins.bindin(2,strdwid,2);
  stmtins.bindin(3,strywid,2);
  stmtins.bindin(4,strclientname,20);
  stmtins.bindin(5,strsendport,20);
  stmtins.bindin(6,mobilenostr,11);
  stmtins.bindin(7,alarmcontent,300);

  if (stmtins.execute() != 0)
  {
    logfile.Write("insert T_SMSSENDLIST failed.%s\n",stmtins.cda.message); return FALSE;
  }

  return TRUE;
}

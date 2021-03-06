#include "idcapp.h"

void CallQuit(int sig);

char strxmlbuffer[4001];
char strlogfilename[301];
char strconnstr[301];
char strstdpath[301];
char strstdbakpath[301];
char strstderrpath[301];
char striflog[31];
char strindexid[31]; 
char strLocalTime[21]; 
char strcharset[101];

connection     conn;
CLogFile       logfile;
CProgramActive ProgramActive;
CDir           DFileDir;
CQXDATA        QXDATA;
FILE           *stdfp;
int            iFileVer;  // 待入库文件的版本，1-数据中心新版本；2-数据中心旧版本；3-省局通用接口格式
CRealMon       RealMon;

// 返回值：0-成功；1-应用数据定义错；2-打开文件失败或文件状态不正确；3-操作数据库表错误
int _dfiletodb();

// 执行XML文件首部的SQL
long ExecSQL();

int main(int argc,char *argv[])
{
  if (argc != 2) 
  {
    printf("\n");
    printf("Using:/htidc/htidc/bin/dfiletodb inifile\n");
    printf("Postgres Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/bin/dfiletodb \"<logfilename>/log/fs/gxpt/dfiletodb.log</logfilename><connstr>host=223.4.5.165 user=postgres password=123456 dbname=gxpt port=5433</connstr><stdpath>/qxdata/fs/gxpt/sdata/std</stdpath><stdbakpath>/qxdata/fs/gxpt/sdata/stdbak</stdbakpath><stderrpath>/qxdata/fs/gxpt/sdata/stderr</stderrpath><iflog>true</iflog><indexid>30001</indexid>\"\n\n");
    printf("Mysql Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/c/dfiletodb \"<logfilename>/log/szqx/dfiletodb_mysql.log</logfilename><connstr>10.153.121.120,root,123456,szqx,3306</connstr><stdpath>/qxdata/szqx/sdata/stdzdz</stdpath><stdbakpath>/qxdata/szqx/sdata/stdbak</stdbakpath><stderrpath>/qxdata/szqx/sdata/stderr</stderrpath><iflog>true</iflog><indexid>30001</indexid><charset>gbk</charset>\"\n\n");
    printf("Oracle Example:/htidc/htidc/bin/procctl 10 /htidc/htidc/c/dfiletodb \"<logfilename>/log/szqx/dfiletodb_mysql.log</logfilename><connstr>szidc/pwdidc@SZQX_10.153.98.31</connstr><stdpath>/qxdata/szqx/sdata/stdzdz</stdpath><stdbakpath>/qxdata/szqx/sdata/stdbak</stdbakpath><stderrpath>/qxdata/szqx/sdata/stderr</stderrpath><iflog>true</iflog><indexid>30001</indexid><charset>Simplified Chinese_China.ZHS16GBK</charset>\"\n\n");
    
    printf("二维表格数据入库的主程序，支持oracle、mysql、pg数据库。\n");
    printf("logfilename 本程序运行的日志文件。\n");
    printf("connstr 数据库连接参数，oracle数据库是username/password@tnsname，pg数据库是host= user= password= dbname= port=。\n");
    printf("stdpath 待入库的xml文件存放的目录。\n");
    printf("stdbakpath 入库后的文件备份的目录。可以为空\n");
    printf("stderrpath 如果入库发生错误，xml文件将被转移到该目录。\n");
    printf("iflog 如果xml文件中的记录入库失败，是否记录日志，true-记录，false-不记录。\n\n");
    printf("indexid 数据监控的流程编号，如果为空，则不生成数据采集监控文件。\n");
    printf("注意：\n");
    printf("  1、stdbakpath目录下的文件由deletefiles定时清理，stderrpath目录下的文件系统管理员要定期检查，找出失败原因。\n"); 
    printf("  2、如果采用的是pg数据库，必须在数据库中先运行以下脚本创建函数。\n");
    printf("  create or replace function to_null(varchar) returns numeric as $$\n");
    printf("  begin\n");
    printf("  if (length($1)=0) then\n");
    printf("    return null;\n");
    printf("  else\n");
    printf("    return $1;\n");
    printf("  end if;\n");
    printf("  end\n");
    printf("  $$ LANGUAGE plpgsql;\n");
    printf("  3、如果采用的是oracle数据库，必须在数据库中先运行以下脚本创建函数。\n");
    printf("  create or replace function to_null(in_value in varchar2) return varchar2\n");
    printf("  is\n");
    printf("  begin\n");
    printf("    return in_value;\n");
    printf("  end;\n");
    printf("  /\n\n");
 
    return -1;
  }

  memset(strxmlbuffer,0,sizeof(strxmlbuffer));
  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strstdpath,0,sizeof(strstdpath));
  memset(strstdbakpath,0,sizeof(strstdbakpath));
  memset(strstderrpath,0,sizeof(strstderrpath));
  memset(striflog,0,sizeof(striflog));
  memset(strindexid,0,sizeof(strindexid));
  memset(strcharset,0,sizeof(strcharset));

  strncpy(strxmlbuffer,argv[1],4000);

  GetXMLBuffer(strxmlbuffer,"logfilename",strlogfilename,300);
  GetXMLBuffer(strxmlbuffer,"connstr",strconnstr,300);
  GetXMLBuffer(strxmlbuffer,"stdpath",strstdpath,300);
  GetXMLBuffer(strxmlbuffer,"stderrpath",strstderrpath,300);
  GetXMLBuffer(strxmlbuffer,"iflog",striflog,30);
  GetXMLBuffer(strxmlbuffer,"charset",strcharset,100);
  GetXMLBuffer(strxmlbuffer,"indexid",strindexid,30);
  if(strstr(strxmlbuffer,"stdbakpath")!=0){ GetXMLBuffer(strxmlbuffer,"stdbakpath",strstdbakpath,300); }

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0) { printf("connstr is null.\n"); return -1; }
  if (strlen(strstdpath) == 0) { printf("stdpath is null.\n"); return -1; }
  // if (strlen(strstdbakpath) == 0) { printf("stdbakpath is null.\n"); return -1; }
  if (strlen(strstderrpath) == 0) { printf("stderrpath is null.\n"); return -1; }
  // if (strlen(striflog) == 0) { printf("iflog is null.\n"); return -1; }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);

  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  logfile.SetAlarmOpt("dfiletodb");

  // 注意，程序超时是500秒
  ProgramActive.SetProgramInfo(&logfile,"dfiletodb",500);

  // 连接应用数据库，此数据库连接用于正常的数据处理
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("conn.connecttodb(%s) failed\n",strconnstr); CallQuit(-1);
  }

  // 设置字符集
  if (strlen(strcharset) != 0) conn.character(strcharset);

  ProgramActive.SetProgramInfo(&logfile,"dfiletodb",180);

  logfile.Write("dfiletodb beging.\n");

  QXDATA.BindConnLog(&conn,&logfile);

  int timetvl=31;

  BOOL bContinue=FALSE;
  char strSTDBAKFileName[201];
  char strSTDERRFileName[201];

  while (TRUE)
  {
    // 写入进程活动信息
    ProgramActive.WriteToFile();

    // 写进程活动日志文件
    RealMon.WriteToProActLog(strindexid,"dfiletodb",500);

    if (timetvl++>30) // 重新载入应用数据入库参数（T_APPDTYPE）
    {
      timetvl=0;

      if (QXDATA.LoadFileCFG() != 0) CallQuit(-1);
    }

    char strCMD[1024];
    memset(strCMD,0,sizeof(strCMD));
    snprintf(strCMD,1000,"/usr/bin/gunzip -f %s/*.gz 1>/dev/null 2>/dev/null",strstdpath);
    system(strCMD);

    // 打开标准格式文件目录
    if (DFileDir.OpenDir(strstdpath) == FALSE)
    {
      logfile.Write("DFileDir.OpenDir %s failed.\n",strstdpath); CallQuit(-1);
    }

    bContinue = FALSE;

    // 逐行获取每个文件并入库
    while (DFileDir.ReadDir() == TRUE)
    {
      // 写入进程活动信息
      ProgramActive.WriteToFile();

      // 如果文件的时间在当前时间的前5秒之内，就暂时不入库，这么做的目的是为了保证数据文件的完整性。
      memset(strLocalTime,0,sizeof(strLocalTime));
      LocalTime(strLocalTime,"yyyy-mm-dd hh24:mi:ss",0-10);
      if (strcmp(DFileDir.m_ModifyTime,strLocalTime)>0) continue;

      memset(strSTDBAKFileName,0,sizeof(strSTDBAKFileName));
      memset(strSTDERRFileName,0,sizeof(strSTDERRFileName));
      
      snprintf(strSTDERRFileName,300,"%s/%s",strstderrpath,DFileDir.m_FileName);

      // 如果是*.XML.GZ，就不处理，跳过
      if ( MatchFileName(DFileDir.m_FileName,"*.XML.GZ") == TRUE) continue;

      // 如果文件名不是以.XML结束，就不处理该文件
      if ( MatchFileName(DFileDir.m_FileName,"*.XML") == FALSE) 
      {
        logfile.Write("FileName %s is invalidation!\n",DFileDir.m_FullFileName); 
        REMOVE(DFileDir.m_FullFileName); 
        continue;
      }

      // 开始处理每个文件
      logfile.Write("Process file %s...",DFileDir.m_FileName);

      // 返回值：0-成功；1-应用数据定义错；2-打开文件失败或文件状态不正确；3-操作数据库表错误
      if (_dfiletodb() == 0)
      { 
        // 如果设置了备份目录参数，那就移到备份目录，否则就删除文件
        if (strlen(strstdbakpath) != 0) 
        {
          snprintf(strSTDBAKFileName,300,"%s/%s",strstdbakpath,DFileDir.m_FileName); 

          // 文件入库成功，把它移动stdbak目录
          if (rename(DFileDir.m_FullFileName,strSTDBAKFileName) != 0)
          {
            REMOVE(DFileDir.m_FullFileName); 
            logfile.WriteEx("failed.rename %s failed.\n",DFileDir.m_FullFileName); 
          }
          else
          {
            logfile.WriteEx("ok.\n"); bContinue = TRUE; 

	    if (conn.commitwork() != 0) 
	    {
	      logfile.Write("conn.commitwork() failed,message=%s\n",conn.lda.message);
	      break;
	    }
          }
        }
        else
        {
          REMOVE(DFileDir.m_FullFileName); logfile.WriteEx("ok.\n"); bContinue = TRUE; 

	  if (conn.commitwork() != 0) 
	  {
	    logfile.Write("conn.commitwork() failed,message=%s\n",conn.lda.message);
	    break;
	  }
        }
      }
      else
      {
        // 文件入库时发生了错误，把它移到stderr目录
        logfile.WriteEx("failed.\n"); 
        if (rename(DFileDir.m_FullFileName,strSTDERRFileName) != 0) REMOVE(DFileDir.m_FullFileName); 
      }

      // 写入进程活动信息
      ProgramActive.WriteToFile();
    }

    // 生成数据入库日志XML文件
    if (RealMon.WriteToDbLog() == FALSE) logfile.Write("WriteToDbLog failed.\n"); 

    // 生成之后要清空容器，不然容器里还有历史数据。
    RealMon.m_vTODBLOG.clear();

    if (bContinue == FALSE) sleep(5);
  }

  logfile.Write("dfiletodb exit.\n");

  return 0;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  logfile.Write("catching the signal(%d).\n",sig);

  if (conn.disconnect() !=0)logfile.Write("conn.disconnect() failed,%s\n",conn.lda.message);

  // 生成数据入库日志XML文件
  if (RealMon.WriteToDbLog() == FALSE) logfile.Write("WriteToDbLog failed.\n"); 

  logfile.Write("dfiletodb exit.\n");

  if (stdfp != 0) fclose(stdfp);

  exit(0);
}

// 返回值：0-成功；1-应用数据定义错；2-打开文件失败或文件状态不正确；3-操作数据库表错误
int _dfiletodb()
{
  // 根据文件名和文件类型获取气象文件类型参数
  strcpy(QXDATA.m_filename,DFileDir.m_FileName);

  // 1-应用数据定义错
  if (QXDATA.GETFILECFG() == FALSE) { logfile.WriteEx("QXDATA.GETFILECFG "); return 1; }

  stdfp=0;

  // 打开XML文件
  if ( (stdfp=FOPEN(DFileDir.m_FullFileName,"r")) == 0 ) { logfile.WriteEx("FOPEN "); return 2; }

  // 判断文件是否以"</data>"结束,旧以END结束
  if ( (CheckFileSTS(DFileDir.m_FullFileName,"</data>") == FALSE) &&
       (CheckFileSTS(DFileDir.m_FullFileName,"END") == FALSE) &&
       (CheckFileSTS(DFileDir.m_FullFileName,"</DS>") == FALSE) )
  { 
    logfile.WriteEx(" state  "); fclose(stdfp); stdfp=0; return 2; 
  }

  iFileVer=2;
  if (CheckFileSTS(DFileDir.m_FullFileName,"</data>") == TRUE) iFileVer=1;
  if (CheckFileSTS(DFileDir.m_FullFileName,"END") == TRUE)     iFileVer=2;
  if (CheckFileSTS(DFileDir.m_FullFileName,"</DS>") == TRUE)   iFileVer=3;

  // 获取对应表的字段信息
  if (QXDATA.GETTABCOLUMNS() != 0) { logfile.WriteEx("QXDATA.GETTABCOLUMNS "); fclose(stdfp); stdfp=0; return 3; }

  // 如果没有获取到字段信息，就是表不存在，返回
  if (QXDATA.m_vTABCOLUMNS.size() == 0) { logfile.WriteEx("table %s no exist ",QXDATA.m_tname); fclose(stdfp); stdfp=0; return 1; }

  // 生成操作数据表的SQL语句
  QXDATA.CrtSQL();

  // 准备操作数据表的SQL语句，包括查询、更新和插入
  if (QXDATA.PreSQL() != 0) { logfile.WriteEx("QXDATA.PreSQL "); fclose(stdfp); stdfp=0; return 3; }

  // 执行XML文件首部的SQL
  if (ExecSQL() != 0) { logfile.WriteEx("ExecSQL "); fclose(stdfp); stdfp=0; return 3; }
 
  UINT uInsUptCount=0;       // 插入和更新的总记录数
  UINT uActiveCount=0;
  char strBuffer[81920];

  uInsUptCount=0;

  // 处理文件的全部行
  while (TRUE)
  {
    memset(strBuffer,0,sizeof(strBuffer));

    // 读取一行
    if (iFileVer==1)
    {
      if (FGETS(strBuffer,80000,stdfp,"<endl/>") == FALSE) break;
    }
    if (iFileVer==2)
    {
      if (FGETS(strBuffer,80000,stdfp,"endl") == FALSE) break;
    }
    if (iFileVer==3)
    {
      if (FGETS(strBuffer,80000,stdfp,"</R>") == FALSE) break;
    }

    // 一行的内容不可能小于5，如果小于5，一般是空行，丢弃它。
    if (strlen(strBuffer) < 5) continue;

    // 把每行的内容解包到字段中
    QXDATA.UNPackBuffer(strBuffer,iFileVer);

    QXDATA.BindParams();

    // 插入或更新到表中
    if (QXDATA.InsertTable(strBuffer,striflog) != 0) { fclose(stdfp); stdfp=0; return 3; }

    /*
    // 如果出现了无效的记录，打出来
    if ( uInsUptCount == (QXDATA.m_InsCount+QXDATA.m_UptCount) )
    {
      logfile.Write("cda.rc=%ld,line=%s\n",QXDATA.stmtinserttable.cda.rc,strBuffer);
    }

    uInsUptCount=QXDATA.m_InsCount+QXDATA.m_UptCount;
    */

    // 每处理1000条记录时写入一次进程活动信息
    if (uActiveCount >= 1000) { uActiveCount=0; ProgramActive.WriteToFile(); }

    uActiveCount++;
  }

  fclose(stdfp); stdfp=0;

  QXDATA.m_DiscardCount=QXDATA.m_TotalCount-QXDATA.m_UptCount-QXDATA.m_InsCount;

  logfile.WriteEx("total=%lu,insert=%lu,update=%lu,discard=%lu.",\
                   QXDATA.m_TotalCount,QXDATA.m_InsCount,QXDATA.m_UptCount,QXDATA.m_DiscardCount);

  // 收集入库信息，用于生成入库日志。
  if (strlen(strindexid) != 0)
  {
    memset(&RealMon.m_stTODBLOG,0,sizeof(RealMon.m_stTODBLOG));
    strcpy(RealMon.m_stTODBLOG.indexid,strindexid);
    getLocalIP(RealMon.m_stTODBLOG.serverip);
    strcpy(RealMon.m_stTODBLOG.programname,"dfiletodb");

    memset(strLocalTime,0,sizeof(strLocalTime));
    LocalTime(strLocalTime,"yyyymmddhh24miss");
    strcpy(RealMon.m_stTODBLOG.ddatetime,strLocalTime);

    strcpy(RealMon.m_stTODBLOG.filetime,DFileDir.m_ModifyTime);
    strcpy(RealMon.m_stTODBLOG.filename,DFileDir.m_FullFileName);
    snprintf(RealMon.m_stTODBLOG.filesize,50,"%lu",DFileDir.m_FileSize);
    strcpy(RealMon.m_stTODBLOG.tname,QXDATA.m_tname);
    snprintf(RealMon.m_stTODBLOG.total,50,"%lu",QXDATA.m_TotalCount);
    snprintf(RealMon.m_stTODBLOG.insrows,50,"%lu",QXDATA.m_InsCount);
    snprintf(RealMon.m_stTODBLOG.uptrows,50,"%lu",QXDATA.m_UptCount);
    snprintf(RealMon.m_stTODBLOG.disrows,50,"%lu",QXDATA.m_DiscardCount);

    RealMon.m_vTODBLOG.push_back(RealMon.m_stTODBLOG);
  }

  return 0;
}


// 执行XML文件首部的SQL
long ExecSQL()
{
  char strLine[2048],strSQL[2048];
  memset(strLine,0,sizeof(strLine));
  memset(strSQL,0,sizeof(strSQL));

  if (iFileVer==1)
  {
    if (FGETS(strLine,2000,stdfp,"<endl/>") == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
    if (GetXMLBuffer(strLine,"sql",strSQL,2000) == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
  }
  if (iFileVer==2)
  {
    if (FGETS(strLine,1000,stdfp) == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
    if (strcmp(strLine,"execsql") != 0) { fseek(stdfp,0L,SEEK_SET); return 0; }
    if (FGETS(strSQL,2000,stdfp,"endl") == FALSE) { fseek(stdfp,0L,SEEK_SET); return 0; }
  }

  if (iFileVer==3) return 0;

  sqlstatement stmt;
  stmt.connect(&conn);
  if (strcmp(conn.m_dbtype,"oracle")==0)
  {
    stmt.prepare("\
      BEGIN\
        %s\
      END;",strSQL);
  }
  else
  {
    stmt.prepare(strSQL);
  }
  if (stmt.execute() != 0)
  {
    logfile.Write("%s\n%s\n",strSQL,stmt.cda.message);
  }

  // 注意，以下代码不能启用，在这里不能再定位到文件的首部，不能把sql等行当成数据处理。
  // fseek(stdfp,0L,SEEK_SET);

  return stmt.cda.rc;
}

/*
create or replace function to_null(varchar) returns numeric as $$
begin
if (length($1)=0) then
  return null;
else
  return $1;
end if;
end
$$ LANGUAGE plpgsql;
*/


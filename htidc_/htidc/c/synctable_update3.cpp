#include "idcapp.h"

connection conn;
CLogFile   logfile;
CProgramActive ProgramActive;

void CallQuit(int sig);

int lobfieldcount=0;

// 判断目的表是否存在，如果表不存在，返回失败
// 判断目的表是否有dstpkfieldname列，如果没有，返回失败
// 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
BOOL CheckDstTName();

// 执行数据同步
BOOL SyncTable();

char strlogfilename[301];
char strconnstr[101];
char strcharset[51];
char strsrctname[51];
char strsrcpkfieldname[51];
char strdstpkfieldname[51];
char strsrcfields[2001];
char strdsttname[51];
char strdstfields[2001];
char strwhere[501];
char stralarmbz[21];
int  maxcount;

int main(int argc,char *argv[])
{
  if (argc != 2)  
  {
    printf("Using:/htidc/htidc/bin/synctable_update3 xmlbuffer\n");
    printf("Example:/htidc/htidc/bin/procctl 50 /htidc/htidc/bin/synctable_update3 \"<logfilename>/log/szqx/synctable_update3_INDEXINFO.log</logfilename><charset>Simplified Chinese_China.ZHS16GBK</charset><connstr>szidc/pwdidc@SZQX_10.153.98.24</connstr><srctname>T_INDEXINFO</srctname><srcpkfieldname>cityid</srcpkfieldname><dstpkfieldname>cityid</dstpkfieldname><srcfields>cityid,cityname,pubdate,gm</srcfields><dsttname>T_INDEXINFO@SZIDC</dsttname><dstfields>cityid,cityname,pubdate,gm</dstfields><where>where pubdate>sysdate-1</where><maxcount>300</maxcount><alarmbz>FALSE</alarmbz>\"\n\n");

    printf("这是一个工具程序，用于在Oracle数据库之间同步数据，同步的方法是全表刷新或按条件刷新。\n");
    printf("synctable_update3程序为了不能在目的数据库中创建数据库链路增加，更改为在本地数据库创建链路,用法基本同synctable_update1使用相同，不同点在constr为本地数据库连接参数和只能在dsttname中加目的数据库链路。\n");
    printf("<logfilename>/log/szqx/synctable_update3_INDEXINFO.log</logfilename> 本程序运行的日志文件名。\n");
    printf("<connstr>szidc/pwdidc@SZQX_10.153.98.24</connstr> 本地数据库的连接参数。\n");
    printf("<charset>Simplified Chinese_China.ZHS16GBK</charset> 数据库的字符集，必须要与<connstr>"\
           "参数描述的数据库相同。\n");
    printf("<srctname>T_INDEXINFO</srctname> 数据源表名。\n");
    printf("<srcpkfieldname>cityid</srcpkfieldname> 数据源表的主键字段名，注意，该字段取值分两种情况：\n"\
           "如果源表有主键字段，那么就填入主键字段，如果没有，就填rowid。如果where为空，本字段无意义。\n");
    printf("<dstpkfieldname>cityid</dstpkfieldname> 数据目的表的主键字段名，注意，该字段取值分两种情况：\n"\
           "如果目的表有主键字段，那么就填入主键字段，如果没有，就填sync_rowid。如果where为空，本字段无意义。\n");
    printf("<srcfields>cityid,cityname,pubdate,gm</srcfields> 数据源表的字段列表，用于填充在select和from之间，"\
           "所以，srcfields可以是真实的字段，也可以是任何Oracle的函数或运算。\n");
    printf("<dsttname>T_INDEXINFO@SZIDC</dsttname> 接收数据的目的表名,在表名后加数据库链路参数，就可以指向远程数据库。\n");
    printf("<dstfields>cityid,cityname,pubdate,gm</dstfields> 接收数据的目的表的字段列表，与"\
           "<srcfields>不同，它必须是真实存在的字段。\n");
    printf("<where>where pubdate>sysdate-1</where> 同步的条件，即select语句的where部分。\n");
    printf("<maxcount>300</maxcount> 执行一次同步操作的记录数，建议采用300。\n");
    printf("<alarmbz>FALSE</alarmbz> 程序运行是否发出告警，缺省是TRUE。\n\n");

    printf("注意：\n");
    printf("  1、<srcfields>和<dstfields>可以为空，如果为空，将以目的表的数据字典中的字段填充。\n");
    printf("     如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。\n");
    printf("  2、<where>可以为空，如果为空，每次都会刷新表中全部的记录。\n");
    printf("  3、<charset>可以为空，如果为空，本程序将不会设置字符集环境参数，那么在操作系统环境中必须已\n");
    printf("     设置正确的NLS_LANG环境变量。\n");
    printf("  4、数据源表和目的表必须要有主键，如果没有主键，可采用rowid同步，目的表必须存在sync_rowid字段，数据类型为rowid，\n");
    printf("     并且，sync_rowid字段必须是唯一索引。\n");
    printf("  5、本程序由procctl调度，运行一次即执行一次同步。\n\n");
    printf("  6、<maxcount>参数非常重要，如果数据源表有非法数据，当<maxcount>为1时，失败的记录无法同步，但不\n");
    printf("     会影响其它的记录，目的表将丢失一条数据，如果<maxcount>大于1，包含失败记录的同一批次的同步将全部失败，目的表将丢失大批数据。\n\n");


    return -1;
  }

  memset(strlogfilename,0,sizeof(strlogfilename));
  memset(strconnstr,0,sizeof(strconnstr));
  memset(strcharset,0,sizeof(strcharset));
  memset(strsrctname,0,sizeof(strsrctname));
  memset(strsrcpkfieldname,0,sizeof(strsrcpkfieldname));
  memset(strdstpkfieldname,0,sizeof(strdstpkfieldname));
  memset(strsrcfields,0,sizeof(strsrcfields));
  memset(strdsttname,0,sizeof(strdsttname));
  memset(strdstfields,0,sizeof(strdstfields));
  memset(strwhere,0,sizeof(strwhere));
  memset(stralarmbz,0,sizeof(stralarmbz));
  maxcount=0;

  GetXMLBuffer(argv[1],"logfilename",strlogfilename,300);
  GetXMLBuffer(argv[1],"connstr",strconnstr,100);
  GetXMLBuffer(argv[1],"charset",strcharset,50);
  GetXMLBuffer(argv[1],"srctname",strsrctname,50);
  GetXMLBuffer(argv[1],"srcpkfieldname",strsrcpkfieldname,50);
  GetXMLBuffer(argv[1],"dstpkfieldname",strdstpkfieldname,50);
  GetXMLBuffer(argv[1],"srcfields",strsrcfields,2000);
  GetXMLBuffer(argv[1],"dsttname",strdsttname,50);
  GetXMLBuffer(argv[1],"dstfields",strdstfields,2000);
  GetXMLBuffer(argv[1],"where",strwhere,500);
  GetXMLBuffer(argv[1],"alarmbz",stralarmbz,10);
  GetXMLBuffer(argv[1],"maxcount",&maxcount);

  if (strlen(strlogfilename) == 0) { printf("logfilename is null.\n"); return -1; }
  if (strlen(strconnstr) == 0)     { printf("connstr is null.\n"); return -1; }
  if (strlen(strcharset) == 0)     { printf("charset is null.\n"); return -1; }
  if (strlen(strsrctname) == 0)    { printf("srctname is null.\n"); return -1; }
  if (strlen(strwhere)!=0) { if (strlen(strsrcpkfieldname) == 0)    { printf("srcpkfieldname is null.\n"); return -1; } }
  if (strlen(strwhere)!=0) { if (strlen(strdstpkfieldname) == 0)    { printf("dstpkfieldname is null.\n"); return -1; } }
  // if (strlen(strsrcfields) == 0)   { printf("srcfields is null.\n"); return -1; }
  if (strlen(strdsttname) == 0)    { printf("dsttname is null.\n"); return -1; }
  // if (strlen(strdstfields) == 0)   { printf("dstfields is null.\n"); return -1; }
  // if (strlen(strwhere) == 0)       { printf("where is null.\n"); return -1; }
  if (strlen(stralarmbz) == 0)           strcpy(stralarmbz,"TRUE");
  if ( (maxcount==0) || (maxcount>300) ) maxcount=300;

  // 规范表名和字段名的大小写
  ToUpper(strsrctname);       ToUpper(strdsttname);
  ToLower(strsrcpkfieldname); ToLower(strdstpkfieldname); ToLower(strsrcfields); ToLower(strdstfields);

  // 如果<srcfields>和<dstfields>中只要有一个参数为空，程序将认为两个参数都为空。
  if ( (strlen(strsrcfields)==0) || (strlen(strdstfields)==0) )
  {
    memset(strsrcfields,0,sizeof(strsrcfields)); memset(strdstfields,0,sizeof(strdstfields));
  }

  // 关闭全部的信号和输入输出
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程
  // 但请不要用 "kill -9 +进程号" 强行终止
  CloseIOAndSignal(); signal(SIGINT,CallQuit); signal(SIGTERM,CallQuit);
 

  // 打开日志文件
  if (logfile.Open(strlogfilename,"a+") == FALSE)
  {
    printf("logfile.Open(%s) failed.\n",strlogfilename); return -1;
  }

  //打开告警
  if (strcmp(stralarmbz,"TRUE")==0) logfile.SetAlarmOpt("synctable_update");

  // 注意，程序超时是1200秒
  ProgramActive.SetProgramInfo(&logfile,"synctable_update3",1200);

  // 如果本地表中有"@"字符，就一定搞错了源表和目的表的关系
  if (strstr(strsrctname,"@") > 0) 
  { 
    logfile.Write("%s表必段是%s连接中指定的数据库的本地表，不允许指向远程数据库的表。\n",\
                   strsrctname,strconnstr);
    return -1;
  }

  // 连接数据库
  if (conn.connecttodb(strconnstr) != 0)
  {
    logfile.Write("connect database %s failed.\n",strconnstr); return -1;
  }

  // 判断目的表是否存在，如果表不存在，返回失败
  // 判断目的表是否有dstpkfieldname列，如果没有，返回失败
  // 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
  if (CheckDstTName()==FALSE) CallQuit(-1);

  // 杀死与同步目的表有关的会话，清理出锁
  KillLocked(&conn,strdsttname);

  // 执行数据同步
  if (SyncTable()==FALSE) CallQuit(-1);

  return 0;
}

// 判断目的表是否存在，如果表不存在，返回失败
// 判断目的表是否有dstpkfieldname列，如果没有，返回失败
// 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
BOOL CheckDstTName()
{
  CTABFIELD TABFIELD;

  // 获取该表全部的字段
  TABFIELD.GetALLField(&conn,strdsttname,0,TRUE);

  if (TABFIELD.m_fieldcount==0)
  {
    logfile.Write("%s表不存在，请创建它。\n",strdsttname); return FALSE;
  }

  // 旧的同步表中有sync_rownum字段，排除它
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rownum,","");
  UpdateStr(TABFIELD.m_allfieldstr,",sync_rownum","");

  // 目的表表名和链路拆分
  char strdblink[101],*strpos=0,strtablename[51];
  memset(strdblink,0,sizeof(strdblink));
  memset(strtablename,0,sizeof(strtablename));

  if ((strpos = strstr(strdsttname,"@")) == 0) return FALSE;

  strncpy(strtablename,strdsttname,strlen(strdsttname)-strlen(strpos));
  strncpy(strdblink,strpos,100);

  // 如果是全表刷新的同步方式，下面的检查都不做了
  if (strlen(strwhere)!=0) 
  {
    // 判断目的表是否有dstpkfieldname列，如果没有，返回失败
    if (strstr(TABFIELD.m_allfieldstr,strdstpkfieldname) == 0)
    {
      logfile.Write("%s表没有%s列，请创建它。\n",strdsttname,strdstpkfieldname); return FALSE;
    }
  
    // 判断dstpkfieldname列是否创建了唯一索引，如果没有，返回失败
    char stroutput[51];
    memset(stroutput,0,sizeof(stroutput));
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare("select column_name from USER_IND_COLUMNS%s where table_name=upper('%s') and column_name=upper('%s')",strdblink,strtablename,strdstpkfieldname);

    //stmt.bindin(1,strdsttname,50);
    stmt.bindout(1,stroutput,50);
    stmt.execute();
    stmt.next();
  
    if (strlen(stroutput)==0)
    {
      logfile.Write("%s表的%s列没有创建唯一索引，请创建它。\n",strdsttname,strdstpkfieldname); return FALSE;
    }
  }

  UpdateStr(TABFIELD.m_allfieldstr,",sync_rowid","");
  UpdateStr(TABFIELD.m_allfieldstr,"sync_rowid,","");

  // <srcfields>和<dstfields>为空，将以目的表的数据字典中的字段填充。
  if ( (strlen(strsrcfields)==0) && (strlen(strdstfields)==0) )
  {
    strcpy(strsrcfields,TABFIELD.m_allfieldstr); strcpy(strdstfields,TABFIELD.m_allfieldstr);
  }

  // 如果是按rowid同步，则为strsrcfields加上rowid列，为strdstfields加上sync_rowid列
  if (strcmp(strsrcpkfieldname,"rowid")==0) 
  {
    strcat(strsrcfields,",rowid"); strcat(strdstfields,",sync_rowid");
  }

  lobfieldcount=0;
  // 判断目的表是否有二进制字段，如果有，把字段的个数存放在lobfieldcount变量中
  sqlstatement stmt;
  stmt.connect(&conn);
  stmt.prepare(\
     "select count(*) from USER_TAB_COLUMNS%s where table_name=upper('%s') and data_type in ('BLOB','CLOB')",strdblink,strtablename);
  //stmt.bindin(1,strdsttname,50);
  stmt.bindout(1,&lobfieldcount);
  stmt.execute();
  stmt.next();

  return TRUE;
}

void CallQuit(int sig)
{
  if (sig > 0) signal(sig,SIG_IGN);

  // 杀死与同步目的表有关的会话，清理出锁
  KillLocked(&conn,strdsttname);

  conn.rollbackwork();

  logfile.Write("catching the signal(%d).\n",sig);

  logfile.Write("synctable_update3 exit.\n");

  exit(0);
}

#define MAXCOUNT 300

// 执行数据同步
BOOL SyncTable()
{
  CTimer Timer;
  logfile.Write("copy %s to %s ... ",strsrctname,strdsttname);

  // 如果同步条件子句为空，就一定是同步该表全部的记录，
  // 这种表的记录一般不会太多，为了保证数据一致性和效率，用一个SQL过程搞定它。
  if (strlen(strwhere)==0)
  {
    sqlstatement stmt;
    stmt.connect(&conn);
    stmt.prepare("\
      BEGIN\
         delete from %s;\
         insert into %s(%s) select %s from %s;\
      END;",strdsttname,strdsttname,strdstfields,strsrcfields,strsrctname);
    if (stmt.execute() != 0)
    {
      logfile.WriteEx("failed.\n%s\n%s\n",stmt.m_sql,stmt.cda.message); return FALSE;
    }

    logfile.WriteEx("ok,table refreshed(%d).\n",Timer.Elapsed());

    conn.commitwork();

    return TRUE;
  }

  // 同步条件子句不为空，就按条件更新
  UINT uMaxCount=0;
  UINT ccount=0;
  char pkfieldvalue[51],pkfieldvaluen[MAXCOUNT][51];

  if (lobfieldcount == 0) uMaxCount=maxcount;
  if (lobfieldcount >  0) uMaxCount=1;

  sqlstatement selsrc,delstmt,insstmt;

  selsrc.connect(&conn);
  selsrc.prepare("select %s from %s %s",strsrcpkfieldname,strsrctname,strwhere);
  selsrc.bindout(1,pkfieldvalue,50);

  if (selsrc.execute() != 0)
  {
    logfile.WriteEx("failed.\n%s\n%s\n",selsrc.m_sql,selsrc.cda.message); return FALSE;
  }
  
  UINT ii=0;
  char strtemp[11],strBindStr[4096];

  memset(strBindStr,0,sizeof(strBindStr));

  for (ii=0; ii<uMaxCount; ii++)
  {
    memset(strtemp,0,sizeof(strtemp));
    if (ii==0) sprintf(strtemp,":%lu",ii+1);
    if (ii >0) sprintf(strtemp,",:%lu",ii+1);
    strcat(strBindStr,strtemp);
  }

  insstmt.connect(&conn);
  insstmt.prepare("\
    BEGIN\
      delete from %s where %s in (%s);\
      insert into %s(%s) select %s from %s where %s in (%s);\
    END;",strdsttname,strdstpkfieldname,strBindStr,strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname,strBindStr);\
  for (ii=0; ii<uMaxCount; ii++)
  {
    insstmt.bindin(ii+1, pkfieldvaluen[ii],50);
  }

  while (TRUE)
  {
    memset(pkfieldvalue,0,sizeof(pkfieldvalue));

    if (selsrc.next() != 0) break;

    strncpy(pkfieldvaluen[ccount],pkfieldvalue,50);

    ccount++;

    // 每uMaxCount条记录就插入一次
    if (ccount == uMaxCount)
    {
      if (insstmt.execute() != 0)
      {
        logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

        if (insstmt.cda.rc != 1) return FALSE;
      }

      ProgramActive.WriteToFile();

      conn.commitwork();

      memset(pkfieldvaluen,0,sizeof(pkfieldvaluen));

      ccount=0;
    }
  }

  // 在以上循环处理的时候，如果不足uMaxCount，就在这里处理
  for (ii=0; ii<ccount; ii++)
  {
    insstmt.prepare("\
      BEGIN\
        delete from %s where %s=:1;\
        insert into %s(%s) select %s from %s where %s=:2;\
      END;",strdsttname,strdstpkfieldname,strdsttname,strdstfields,strsrcfields,strsrctname,strsrcpkfieldname);
    insstmt.bindin(1, pkfieldvaluen[ii],50);
    insstmt.bindin(2, pkfieldvaluen[ii],50);
    if (insstmt.execute() != 0)
    {
      logfile.WriteEx("failed.\n%s\n%s\n",insstmt.m_sql,insstmt.cda.message); 

      if (insstmt.cda.rc != 1) return FALSE;
    }

    conn.commitwork();
  }

  logfile.WriteEx("ok,%lu rows updated(%d).\n",selsrc.cda.rpc,Timer.Elapsed());

  return TRUE;
}


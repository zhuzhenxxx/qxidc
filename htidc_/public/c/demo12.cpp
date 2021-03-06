//
// 本程序演示采用CTcpServer类，实现socket通讯的服务端
// 

#include "_public.h"

int main(int argc,char *argv[])
{
  if (argc != 2)
  {
    printf("\n");
    printf("Using:./demo12 port\n\n");

    printf("Example:./demo12 5010\n\n");

    printf("本程序演示采用CTcpServer类，实现socket通讯的服务端。\n\n");

    return -1;
  }

  CTcpServer TcpServer;

  // 服务端初始化
  if (TcpServer.InitServer(atoi(argv[1])) == false)
  {
    printf("TcpServer.InitServer(%s) failed.\n",argv[1]); return -1;
  }

  // 等待客户端的连接
  if (TcpServer.Accept() == false)
  {
    printf("TcpServer.Accept() failed.\n"); return -1;
  }

  printf("client ip is %s\n",TcpServer.GetIP());

  char strRecvBuffer[1024],strSendBuffer[1024];

  memset(strRecvBuffer,0,sizeof(strRecvBuffer));

  // 读取客户端的报文，等时间是10秒
  //if (TcpServer.Read(strRecvBuffer,10)==false) 
  if (TcpServer.Read(strRecvBuffer)==false) 
  {
    if (TcpServer.m_btimeout==true) printf("timeout\n");
    printf("TcpServer.Read() failed.\n"); return -1;
  }

  printf("recv ok:%s\n",strRecvBuffer);

  memset(strSendBuffer,0,sizeof(strSendBuffer));
  strcpy(strSendBuffer,"ok");

  // 向客户端返回响应内容
  if (TcpServer.Write(strSendBuffer)==false) 
  {
    printf("TcpServer.Write() failed.\n"); return -1;
  }

  printf("send ok:%s\n",strSendBuffer);

  return 0;
}


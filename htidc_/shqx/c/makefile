#########################################
# ��ע�⣬����ʱ��ȷ�����¼��㣺
# 1���Ѱ�װoracle�Ŀͻ��˻�oracle����˳��򣬲��ұ��밲װ��OCI������
# 2�����ú�ORACLE_HOME��������
#
#########################################

# oracleͷ�ļ�·��
ORAINCL = -I$(ORACLE_HOME)/rdbms/public

# oracle���ļ�·��
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle��oci��
ORALIBS = -lclntsh

# ͨ�ÿ��ͷ�ļ�·��
PUBINCL = -I/root/htidc/public/c

# ͨ�ÿ�ܿ��ļ�·��
PUBCPP = /root/htidc/public/c/_public.cpp 
FTPCPP = /root/htidc/public/c/_ftp.cpp 
OCICPP = /root/htidc/public/c/_ooci.cpp

CC=g++

# CFLAGS = -O2
# CFLAGS = -O2 -Wall

CFLAGS = -g -Wno-write-strings -Wno-unused-variable


all: crtsurfdata crtsurfdata1 psurfdata_old psurfdata psurfdata1 psignallog

crtsurfdata:crtsurfdata.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o crtsurfdata crtsurfdata.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp crtsurfdata ../bin/.

crtsurfdata1:crtsurfdata1.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o crtsurfdata1 crtsurfdata1.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp crtsurfdata1 ../bin/.

psurfdata_old: psurfdata_old.cpp 
	$(CC) $(CFLAGS) -o psurfdata_old psurfdata_old.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) -lm -lc
	cp psurfdata_old ../bin/.

psurfdata: psurfdata.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o psurfdata psurfdata.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp psurfdata ../bin/.

psurfdata1: psurfdata1.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o psurfdata1 psurfdata1.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp psurfdata1 ../bin/.

psignallog: psignallog.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o psignallog psignallog.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc
	cp psignallog ../bin/.

tt: tt.cpp _shqx.h _shqx.cpp
	$(CC) $(CFLAGS) -o tt tt.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) $(PUBINCL) $(PUBCPP) $(OCICPP) _shqx.cpp -lm -lc


clean:
	rm -rf crtsurfdata crtsurfdata1 psurfdata_old psurfdata psurfdata1 psignallog
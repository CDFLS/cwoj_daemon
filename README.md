# ���� OJ �����ػ�����
������Ϊ���� OJ �����ػ����򡣳��� OJ ��ҳ����[����](https://github.com/CDFLS/CWOJ)��

## �����
`apt install libmysqlclient-dev libmicrohttpd-dev libboost-all-dev`

## �����ļ�
�뽫�����ļ������ `/etc/cwojconfig.ini`��
�����ļ�˵����
```ini
[system]
datadir=�������ݴ��Ŀ¼
TempDir=�ڴ���Ŀ¼���� IO ����ʹ�ã��������Ҫ���Բ�����ʵ�Ⲣû��ʲô���ã�
DATABASE_USER=���ݿ��û���
DATABASE_PASS=���ݿ�����
DATABASE_NAME=���ݿ���
```

## ��װ
���ڱ��������Լ�����ʹ�� GNU Autotools ������û��д `make install`��
�ָ�����װ�ű��Թ��ο���Ubuntu 16.04����

```sh
#!/bin/bash
# ���� root Ȩ��ִ��
cp /bin/* /usr/bin/
cp cwojconfig.ini /etc/
cp cwojdaemon.service /etc/systemd/system/
systemctl daemon-reload
```

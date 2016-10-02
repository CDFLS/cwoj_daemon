# 成外 OJ 评测守护程序
本程序为成外 OJ 评测守护程序。成外 OJ 网页端在[这里](https://github.com/CDFLS/CWOJ)。

**警告** 当前守护程序会以 root 权限运行。以后将加入更多的安全设置，并会增强安全措施。

## 所需库
`apt install libmysqlclient-dev libmicrohttpd-dev libboost-all-dev`

## 配置文件
安装完成后，配置文件将被复制到 `/etc/cwojconfig.ini`。
配置文件说明：
```ini
[system]
datadir=评测数据存放目录
TempDir=内存盘目录（供 IO 加速使用，如果不需要可以不填此项，实测并没有什么卵用）
DATABASE_USER=数据库用户名
DATABASE_PASS=数据库密码
DATABASE_NAME=数据库名
```

## ~~安装~~ 构建和安装
我们的构建系统已经更新，我们已经从GNU Autotools迁移到CMake。  
现在，请在项目根目录使用以下指令进行构建和安装（请注意，在目前情况下，**后两个指令需要超级用户权限**）：  
```sh
mkdir build
cd build
cmake ..
make
make install
systemctl daemon-reload
```

~~由于本人懒惰以及不会使用 GNU Autotools ，于是没有写 `make install`。~~
~~现给出安装脚本以供参考（Ubuntu 16.04）：~~  
**以下是旧指令，请不要使用以下指令进行安装。**
```sh
#!/bin/bash
# 请以 root 权限执行
cp /bin/* /usr/bin/
cp cwojconfig.ini /etc/
cp cwojdaemon.service /etc/systemd/system/
systemctl daemon-reload
```

## 运行
在安装后，可直接以系统服务方式运行 cwoj：
```sh
# 启用服务
systemctl enable cwojdaemon
# 运行服务
systemctl start cwojdaemon
```

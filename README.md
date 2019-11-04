### http post解析器

------

DPI解析 http post 请求之 multipart/form-data。



### 一. 编译

#### 1.1 第三方库

- librdkafka

  kafka客户端库  [下载地址](https://github.com/edenhill/librdkafka/archive/v1.1.0.tar.gz "kafka")

- libpcap

  libpcap采集库  [下载地址](https://www.tcpdump.org/release/libpcap-1.9.0.tar.gz "libpcap")

- PF_RING-7.2.0     

  PF_RING高性能采集库  [下载地址](https://github.com/ntop/PF_RING/archive/7.2.0.tar.gz "pfring")



#### 1.2 编译

```shell
# 编译libpcap
tar -xvf libpcap-1.9.0.tar.gz
cd libpcap-1.9.0
./configure --prefix=/usr/local/libpcap
make && make install 

# 编译kafka 
tar -xvf librdkafka-1.1.0.tar.gz
cd librdkafka-1.1.0
./configure --prefix=/usr/local/kafka
make && make install 

# 编译主程序
wget https://github.com/spkettas/httppost/archive/master.zip
unzip master.zip
cd master
make

#pfring版本
make -f makefile_pf
```



### 二. 模块说明

- 流量采集模块

默认采用`libpcap`原始套接字抓包，流量较大时，可考虑PF_RING，DPDK 高性能包采集库。

- http post解析模块

http post上传文件时，由于包体较大，需要分多次发送。程序需要建立`流表`保存上下文关系，`累积`多个数据包数据，才能重组成完成的body。

[multipart/form-data 请求说明](https://my.oschina.net/cnlw/blog/168466 "multipart")

- LRU算法

存在一些不完整的http post数据包，导致缓存区大量累积无法组包成功的数据，程序侧定时通过LRU算法清除老化的流表。

> 注：可解析TCP FIN包，主动断开流表。

- 内存池

流量过大时，频繁new/delete会产生大量的内存碎片，通过内存池方式加速内存分配。本文实现较为简单，直接切分大块内存，定长分配，对于变长数据包，会造成内存浪费。

> 注：可参考 nginx slab 内存池优化。



### 三. 启动

* 目录说明
httppost
  -- bin  可执行文件及启动脚本
  -- lib   依赖第三方库
  -- conf  配置文件路径 
  -- log   日志路径
  -- test  单元测试目录



* 配置说明

httppost/conf/sys.cfg

```shell
daemon=0		    # 是否后台
loglevel=4      # 日志等级，4debug 3info 2warn 1error，数字越小，日志越少
dev=eth1|eth2   # 采集网卡，多网卡以|分隔
port=80			    # 监听目标协议端口  		
poolsize=200000	# 连接池节点数目

kafka_host=192.168.10.101:90921    # kafka服务器地址列表
dest_topic=flow_topic              # kafka topic
```



* 启动

```shell
cd  httppost/bin
./service.sh start
```


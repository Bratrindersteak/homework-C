# 基于socket的文件传输

- Linux基本命令学习
    - df
    - du
    - scp
    - mkdir
    - rm
    - ls
    - chmod

- 要求
  - Ubuntu主机作为server发送文件
  - ArchLinux虚拟机作为client接收文件并存储在本地
  - 传输要求：
    - server先发出发送请求filename，size
    - client再响应，回复允许
    - server再发送真正文件
    - client回复收到，socket断开
  - 文件传输格式：
    - 整形
    - 二进制
    - 4字节


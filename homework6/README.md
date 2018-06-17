# Raw Socket任务要求

> 场景描述：
一个分布式的工作组集群，其中有一个服务器，其他的全部是成员。
服务器首先启动（命令：grp_server <grp_ID>, 其中grp_ID为一个十六进制数），
成员随后按序启动，申请自己的NodeID（命令：grp_client <grp_ID>）
当服务器启动的时候，需要向集群中询问，是否有同样grp_ID的master存在，如果存在，报错退出；否则，自己开始担当服务器角色。
每个成员启动的时候，都需要向集群中询问：谁是服务器；在服务器响应以后，向服务器发送申请NodeID的命令字：

1. 编写两个程序，一个作为服务器，一个作为Client，采用Raw Socket进行通讯
   通讯的内容格式：
   struct DATA_PROTO
   {
     unsigned int dwGroupID;      // 为命令启动时输出的参数 grp_ID
     unsigned int dwRequestTime;  // 数据包发送时距离1970年1月1日零点的秒钟数
     unsigned short wGroupCmd;    // 0x0FF0时，寻找group中已经存在的服务器，此时，该数据包应该为一个广播包，也就是说：LLC中的目的MAC为0xFFFFFFFFFFFF
                                  // 0x0F01时，成员向服务器申请自己合法的NodeID，服务器按顺序发放
                                  // 0x00F0时，为服务器响应成员的数据包，告知自己为group中的服务器
                                  // 0x0001时，为服务器发放给成员的NodeID响应包
     unsigned short wNodeID;
   }
  这些数据传输需要可以通过wireshark捕获
  甚至wireshark可以帮助调试
  
2. 试图将服务器端和成员端的程序合成为一个程序，在程序启动的时候，增加一个参数，标明自己是服务器还是成员.

//
// 会话使用说明：
//   每个会话中，在HDR_FORMAT中含有APP_ID和USID
//   其中...
//     APP_ID标识了会话所属的应用，便于之后的应用线程的创建
//     USID标识了会话在对端的会话表中的记录位置，便于快速的数据包分发
//
// 这个是会话建立的服务器端
// 客户端是FuncSessionInitC
//
int FuncSessionInitS(unsigned short wClientNodeID, unsigned int dwClientMACInt, unsigned short wClientMACShort, int dwBufferSize, unsigned short dwAppID)
{
  int ii;
  struct RECV_FRAME_UNIT **ptrBuf;
  unsigned short wVerifyFlag;
  // for sending frame
  unsigned short dwPacketFlag;
  struct SEND_FRAME_UNIT *ptrSendFrame;
  struct LLC_FORMAT *pLLC;
  struct HDR_FORMAT *pHdr;
  struct SESSION_CMD *pSessionCmd;

  //
  // apply the recv buffer
  //
  ptrBuf = (struct RECV_FRAME_UNIT **)malloc(dwBufferSize*sizeof(struct RECV_FRAME_UNIT **));
  if (ptrBuf == NULL)
  {
    return -1;
  }

  //
  // 在会话表中添加记录
  //
  srand((int)time(0));
  wVerifyFlag = rand() % 65536;
  ii = 0;
  while (ptrSessions[ii].dwDestSessionIndex != 0xFFFF)
  {
    ii++;
  }
  ptrSessions[ii].dwDestSessionIndex = wVerifyFlag;     // 目前还不知道, 所以暂定一个随机数
  ptrSessions[ii].wDestNodeID = wClientNodeID;
  ptrSessions[ii].dwDestMACInt = dwClientMACInt;
  ptrSessions[ii].wDestMACShort = wClientMACShort;
  sem_init(&(ptrSessions[ii].semSession), 0, 0);
  ptrSessions[ii].ptrRecvBuffer = ptrBuf;
  ptrSessions[ii].dwRecvBufferSize = dwBufferSize;
  ptrSessions[ii].dwRecvEnd = 0;
  ptrSessions[ii].dwRecvBegin = 0;
  ptrSessions[ii].dwRecvedCounter = 0;

  ptrSendFrame = FuncGetSendBuf(0);
  pLLC = (struct LLC_FORMAT *)(&(ptrSendFrame->ptrFrame));
  pHdr = (struct HDR_FORMAT *)(&(pLLC->pData));
  pSessionCmd = (struct SESSION_CMD *)(&(pHdr->pData));
  pLLC->SourceInt = LocalMACInt;
  pLLC->SourceShort = LocalMACShort;
  pLLC->DestInt = dwClientMACInt;
  pLLC->DestShort = wClientMACShort;
  pLLC->wFrameType = REDFISH_PROTO;
  pHdr->wNodeID = wClientNodeID;       // to NodeID
  pHdr->wFlag1 = ((unsigned short)(dwAppID<<3) | 0x0);
  dwPacketFlag = SESSION_CLIENT;        // Session Init
  pHdr->dwFlag2 = ((unsigned short)dwPacketFlag | 0x3FFF);
  pHdr->wDataLen = sizeof(struct HDR_FORMAT) + sizeof(struct SESSION_CMD);
  pSessionCmd->wVerifyFlag = wVerifyFlag;
  pSessionCmd->wServerSessionIndex = ii;
  pSessionCmd->wServerNodeID = dwNodeID;
  pSessionCmd->dwServerMACInt = LocalMACInt;
  pSessionCmd->wServerMACShort = LocalMACShort;
  pSessionCmd->wClientNodeID = wClientNodeID;
  pSessionCmd->wClientSessionIndex = 0xFFFF;
  pSessionCmd->dwClientMACInt = dwClientMACInt;
  pSessionCmd->wClientMACShort = wClientMACShort;
  //
  ptrSendFrame->dwLen = 74;
  sem_post(&semSend);

  return 1;
}

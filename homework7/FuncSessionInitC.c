//
// 这个是会话建立的客户端
// 服务器端是FuncSessionInitS
//
int FuncSessionInitC(unsigned short wServerNodeID, unsigned int dwServerMACInt, unsigned short wServerMACShort, short wServerSessionIndex, unsigned short wVerifyFlag, int dwBufferSize, unsigned short dwAppID, short *wSessionIndex)
{
  int ii;
  struct RECV_FRAME_UNIT **ptrBuf;
  unsigned short wTempIndex;
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
  ii = 0;
  while (ptrSessions[ii].dwDestSessionIndex != 0xFFFF)
  {
    ii++;
  }
  ptrSessions[ii].dwDestSessionIndex = wServerSessionIndex;
  ptrSessions[ii].wDestNodeID = wServerNodeID;
  ptrSessions[ii].dwDestMACInt = dwServerMACInt;
  ptrSessions[ii].wDestMACShort = wServerMACShort;
  sem_init(&(ptrSessions[ii].semSession), 0, 0);
  ptrSessions[ii].ptrRecvBuffer = ptrBuf;
  ptrSessions[ii].dwRecvBufferSize = dwBufferSize;
  ptrSessions[ii].dwRecvEnd = 0;
  ptrSessions[ii].dwRecvBegin = 0;
  ptrSessions[ii].dwRecvedCounter = 0;
  //
  *wSessionIndex = ii;

  ptrSendFrame = FuncGetSendBuf(0);
  pLLC = (struct LLC_FORMAT *)(&(ptrSendFrame->ptrFrame));
  pHdr = (struct HDR_FORMAT *)(&(pLLC->pData));
  pSessionCmd = (struct SESSION_CMD *)(&(pHdr->pData));
  pLLC->SourceInt = LocalMACInt;
  pLLC->SourceShort = LocalMACShort;
  pLLC->DestInt = dwServerMACInt;
  pLLC->DestShort = wServerMACShort;
  pLLC->wFrameType = REDFISH_PROTO;
  pHdr->wNodeID = wServerNodeID;       // to NodeID
  pHdr->wFlag1 = ((unsigned short)(dwAppID<<3) | 0x0);
  dwPacketFlag = SESSION_SERVER;        // Session Init
  pHdr->dwFlag2 = ((unsigned short)dwPacketFlag | wServerSessionIndex);
  pHdr->wDataLen = sizeof(struct HDR_FORMAT) + sizeof(struct SESSION_CMD);
  pSessionCmd->wVerifyFlag = wVerifyFlag;
  pSessionCmd->wServerSessionIndex = wServerSessionIndex;
  pSessionCmd->wServerNodeID = wServerNodeID;
  pSessionCmd->dwServerMACInt = dwServerMACInt;
  pSessionCmd->wServerMACShort = wServerMACShort;
  pSessionCmd->wClientNodeID = dwNodeID;
  pSessionCmd->wClientSessionIndex = ii;
  pSessionCmd->dwClientMACInt = LocalMACInt;
  pSessionCmd->wClientMACShort = LocalMACShort;
  //
  ptrSendFrame->dwLen = 74;
  sem_post(&semSend);

  return 1;
}

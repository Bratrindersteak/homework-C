//
// the thread of the network sending
//
void *FuncThreadSend(void *dwCoreID)
{
  int ii, jj;
  int dwID;
  int dwRet;
  int dwSendBufSize;
  socklen_t dwOptLen;
  struct LLC_FORMAT pSend;
  struct SEND_FRAME_UNIT *ptrSendBuf;
  //unsigned char buf[10240];
  socklen_t dwSocketOptSsize;

  //
  // to bundle the thread with core-0
  //
  dwID = *(int *)dwCoreID;
  if (set_cpu(dwID) != 1)
  {
    if (DB_DEBUG)
    {
      printf("ERROR : set CPU error for thread of sending !!!\n");
    }
    return NULL;
  }

  //
  // set the send timeout for send socket
  //
  socketSend = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (socketSend == -1)
  {
    printf("ERROR : Create the send socket error !!!\n");
    return NULL;
  }
  setsockopt(socketSend, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tvNetTimeout, sizeof(tvNetTimeout));
  //dwRet = getsockopt(socketSend, SOL_SOCKET, SO_SNDBUF, &snd_buf_size, &opt_size);

  //
  // set the send buffer size for the send socket
  //
  dwOptLen = sizeof(dwSendBufSize);
  dwRet = getsockopt(socketSend, SOL_SOCKET, SO_SNDBUF, &dwSendBufSize, &dwOptLen);
  if (dwRet < 0)
  {
    printf("ERROR : Can NOT get the send buffer size of the socket !!!\n");
  }
  else
  {
    printf("INFO : wmem_size = %d\n", dwSendBufSize);
  }

  //
  // to fill the sending device struct
  //
  bzero(&devSend, sizeof(devSend));
  devSend.sll_family = AF_PACKET;
  memcpy(devSend.sll_addr, pLocalMAC, 6);
  devSend.sll_halen = htons(6);
  devSend.sll_ifindex = dwLocalIfIndex;        // This need to be CAUSED !!!
//printf("d : ifindex = %d\n", dwLocalIfIndex);

  //
  // do the data sending
  //
  while (!dwExitNetwork)
  {
    sem_wait(&semSend);

    // 轮训所有的发送优先级队列
    jj = 0;
    while (pSendQueue[jj].dwFrameCounter == 0)
    {
      jj++;
    }
    ptrSendBuf = (struct SEND_FRAME_UNIT *)((char *)(pSendQueue[jj].ptrSend)+pSendQueue[jj].dwSendBegin*(dwSendLFrameSize[jj]+sizeof(int)));
    sendto(socketSend, (void *)(&(ptrSendBuf->ptrFrame)), ptrSendBuf->dwLen, 0, (struct sockaddr *)(&devSend), sizeof(devSend));
    // to adjust the buffer pointer of the frames needed to be sent
    pSendQueue[jj].dwFrameCounter--;
    pSendQueue[jj].dwSendBegin++;
    pSendQueue[jj].dwSendBegin = pSendQueue[jj].dwSendBegin % dwSendLCounter[jj];
  }

  close(socketSend);

  return NULL;
}

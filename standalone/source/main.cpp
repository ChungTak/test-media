#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MediaServer.h"
#include "hv/TcpServer.h"
#include "rtmp-server.h"

using namespace hv;

int main(int argc, char* argv[]) {
  printf("main\n");

  int port = 1935;

  TcpServer srv;
  int listenfd = srv.createsocket(port);
  if (listenfd < 0) {
    return -20;
  }

  printf("server listen on port %d, listenfd=%d ...\n", port, listenfd);
  srv.onConnection = [](const SocketChannelPtr& channel) {
    std::string peeraddr = channel->peeraddr();
    if (channel->isConnected()) {
      NodeRtmpServer nodeRtmpServer(channel);
      nodeRtmpServer.start();
      channel->setContext(&nodeRtmpServer);

      printf("%s connected! connfd=%d\n", peeraddr.c_str(), channel->fd());
    } else {
      printf("%s disconnected! connfd=%d\n", peeraddr.c_str(), channel->fd());
    }
  };
  srv.onMessage = [](const SocketChannelPtr& channel, Buffer* buf) {
    // echo
    // printf("< %d\n", (int)buf->size());
    // Buffer buffer(buf->data(), buf->size());
    // rtmpChannelList.emplace_back(channel);
    NodeRtmpServer* nodeRtmpServer = channel->getContext<NodeRtmpServer>();
    rtmp_server_input(nodeRtmpServer->rtmp, (uint8_t*)buf->data(), buf->size());
    // channel->write(buf);
  };
  srv.onWriteComplete = [](const SocketChannelPtr& channel, Buffer* buf) {
    // printf("> %.*s\n", (int)buf->size(), (char*)buf->data());
  };
  srv.setThreadNum(1);
  srv.start();
  printf("start\n");

  while (1) hv_sleep(1);
  printf("endend\n");

  return 0;
}
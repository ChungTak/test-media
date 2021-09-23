#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hv/TcpServer.h"
#include "rtmp-server.h"

using namespace hv;

static std::vector<SocketChannelPtr> rtmpChannelList;

static int rtmp_server_send(void* param, const void* header, size_t len, const void* data,
                            size_t bytes) {
  printf("rtmp_server_send \n");
  SocketChannelPtr channel = rtmpChannelList[0];
  if (len > 0) {
    channel.get()->write(header, len);
  }
  channel.get()->write(data, bytes);

  return len + bytes;
}

static int rtmp_server_onpublish(void* param, const char* app, const char* stream,
                                 const char* type) {
  printf("rtmp_server_onpublish(%s, %s, %s)\n", app, stream, type);
  return 0;
}

static int rtmp_server_onscript(void* param, const void* script, size_t bytes, uint32_t timestamp) {
  printf("[S] timestamp: %u\n", timestamp);
  return 0;
}

static int rtmp_server_onvideo(void* param, const void* data, size_t bytes, uint32_t timestamp) {
  printf("[V] timestamp: %u\n", timestamp);
  return 0;
}

static int rtmp_server_onaudio(void* param, const void* data, size_t bytes, uint32_t timestamp) {
  printf("[A] timestamp: %u\n", timestamp);
  return 0;
}

int main(int argc, char* argv[]) {
  printf("main\n");

  static uint8_t packet[8 * 1024 * 1024];

  struct rtmp_server_handler_t handler;
  memset(&handler, 0, sizeof(handler));
  handler.send = rtmp_server_send;
  handler.onpublish = rtmp_server_onpublish;
  handler.onaudio = rtmp_server_onaudio;
  handler.onvideo = rtmp_server_onvideo;
  handler.onscript = rtmp_server_onscript;
  rtmp_server_t* rtmp = rtmp_server_create(NULL, &handler);

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
      printf("%s connected! connfd=%d\n", peeraddr.c_str(), channel->fd());
    } else {
      printf("%s disconnected! connfd=%d\n", peeraddr.c_str(), channel->fd());
    }
  };
  srv.onMessage = [&rtmp](const SocketChannelPtr& channel, Buffer* buf) {
    // echo
    // printf("< %d\n", (int)buf->size());
    rtmpChannelList.push_back(channel);
    rtmp_server_input(rtmp, (uint8_t*)buf->data(), buf->size());

    // channel->write(buf);
  };
  srv.onWriteComplete = [](const SocketChannelPtr& channel, Buffer* buf) {
    // printf("> %.*s\n", (int)buf->size(), (char*)buf->data());
  };
  srv.setThreadNum(4);
  srv.start();
  printf("start\n");

  while (1) hv_sleep(1);
  printf("endend\n");

  rtmp_server_destroy(rtmp);
  return 0;
}
#include <greeter/greeter.h>
#include <greeter/version.h>
#include <rtmp/rtmpsession.h>

#include <iostream>

#include "hv/TcpServer.h"
using namespace hv;

auto main(int argc, char** argv) -> int {
  std::cout << "hello rtmp server!" << std::endl;
  std::vector<RtmpSessionPtr> rtmpSessionList;

  struct rtmp_server_handler_t handler;
  handler.send = rtmp_handler_send;
  handler.onplay = rtmp_handler_onplay;
  handler.onseek = rtmp_handler_onseek;
  handler.onpause = rtmp_handler_onpause;
  handler.onaudio = rtmp_handler_onaudio;
  handler.onvideo = rtmp_handler_onvideo;
  handler.onscript = rtmp_handler_onscript;
  handler.onpublish = rtmp_handler_onpublish;
  handler.ongetduration = rtmp_handler_ongetduration;
  int count = 0;
  std::string addr = "0.0.0.0:1935";
  int thread_num = 4;
  int port = 1935;

  TcpServer srv;
  int listenfd = srv.createsocket(port);
  if (listenfd < 0) {
    return -20;
  }
  printf("server listen on port %d, listenfd=%d ...\n", port, listenfd);
  srv.onConnection = [&handler, &rtmpSessionList, &count](const SocketChannelPtr& channel) {
    std::string peeraddr = channel->peeraddr();
    if (channel->isConnected()) {
      printf("%s connected! connfd=%d\n", peeraddr.c_str(), channel->fd());
      RtmpSessionPtr clientPtr(new RtmpSession(channel, rtmpSessionList));
      rtmp_server_t* handle = rtmp_server_create(clientPtr.get(), &handler);
      clientPtr->SetRtmpHandle(handle);
      //  std::shared_ptr<RtmpSession> clientPtr(&client);

      channel->setContext(&clientPtr);
      if (count > 0) {
        rtmpSessionList.push_back(RtmpSessionPtr(clientPtr));
      }

      count++;
    } else {
      printf("%s disconnected! connfd=%d\n", peeraddr.c_str(), channel->fd());
    }
  };
  srv.onMessage = [](const SocketChannelPtr& channel, Buffer* buf) {
    // echo
    printf("< %.*s\n", (int)buf->size(), (char*)buf->data());

    RtmpSessionPtr* ptr = channel->getContext<RtmpSessionPtr>();

    ptr->get()->OnReceive(channel, buf);
  };

  srv.setThreadNum(4);
  srv.start();

  while (1) hv_sleep(1);
  return 0;
}

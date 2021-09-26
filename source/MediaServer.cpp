#include "MediaServer.h"

#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "flv-proto.h"
#include "flv-writer.h"
#define PORT 1935
#include <arpa/inet.h>
#include <sys/types.h>

#include <cstring>  // sizeof()
#include <iostream>
NodeRtmpServer::NodeRtmpServer(SocketChannelPtr channel) {
  this->channel = channel;
  flvDemuxer = flv_demuxer_create(NodeRtmpServer::flv_demuxer_handler, this);
  flvMuxer = flv_muxer_create(NodeRtmpServer::flv_muxer_handler, this);
}

NodeRtmpServer::~NodeRtmpServer() {
  printf("NodeRtmpServer Destory\n");

  if (rtmp != nullptr) {
    rtmp_server_destroy(rtmp);
  }
  if (flvDemuxer != nullptr) {
    flv_demuxer_destroy(flvDemuxer);
  }
  if (flvMuxer != nullptr) {
    flv_muxer_destroy(flvMuxer);
  }
}

int NodeRtmpServer::flv_demuxer_handler(void *param, int codec, const void *data, size_t bytes,
                                        uint32_t pts, uint32_t dts, int flags) {
  NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;
  return 0;
}

int NodeRtmpServer::flv_muxer_handler(void *param, int type, const void *data, size_t bytes,
                                      uint32_t timestamp) {
  printf("flv_muxer_handler\n");
  NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;
  // Napi::Env env = nodeRtmpServer->emit.Env();
  rtmp_server_t *rtmp = nodeRtmpServer->rtmp;
  switch (type) {
    case FLV_TYPE_SCRIPT:
      printf("FLV_TYPE_SCRIPT\n");
      return rtmp_server_send_script(rtmp, data, bytes, timestamp);
    case FLV_TYPE_AUDIO:
      printf("FLV_TYPE_AUDIO\n");
      return rtmp_server_send_audio(rtmp, data, bytes, timestamp);
    case FLV_TYPE_VIDEO:
      printf("FLV_TYPE_VIDEO\n");
      return rtmp_server_send_video(rtmp, data, bytes, timestamp);
    default:
      printf("default\n");

      assert(0);
      return -1;
  }
  return 0;
}

int NodeRtmpServer::start() {
  printf("NodeRtmpServer start \n");
  struct rtmp_server_handler_t handler;
  memset(&handler, 0, sizeof(handler));
  handler.send
      = [](void *param, const void *header, size_t len, const void *payload, size_t bytes) {
          printf("rtmp_handler_send len = %d   bytes = %d\n", len, bytes);

          NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;

          return 0;
        };
  handler.onpublish = [](void *param, const char *app, const char *stream, const char *type) {
    printf("rtmp_server_onpublish(%s, %s, %s)\n", app, stream,
           type);  // rtmp_server_onpublish(live, livestream, live)
    NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;

    return 0;
  };
  handler.onplay = [](void *param, const char *app, const char *stream, double start,
                      double duration, uint8_t reset) {
    printf("rtmp_server_onplay(%s, %s, %f, %f, %d)\n", app, stream, start, duration, (int)reset);

    NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;

    return 0;
  };

  handler.onscript = [](void *param, const void *data, size_t bytes, uint32_t timestamp) {
    printf("onscript\n");
    NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;
    // Napi::Env env = nodeRtmpServer->emit.Env();
    // nodeRtmpServer->emit.Call(nodeRtmpServer->recv, {Napi::String::New(env, "onscript")});
    flv_demuxer_input(nodeRtmpServer->flvDemuxer, FLV_TYPE_SCRIPT, data, bytes, timestamp);

    return 0;
  };
  handler.onvideo = [](void *param, const void *data, size_t bytes, uint32_t timestamp) {
    printf("onvideo\n");
    NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;
    // Napi::Env env = nodeRtmpServer->emit.Env();
    flv_demuxer_input(nodeRtmpServer->flvDemuxer, FLV_TYPE_VIDEO, data, bytes, timestamp);
    // Napi::Buffer<uint8_t> buf = Napi::Buffer<uint8_t>::Copy(env, (uint8_t *)data, bytes);

    // nodeRtmpServer->emit.Call(nodeRtmpServer->recv, {Napi::String::New(env, "onvideo"), buf});

    return 0;
  };
  handler.onaudio = [](void *param, const void *data, size_t bytes, uint32_t timestamp) {
    printf("onaudio\n");

    // Napi::Env env = nodeRtmpServer->emit.Env();
    NodeRtmpServer *nodeRtmpServer = (NodeRtmpServer *)param;

    // nodeRtmpServer->emit.Call(nodeRtmpServer->recv, {Napi::String::New(env, "onaudio")});
    flv_demuxer_input(nodeRtmpServer->flvDemuxer, FLV_TYPE_AUDIO, data, bytes, timestamp);

    return 0;
  };

  printf("NodeRtmpServer callback init done \n");

  // s_flv = flv_writer_create("h264.flv");
  printf("NodeRtmpServer flv_writer_create init done \n");
  this->rtmp = rtmp_server_create(this, &handler);

  //   char buffer[2 * 1024 * 1024] = {0};
  //   // int valread = read(this->socket, buffer, 1024);
  //   int valread;
  //   while (valread = (read(this->socket, buffer, 1024)) > 0) {
  //     printf("%s\n", buffer);

  //     // assert(0 == rtmp_server_input(s_rtmp, packet, r));
  //   }
  //   printf("%d\n", valread);
  // flv_writer_destroy(s_flv);
  return 0;
}

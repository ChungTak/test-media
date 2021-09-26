#ifndef NODERTMPSERVER_H
#define NODERTMPSERVER_H

#include "flv-demuxer.h"
#include "flv-muxer.h"
#include "hv/TcpServer.h"
#include "rtmp-server.h"

using namespace hv;
class NodeRtmpServer {
public:
  NodeRtmpServer(SocketChannelPtr channel);
  ~NodeRtmpServer();
  static int flv_demuxer_handler(void *param, int codec, const void *data, size_t bytes,
                                 uint32_t pts, uint32_t dts, int flags);
  static int flv_muxer_handler(void *param, int type, const void *data, size_t bytes,
                               uint32_t timestamp);
  rtmp_server_t *rtmp;
  struct flv_demuxer_t *flvDemuxer;
  struct flv_muxer_t *flvMuxer;

  int start();
  int id = 0;
  SocketChannelPtr channel;
};

#endif
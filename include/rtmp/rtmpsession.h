#pragma once
#include <memory>
#include <vector>

#include "flv-demuxer.h"
#include "flv-muxer.h"
#include "flv-proto.h"
#include "flv-writer.h"
#include "hv/TcpServer.h"
#include "rtmp-server.h"
using namespace hv;

enum RS_Status {
  RS_INIT,
  RS_SIGNALLING,
  RS_PAUSE,
  RS_SENDSTREAM,

  RS_ERROR
};

class RtmpSession {
  unsigned int mFrameSeq;
  unsigned long mLastFrameTS;

  int SendStream();
  int RecvSignaling();

public:
  // int mSocket;
  RS_Status mStatus;
  rtmp_server_t *mRtmpHandle;
  SocketChannelPtr mConn;
  flv_muxer_t *mPacker;
  flv_demuxer_t *flvDemuxer;
  std::vector<std::shared_ptr<RtmpSession>> *_rtmpSessionList;
  static int muxHandler(void *param, int type, const void *data, size_t bytes, uint32_t timestamp);
  static int demuxHandler(void *param, int codec, const void *data, size_t bytes, uint32_t pts,
                          uint32_t dts, int flags);

  RtmpSession(const SocketChannelPtr &conn,
              std::vector<std::shared_ptr<RtmpSession>> &rtmpSessionList);
  ~RtmpSession();

  void SetRtmpHandle(rtmp_server_t *handle) { mRtmpHandle = handle; }
  static int ReadSourceStream(char *fileName);
  void OnReceive(const SocketChannelPtr &conn, Buffer *buffer);
  int Run();
};
typedef std::shared_ptr<RtmpSession> RtmpSessionPtr;

// using RtmpSessionPtr = std::shared_ptr<RtmpSession>;

int handler(void *param, int type, const void *data, size_t bytes, u_int ts);
int rtmp_handler_send(void *param, const void *header, size_t len, const void *payload,
                      size_t bytes);
int rtmp_handler_onpublish(void *param, const char *app, const char *stream, const char *type);
int rtmp_handler_onscript(void *param, const void *data, size_t bytes, uint32_t timestamp);
int rtmp_handler_onaudio(void *param, const void *data, size_t bytes, uint32_t timestamp);
int rtmp_handler_onvideo(void *param, const void *data, size_t bytes, uint32_t timestamp);
int rtmp_handler_onplay(void *param, const char *app, const char *stream, double start,
                        double duration, uint8_t reset);
int rtmp_handler_onpause(void *param, int pause, uint32_t ms);
int rtmp_handler_onseek(void *param, uint32_t ms);
int rtmp_handler_ongetduration(void *param, const char *app, const char *stream, double *duration);
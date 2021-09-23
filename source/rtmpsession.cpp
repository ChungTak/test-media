#include <rtmp/rtmpsession.h>

#include <string>

char buffer[5 * 1024];
char rtpPkg[1600];
std::vector<char *> StreamFrame;

RtmpSession::RtmpSession(const SocketChannelPtr &conn, std::vector<RtmpSessionPtr> &rtmpSessionList)
    : mConn(conn),
      _rtmpSessionList(&rtmpSessionList),
      mRtmpHandle(NULL),
      mFrameSeq(0),
      mPacker(NULL),
      flvDemuxer(NULL),
      mLastFrameTS(0),
      mStatus(RS_INIT) {
  if (this->flvDemuxer == nullptr) {
    this->flvDemuxer = flv_demuxer_create(RtmpSession::demuxHandler, this);

    printf("flv_demuxer_create \n");
  }
  if (this->mPacker == nullptr) {
    printf("flv_muxer_create1111t \n");
    this->mPacker = flv_muxer_create(RtmpSession::muxHandler, this);
    printf("flv_muxer_createt \n");
  }
}

RtmpSession::~RtmpSession() {
  if (mPacker) flv_muxer_destroy(mPacker);
  if (flvDemuxer) flv_demuxer_destroy(flvDemuxer);

  if (mRtmpHandle) rtmp_server_destroy(mRtmpHandle);

  // closesocket(mSocket);
  printf("close socket \n");
}
void RtmpSession::OnReceive(const SocketChannelPtr &conn, Buffer *buffer) {
  printf("flv_demuxer_create \n");

  // const evpp::Slice bufSlice = buffer->NextAll();
  // buffer->Reset();
  // printf("recv data size:%d\n", bufSlice.size());
  //  uint8_t* buf = reinterpret_cast<uint8_t*>((const_cast<char*>(buffer->data())));
  rtmp_server_input(mRtmpHandle, (uint8_t *)buffer->data(), buffer->size());
}

int RtmpSession::muxHandler(void *param, int type, const void *data, size_t bytes,
                            u_int timestamp) {
  int r = 0;
  RtmpSession *session = (RtmpSession *)param;
  printf("_rtmpSessionList= %d\n", session->_rtmpSessionList->size());

  // for (auto it = session->_rtmpSessionList->begin(); it != session->_rtmpSessionList->end();)
  // {
  //     auto s = (*it);

  //     if (s->mStatus == RS_SENDSTREAM)
  //     {
  //         switch (type)
  //         {
  //         case FLV_TYPE_SCRIPT:
  //             // printf("FLV_TYPE_SCRIPT= %d\n", s->mStatus);
  //             return rtmp_server_send_script(s->mRtmpHandle, data, bytes, timestamp);
  //         case FLV_TYPE_AUDIO:
  //             // printf("FLV_TYPE_AUDIO= %d\n", s->mStatus);
  //             return rtmp_server_send_audio(s->mRtmpHandle, data, bytes, timestamp);
  //         case FLV_TYPE_VIDEO:
  //             // printf("FLV_TYPE_VIDEO= %d\n", s->mStatus);
  //             return rtmp_server_send_video(s->mRtmpHandle, data, bytes, timestamp);
  //         default:
  //             return -1;
  //         }
  //     }
  // }
  switch (type) {
    case FLV_TYPE_SCRIPT:
      // printf("FLV_TYPE_SCRIPT= %d\n", s->mStatus);
      r = rtmp_server_send_script(session->mRtmpHandle, data, bytes, timestamp);
      break;
    case FLV_TYPE_AUDIO:
      // printf("FLV_TYPE_AUDIO= %d\n", s->mStatus);
      r = rtmp_server_send_audio(session->mRtmpHandle, data, bytes, timestamp);
      break;
    case FLV_TYPE_VIDEO:
      printf("rtmp_server_send_video \n");
      r = rtmp_server_send_video(session->mRtmpHandle, data, bytes, timestamp);
      break;
    default:
      r = -1;
  }
  return r;
}

int RtmpSession::demuxHandler(void *param, int codec, const void *data, size_t bytes, uint32_t pts,
                              uint32_t dts, int flags) {
  int r = 0;
  // printf("demuxHandler= %d\n", codec);

  RtmpSession *session = (RtmpSession *)param;

  // switch (codec)
  // {
  // case FLV_VIDEO_H264:
  //     r = flv_muxer_avc(session->mPacker, data, bytes, pts, dts);
  //     break;
  // case FLV_VIDEO_H265:
  //     r = flv_muxer_hevc(session->mPacker, data, bytes, pts, dts);
  //     break;
  // case FLV_AUDIO_AAC:
  //     r = flv_muxer_aac(session->mPacker, data, bytes, pts, dts);
  //     break;
  // case FLV_AUDIO_MP3:
  //     r = flv_muxer_mp3(session->mPacker, data, bytes, pts, dts);
  //     break;

  // case FLV_VIDEO_AVCC:
  // case FLV_VIDEO_HVCC:
  // case FLV_AUDIO_ASC:
  //     break; // ignore

  // default:
  //     return -1;
  // }

  //   printf("session->_rtmpSessionList= %d\n", session->_rtmpSessionList->size());

  for (auto &s : *session->_rtmpSessionList) {
    //  printf("FLV_TYPE_VIDEO= %d\n", s->mStatus);

    if (s->mStatus == RS_SENDSTREAM && s->mPacker != nullptr) {
      switch (codec) {
        case FLV_VIDEO_H264:
          // printf("FLV_TYPE_VIDEO= %d\n", s->mStatus);
          r = flv_muxer_avc(s->mPacker, data, bytes, pts, dts);
          break;
        case FLV_VIDEO_H265:
          r = flv_muxer_hevc(s->mPacker, data, bytes, pts, dts);
          break;
        case FLV_AUDIO_AAC:
          r = flv_muxer_aac(s->mPacker, data, bytes, pts, dts);
          break;
        case FLV_AUDIO_MP3:
          r = flv_muxer_mp3(s->mPacker, data, bytes, pts, dts);
          break;

        case FLV_VIDEO_AVCC:
        case FLV_VIDEO_HVCC:
        case FLV_AUDIO_ASC:
          break;  // ignore

        default:
          printf("FLV_TYPE_VIDEO= %d\n", s->mStatus);
          return -1;
      }
    }
  }
  return r;
}

int rtmp_handler_send(void *param, const void *header, size_t len, const void *payload,
                      size_t bytes) {
  // printf("rtmp_handler_send len = %d   bytes = %d\n", len, bytes);

  // RtmpSessionPtr clientPtr
  RtmpSession *session = (RtmpSession *)param;
  // RtmpSession *session = reinterpret_cast<RtmpSession *>(param);
  session->mConn->write(header, len);
  if (bytes > 0) {
    session->mConn->write(payload, bytes);
  }
  // printf("rtmp_handler_send result = %d\n", len + bytes);

  return len + bytes;

  // int ret = send(session->mSocket, (char *)header, len, 0);
  // ret += send(session->mSocket, (char *)payload, bytes, 0);
  // if (ret != len + bytes)
  //     session->mStatus = RS_ERROR;
  // return ret;
}

int rtmp_handler_onplay(void *param, const char *app, const char *stream, double start,
                        double duration, uint8_t reset) {
  RtmpSession *session = (RtmpSession *)param;

  session->mStatus = RS_SENDSTREAM;
  return 0;
}

int rtmp_handler_onpause(void *param, int pause, uint32_t ms) { return 0; }

int rtmp_handler_onseek(void *param, uint32_t ms) { return 0; }

int rtmp_handler_onpublish(void *param, const char *app, const char *stream, const char *type) {
  RtmpSession *session = (RtmpSession *)param;
  session->mStatus = RS_INIT;

  return 0;
}

int rtmp_handler_onscript(void *param, const void *data, size_t bytes, uint32_t timestamp) {
  RtmpSession *session = (RtmpSession *)param;
  return flv_demuxer_input(session->flvDemuxer, FLV_TYPE_SCRIPT, data, bytes, timestamp);
  // return 0;
}

int rtmp_handler_onvideo(void *param, const void *data, size_t bytes, uint32_t timestamp) {
  RtmpSession *session = (RtmpSession *)param;
  return flv_demuxer_input(session->flvDemuxer, FLV_TYPE_VIDEO, data, bytes, timestamp);
}

int rtmp_handler_onaudio(void *param, const void *data, size_t bytes, uint32_t timestamp) {
  RtmpSession *session = (RtmpSession *)param;
  return flv_demuxer_input(session->flvDemuxer, FLV_TYPE_AUDIO, data, bytes, timestamp);
  // return 0;
}

int rtmp_handler_ongetduration(void *param, const char *app, const char *stream, double *duration) {
  return 0;
}
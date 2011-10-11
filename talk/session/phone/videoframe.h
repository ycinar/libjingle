/*
 * libjingle
 * Copyright 2004--2011, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TALK_SESSION_PHONE_VIDEOFRAME_H_
#define TALK_SESSION_PHONE_VIDEOFRAME_H_

#include "talk/base/basictypes.h"

namespace flute {
class MagicCamVideoRenderer;
}

namespace cricket {

// Simple rotation constants.
enum {
  ROTATION_0 = 0,
  ROTATION_90 = 90,
  ROTATION_180 = 180,
  ROTATION_270 = 270
};

// Represents a YUV420 (a.k.a. I420) video frame.
class VideoFrame {
  friend class flute::MagicCamVideoRenderer;

 public:
  VideoFrame() : rendered_(false) {}

  virtual ~VideoFrame() {}

  virtual size_t GetWidth() const = 0;
  virtual size_t GetHeight() const = 0;
  size_t GetChromaWidth() const { return (GetWidth() + 1) / 2; }
  size_t GetChromaHeight() const { return (GetHeight() + 1) / 2; }
  virtual const uint8 *GetYPlane() const = 0;
  virtual const uint8 *GetUPlane() const = 0;
  virtual const uint8 *GetVPlane() const = 0;
  virtual uint8 *GetYPlane() = 0;
  virtual uint8 *GetUPlane() = 0;
  virtual uint8 *GetVPlane() = 0;
  virtual int32 GetYPitch() const = 0;
  virtual int32 GetUPitch() const = 0;
  virtual int32 GetVPitch() const = 0;

  // For retrieving the aspect ratio of each pixel. Usually this is 1x1, but
  // the aspect_ratio_idc parameter of H.264 can specify non-square pixels.
  virtual size_t GetPixelWidth() const = 0;
  virtual size_t GetPixelHeight() const = 0;

  // TODO: Add a fourcc format here and probably combine VideoFrame
  // with CapturedFrame.
  virtual int64 GetElapsedTime() const = 0;
  virtual int64 GetTimeStamp() const = 0;
  virtual void SetElapsedTime(int64 elapsed_time) = 0;
  virtual void SetTimeStamp(int64 time_stamp) = 0;

  // Indicates the rotation angle in degrees.
  virtual int GetRotation() const = 0;

  // Make a shallow copy of the frame. The frame buffer itself is not copied.
  // Both the current and new VideoFrame will share a single reference-counted
  // frame buffer.
  virtual VideoFrame *Copy() const = 0;

  // Since VideoFrame supports shallow copy and the internal frame buffer might
  // be shared, in case VideoFrame needs exclusive access of the frame buffer,
  // user can call MakeExclusive() to make sure the frame buffer is exclusive
  // accessable to the current object.  This might mean a deep copy of the frame
  // buffer if it is currently shared by other objects.
  virtual bool MakeExclusive() = 0;

  // Writes the frame into the given frame buffer, provided that it is of
  // sufficient size. Returns the frame's actual size, regardless of whether
  // it was written or not (like snprintf). If there is insufficient space,
  // nothing is written.
  virtual size_t CopyToBuffer(uint8 *buffer, size_t size) const = 0;

  // Converts the I420 data to RGB of a certain type such as ARGB and ABGR.
  // Returns the frame's actual size, regardless of whether it was written or
  // not (like snprintf). Parameters size and pitch_rgb are in units of bytes.
  // If there is insufficient space, nothing is written.
  virtual size_t ConvertToRgbBuffer(uint32 to_fourcc, uint8 *buffer,
                                    size_t size, size_t pitch_rgb) const = 0;

  // Writes the frame into the given planes, stretched to the given width and
  // height. The parameter "interpolate" controls whether to interpolate or just
  // take the nearest-point. The parameter "crop" controls whether to crop this
  // frame to the aspect ratio of the given dimensions before stretching.
  virtual void StretchToPlanes(uint8 *y, uint8 *u, uint8 *v,
                               int32 pitchY, int32 pitchU, int32 pitchV,
                               size_t width, size_t height,
                               bool interpolate, bool crop) const = 0;

  // Writes the frame into the given frame buffer, stretched to the given width
  // and height, provided that it is of sufficient size. Returns the frame's
  // actual size, regardless of whether it was written or not (like snprintf).
  // If there is insufficient space, nothing is written. The parameter
  // "interpolate" controls whether to interpolate or just take the
  // nearest-point. The parameter "crop" controls whether to crop this frame to
  // the aspect ratio of the given dimensions before stretching.
  virtual size_t StretchToBuffer(size_t w, size_t h, uint8 *buffer, size_t size,
                                 bool interpolate, bool crop) const = 0;

  // Writes the frame into the target VideoFrame, stretched to the size of that
  // frame. The parameter "interpolate" controls whether to interpolate or just
  // take the nearest-point. The parameter "crop" controls whether to crop this
  // frame to the aspect ratio of the target frame before stretching.
  virtual void StretchToFrame(VideoFrame *target, bool interpolate,
                              bool crop) const = 0;

  // Stretches the frame to the given size, creating a new VideoFrame object to
  // hold it. The parameter "interpolate" controls whether to interpolate or
  // just take the nearest-point. The parameter "crop" controls whether to crop
  // this frame to the aspect ratio of the given dimensions before stretching.
  virtual VideoFrame *Stretch(size_t w, size_t h, bool interpolate,
                              bool crop) const = 0;

  // Set the video frame to black.
  bool SetToBlack();

  // Size of an I420 image of given dimensions when stored as a frame buffer.
  static size_t SizeOf(size_t w, size_t h) {
    return w * h + ((w + 1) / 2) * ((h + 1) / 2) * 2;
  }

 protected:
  // The frame needs to be rendered to magiccam only once.
  // TODO: Remove this flag once magiccam rendering is fully replaced
  // by client3d rendering.
  mutable bool rendered_;
};

}  // namespace cricket

#endif  // TALK_SESSION_PHONE_VIDEOFRAME_H_
#pragma once
#include "sdmmc.h"
#include "ff.h"

#include <cstddef>
#include <cstdint>

/** SD card media player.
 *
 *  Mounts the card, lists playable files, and decodes them into a ring
 *  buffer that the audio callback pulls from.
 *
 *  WAV (16/24-bit PCM, mono or stereo) is implemented. MP3 / FLAC / AAC
 *  plug in at DecodeSome() once their decoders are wired up.
 */
class MediaPlayer
{
  public:
    static constexpr size_t kMaxFiles   = 64;
    static constexpr size_t kMaxName    = 256;
    static constexpr size_t kRingFrames = 1024;

    /** Mounts the SD card (SDMMC, 4-bit, fast). */
    bool Init();

    /** Scans a directory for playable files. */
    bool ScanDir(const char *path = "/");

    size_t      FileCount() const { return fileCount_; }
    const char *FileName(size_t i) const;
    const char *FilePath(size_t i) const;

    bool Open(size_t index);
    bool Open(const char *path);
    void Close();
    bool IsPlaying() const { return open_; }

    float   SampleRate() const { return sampleRate_; }
    uint8_t Channels() const { return channels_; }

    /** Pull one stereo frame. Returns false when the buffer is empty. */
    bool Read(float &l, float &r);

    /** Decode ahead into the ring buffer. Call often from the main loop. */
    void Update();

  private:
    bool   ParseWavHeader();
    bool   DecodeSome();
    size_t BytesPerSample() const { return bits_ == 24 ? 3 : 2; }
    float  SampleToFloat(const uint8_t *p) const;
    void   Push(float l, float r);

    daisy::SdmmcHandler sd_;
    FATFS               fs_;
    FIL                 file_;
    bool                mounted_ = false;
    bool                open_    = false;

    char   names_[kMaxFiles][kMaxName];
    char   paths_[kMaxFiles][kMaxName];
    size_t fileCount_ = 0;

    uint32_t dataOffset_ = 0;
    uint32_t dataSize_   = 0;
    uint32_t dataRead_   = 0;
    uint16_t bits_       = 16;
    uint8_t  channels_   = 2;
    float    sampleRate_ = 48000.f;

    float          ringL_[kRingFrames];
    float          ringR_[kRingFrames];
    volatile size_t writeIdx_ = 0;
    volatile size_t readIdx_  = 0;

    uint8_t readBuf_[512];
};

#include "MediaPlayer.h"

#include <cstdio>
#include <cstring>

using namespace daisy;

namespace
{
constexpr size_t kWavHeaderBytes = 12;
constexpr size_t kChunkHeaderBytes = 8;
constexpr size_t kFmtBytes = 16;

uint16_t Le16(const uint8_t *p)
{
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t Le32(const uint8_t *p)
{
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8)
           | (static_cast<uint32_t>(p[2]) << 16)
           | (static_cast<uint32_t>(p[3]) << 24);
}

bool HasExtension(const char *name, const char *ext)
{
    const size_t n = strlen(name);
    const size_t e = strlen(ext);
    if(n < e)
        return false;
    for(size_t i = 0; i < e; i++)
    {
        char a = name[n - e + i];
        char b = ext[i];
        if(a >= 'A' && a <= 'Z')
            a = static_cast<char>(a - 'A' + 'a');
        if(b >= 'A' && b <= 'Z')
            b = static_cast<char>(b - 'A' + 'a');
        if(a != b)
            return false;
    }
    return true;
}
} // namespace

bool MediaPlayer::Init()
{
    SdmmcHandler::Config cfg;
    cfg.Defaults();
    cfg.width = SdmmcHandler::BusWidth::BITS_4;
    cfg.speed = SdmmcHandler::Speed::FAST;

    if(sd_.Init(cfg) != SdmmcHandler::Result::OK)
        return false;

    mounted_ = f_mount(&fs_, "/", 1) == FR_OK;
    return mounted_;
}

bool MediaPlayer::ScanDir(const char *path)
{
    if(!mounted_)
        return false;

    fileCount_ = 0;

    DIR     dir;
    FILINFO info;
    if(f_opendir(&dir, path) != FR_OK)
        return false;

    while(fileCount_ < kMaxFiles)
    {
        if(f_readdir(&dir, &info) != FR_OK || info.fname[0] == 0)
            break;

        if(info.fattrib & (AM_HID | AM_DIR))
            continue;

        // Only WAV is decodable so far. MP3/FLAC/AAC get added here.
        if(!HasExtension(info.fname, ".wav"))
            continue;

        strncpy(names_[fileCount_], info.fname, kMaxName - 1);
        names_[fileCount_][kMaxName - 1] = '\0';

        snprintf(paths_[fileCount_], kMaxName, "%s%s", path, info.fname);

        fileCount_++;
    }

    f_closedir(&dir);
    return fileCount_ > 0;
}

const char *MediaPlayer::FileName(size_t i) const
{
    return i < fileCount_ ? names_[i] : "";
}

const char *MediaPlayer::FilePath(size_t i) const
{
    return i < fileCount_ ? paths_[i] : "";
}

bool MediaPlayer::Open(size_t index)
{
    if(index >= fileCount_)
        return false;
    return Open(paths_[index]);
}

bool MediaPlayer::Open(const char *path)
{
    Close();

    if(f_open(&file_, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
        return false;

    if(!ParseWavHeader())
    {
        f_close(&file_);
        return false;
    }

    dataRead_ = 0;
    writeIdx_ = 0;
    readIdx_  = 0;
    open_     = true;
    return true;
}

void MediaPlayer::Close()
{
    if(open_)
        f_close(&file_);
    open_ = false;
}

bool MediaPlayer::ParseWavHeader()
{
    uint8_t hdr[kWavHeaderBytes];
    UINT    br = 0;

    if(f_read(&file_, hdr, kWavHeaderBytes, &br) != FR_OK
       || br != kWavHeaderBytes)
        return false;

    if(memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0)
        return false;

    bool haveFmt  = false;
    bool haveData = false;

    while(!haveData)
    {
        uint8_t chunk[kChunkHeaderBytes];
        if(f_read(&file_, chunk, kChunkHeaderBytes, &br) != FR_OK
           || br != kChunkHeaderBytes)
            return false;

        const uint32_t size = Le32(chunk + 4);

        if(memcmp(chunk, "fmt ", 4) == 0)
        {
            uint8_t fmt[kFmtBytes];
            if(f_read(&file_, fmt, kFmtBytes, &br) != FR_OK || br != kFmtBytes)
                return false;

            const uint16_t format = Le16(fmt + 0);
            channels_             = static_cast<uint8_t>(Le16(fmt + 2));
            sampleRate_           = static_cast<float>(Le32(fmt + 4));
            bits_                 = Le16(fmt + 14);

            if(format != 1 || (bits_ != 16 && bits_ != 24)
               || (channels_ != 1 && channels_ != 2))
                return false;

            haveFmt = true;

            // Skip any extra format bytes.
            if(size > kFmtBytes)
                f_lseek(&file_, f_tell(&file_) + (size - kFmtBytes));
        }
        else if(memcmp(chunk, "data", 4) == 0)
        {
            dataOffset_ = f_tell(&file_);
            dataSize_   = size;
            haveData    = true;
        }
        else
        {
            // Skip unknown chunk (word aligned).
            f_lseek(&file_,
                    f_tell(&file_) + size + (size & 1));
        }
    }

    if(!haveFmt)
        return false;

    return f_lseek(&file_, dataOffset_) == FR_OK;
}

float MediaPlayer::SampleToFloat(const uint8_t *p) const
{
    if(bits_ == 24)
    {
        int32_t v = static_cast<int32_t>(p[0]) << 8
                    | static_cast<int32_t>(p[1]) << 16
                    | static_cast<int32_t>(p[2]) << 24;
        return static_cast<float>(v >> 8) / 8388608.0f;
    }

    return static_cast<float>(static_cast<int16_t>(p[0] | (p[1] << 8)))
           / 32768.0f;
}

void MediaPlayer::Push(float l, float r)
{
    ringL_[writeIdx_ & (kRingFrames - 1)] = l;
    ringR_[writeIdx_ & (kRingFrames - 1)] = r;
    writeIdx_++;
}

bool MediaPlayer::Read(float &l, float &r)
{
    if(readIdx_ == writeIdx_)
        return false;

    const size_t idx = readIdx_ & (kRingFrames - 1);
    l                = ringL_[idx];
    r                = ringR_[idx];
    readIdx_++;
    return true;
}

bool MediaPlayer::DecodeSome()
{
    if(!open_)
        return false;

    const size_t bps        = BytesPerSample();
    const size_t frameBytes = bps * channels_;
    const size_t used       = writeIdx_ - readIdx_;
    const size_t freeFrames = kRingFrames - used;

    if(freeFrames < 64)
        return false;

    size_t wantBytes = sizeof(readBuf_) - (sizeof(readBuf_) % frameBytes);
    size_t frames    = wantBytes / frameBytes;
    if(frames > freeFrames)
        frames = freeFrames;

    size_t bytes = frames * frameBytes;

    if(dataSize_ != 0)
    {
        const uint32_t remaining = dataSize_ - dataRead_;
        if(bytes > remaining)
            bytes = remaining;
    }

    if(bytes == 0)
    {
        Close(); // end of file
        return false;
    }

    UINT br = 0;
    if(f_read(&file_, readBuf_, bytes, &br) != FR_OK || br == 0)
    {
        Close();
        return false;
    }

    dataRead_ += br;
    frames = br / frameBytes;

    for(size_t i = 0; i < frames; i++)
    {
        const uint8_t *p = readBuf_ + i * frameBytes;
        if(channels_ == 1)
        {
            const float s = SampleToFloat(p);
            Push(s, s);
        }
        else
        {
            Push(SampleToFloat(p), SampleToFloat(p + bps));
        }
    }

    return true;
}

void MediaPlayer::Update()
{
    // Keep the ring topped up, but don't spin forever.
    for(uint8_t i = 0; i < 4; i++)
    {
        if(!DecodeSome())
            break;
    }
}

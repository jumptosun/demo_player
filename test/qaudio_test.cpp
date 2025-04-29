#include <thread>
#include <QCoreApplication>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>
#include <QDebug>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 1. 检查命令行参数
    if (argc != 2) {
        qCritical() << "Usage:" << argv[0] << "<wav_file>";
        return -1;
    }

    // 2. 使用 dr_wav 读取 WAV 文件
    drwav wav;
    if (!drwav_init_file(&wav, argv[1], nullptr)) {
        qCritical() << "Failed to open WAV file";
        return -1;
    }

    // 3. 读取全部 PCM 数据
    const size_t bufferSize = wav.totalPCMFrameCount * wav.channels;
    drwav_int16* pcmData = new drwav_int16[bufferSize];
    size_t samplesRead = drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pcmData);
    if (samplesRead != wav.totalPCMFrameCount) {
        qCritical() << "Failed to read full PCM data";
        drwav_uninit(&wav);
        delete[] pcmData;
        return -1;
    }

    // 4. 配置 Qt 音频格式 
    QAudioFormat format;
    format.setSampleRate(wav.sampleRate);        // 采样率
    format.setChannelCount(wav.channels);        // 声道数
    format.setSampleSize(wav.bitsPerSample);                    // 16位采样
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);  // WAV 小端存储
    format.setSampleType(QAudioFormat::SignedInt);    // 有符号整型

    // 5. 初始化音频设备
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    if (!device.isFormatSupported(format)) {
        qWarning() << "Format not supported, using nearest format";
        format = device.nearestFormat(format);  // 尝试近似格式
    }

    QAudioOutput* audioOutput = new QAudioOutput(device, format);
    QIODevice* audioIO = audioOutput->start();  // 获取音频设备接口

    if (!audioIO) {
        qCritical() << "Failed to start audio output";
        drwav_uninit(&wav);
        delete[] pcmData;
        delete audioOutput;
        return -1;
    }

    // 6. 写入 PCM 数据并等待播放完成
    qint64 remaining = bufferSize * sizeof(drwav_int16);
    const char* dataPtr = reinterpret_cast<const char*>(pcmData);

    while (remaining > 0) {
        qint64 written = audioIO->write(dataPtr, remaining);
        if (written <= 0) break;  // 写入失败
        remaining -= written;
        dataPtr += written;
    }

    // 7. 等待播放完成（简单阻塞）
    while (audioOutput->bytesFree() < audioOutput->bufferSize()) {
        QCoreApplication::processEvents();  // 处理事件循环
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 8. 清理资源
    drwav_uninit(&wav);
    delete[] pcmData;
    delete audioOutput;

    return 0;
}
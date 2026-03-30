#pragma once

#include <QByteArray>
#include <QMap>
#include <QTcpSocket>

#include "device/device_adapter.h"

namespace deviceapp {

class RealDeviceAdapter : public IDeviceAdapter {
    Q_OBJECT

public:
    explicit RealDeviceAdapter(QObject *parent = nullptr);

    void connectToDevice(const DeviceConnectionConfig &config) override;
    void disconnectFromDevice() override;
    void readStatusSnapshot() override;
    void applyScanSettings(const ScanSettings &settings) override;
    void applyTuneParameters(const TuneParameters &parameters) override;
    void applyDataProcessingSettings(const DataProcessingSettings &settings) override;
    void setSwitchState(InstrumentSwitch instrumentSwitch, bool on) override;
    void startScan(ScanMode mode) override;
    void stopScan() override;
    void calibrateMassAxis() override;
    void saveCurrentFrame() override;

private:
    void sendCommand(quint8 cmd1, quint8 cmd2, const QByteArray &data = QByteArray());
    QByteArray packFrame(quint8 source, quint8 target, quint8 cmd1, quint8 cmd2, const QByteArray &data) const;

    void processIncomingData();
    bool tryExtractFrame(QByteArray *buffer, quint8 *source, quint8 *target, quint8 *cmd1, quint8 *cmd2, QByteArray *data) const;
    void handleFrame(quint8 source, quint8 target, quint8 cmd1, quint8 cmd2, const QByteArray &data);

    void handleAckNak(bool ack, quint8 replyCmd1, quint8 replyCmd2);
    void handleIoState(const QByteArray &data);
    void handleAdState(const QByteArray &data);
    void handleSimData(const QByteArray &data);
    void handleRawScanStart(const QByteArray &data);
    void handleRawScanChunk(const QByteArray &data);
    void handleRawScanEnd();

    SpectrumFrame decodeSpectrumFromPayload(const QByteArray &payload, ScanMode mode) const;
    static int ioChannelFromSwitch(InstrumentSwitch instrumentSwitch);
    static quint16 encodeDaValue(double value, double rate);
    static void appendLe16(QByteArray *buffer, quint16 value);
    static void appendLe32(QByteArray *buffer, qint32 value);
    static quint16 readLe16(const QByteArray &data, int offset);
    static qint32 readLe32(const QByteArray &data, int offset);
    static quint8 checksum8(const QByteArray &data, int count);

    void emitStatus();

private:
    QTcpSocket socket_;
    QByteArray rxBuffer_;
    DeviceConnectionConfig connectionConfig_;
    InstrumentStatus status_;
    ScanSettings scanSettings_;
    TuneParameters tuneParameters_;
    DataProcessingSettings dataProcessingSettings_;
    SpectrumFrame lastFrame_;

    int expectedPackageCount_ = 0;
    int expectedPayloadSize_ = 0;
    QMap<int, QByteArray> scanChunks_;

    static constexpr quint8 kFrameHead = 0x24;     // '$'
    static constexpr quint8 kFrameTail = 0x26;     // '&'
    static constexpr quint8 kAddrPc = 0x02;
    static constexpr quint8 kAddrService = 0x03;
};

}  // namespace deviceapp

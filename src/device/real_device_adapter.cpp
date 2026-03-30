#include "device/real_device_adapter.h"

#include <QDateTime>
#include <QtMath>

#include "core/types.h"

namespace deviceapp {

RealDeviceAdapter::RealDeviceAdapter(QObject *parent) : IDeviceAdapter(parent) {
    status_.vacuum.valuePa = 9.9e-5;
    status_.vacuum.thresholdPa = 5.0e-5;

    connect(&socket_, &QTcpSocket::connected, this, [this]() {
        status_.connected = true;
        status_.lastError.clear();
        emit connectionChanged(true);
        emitStatus();
        readStatusSnapshot();
    });

    connect(&socket_, &QTcpSocket::disconnected, this, [this]() {
        status_.connected = false;
        status_.scanning = false;
        emit connectionChanged(false);
        emitStatus();
    });

    connect(&socket_, &QTcpSocket::readyRead, this, [this]() {
        rxBuffer_.append(socket_.readAll());
        processIncomingData();
    });

    connect(&socket_, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        status_.lastError = socket_.errorString();
        emit errorOccurred("设备通信错误: " + status_.lastError);
        emitStatus();
    });
}

void RealDeviceAdapter::connectToDevice(const DeviceConnectionConfig &config) {
    connectionConfig_ = config;
    if (socket_.state() != QAbstractSocket::UnconnectedState) {
        socket_.abort();
    }
    rxBuffer_.clear();
    socket_.connectToHost(config.host, config.port);
}

void RealDeviceAdapter::disconnectFromDevice() {
    socket_.disconnectFromHost();
}

void RealDeviceAdapter::readStatusSnapshot() {
    if (!status_.connected) {
        return;
    }
    // Query all IO states.
    sendCommand(0x80, 0x02, QByteArray(4, '\0'));
    // Query all AD values.
    sendCommand(0x80, 0x05, QByteArray(4, '\0'));
    // Query version.
    sendCommand(0x80, 0x30, QByteArray(4, '\0'));
}

void RealDeviceAdapter::applyScanSettings(const ScanSettings &settings) {
    scanSettings_ = settings;
}

void RealDeviceAdapter::applyTuneParameters(const TuneParameters &parameters) {
    tuneParameters_ = parameters;
    if (!status_.connected) {
        return;
    }

    // 0x80 0x04: set multi-channel DA values, each item is [id_lo id_hi value_lo value_hi]
    QByteArray data;
    const auto appendDa = [&](quint16 channel, quint16 value) {
        appendLe16(&data, channel);
        appendLe16(&data, value);
    };
    appendDa(1, encodeDaValue(parameters.repellerVoltage, -21.57));
    appendDa(2, encodeDaValue(parameters.lens1Voltage, -21.54));
    appendDa(3, encodeDaValue(parameters.lens2Voltage, -22.0));
    appendDa(6, encodeDaValue(-qAbs(parameters.multiplierVoltage), -400.0));
    appendDa(9, encodeDaValue(parameters.lowMassCompensation, 1.0));
    appendDa(11, encodeDaValue(parameters.highMassCompensation, 1.0));
    sendCommand(0x80, 0x04, data);
}

void RealDeviceAdapter::applyDataProcessingSettings(const DataProcessingSettings &settings) {
    dataProcessingSettings_ = settings;
}

void RealDeviceAdapter::setSwitchState(InstrumentSwitch instrumentSwitch, bool on) {
    status_.switchStates[instrumentSwitch] = on;
    emitStatus();
    if (!status_.connected) {
        return;
    }

    QByteArray data;
    data.append(static_cast<char>(ioChannelFromSwitch(instrumentSwitch)));
    data.append(static_cast<char>(on ? 1 : 0));
    sendCommand(0x80, 0x01, data);
}

void RealDeviceAdapter::startScan(ScanMode mode) {
    if (!status_.connected) {
        emit errorOccurred("设备未连接");
        return;
    }

    if (mode == ScanMode::FullScan) {
        QByteArray payload;
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.massStart)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.massEnd)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.scanSpeed)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.voltageRangeMin * 1000.0)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.voltageRangeMax * 1000.0)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.scanTimeMs * 1000.0)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.flybackTimeMs * 1000.0)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.sampleRateHz)));
        sendCommand(0x80, 0x10, payload);
    } else {
        QByteArray payload;
        const int ionCount = scanSettings_.targetIons.size();
        appendLe32(&payload, ionCount);
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.rampVoltage * 1000.0)));
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.flybackTimeMs * 1000.0)));

        for (double ion : scanSettings_.targetIons) {
            appendLe32(&payload, static_cast<qint32>(qRound(ion)));
        }
        for (double voltage : scanSettings_.targetVoltages) {
            appendLe32(&payload, static_cast<qint32>(qRound(voltage * 1000.0)));
        }
        for (int i = 0; i < ionCount; ++i) {
            appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.targetDwellTimeMs * 1000.0)));
        }
        appendLe32(&payload, static_cast<qint32>(qRound(scanSettings_.sampleRateHz)));
        payload.append(static_cast<char>(1));  // Prefer processed SIM data (0x80 0x15)
        sendCommand(0x80, 0x11, payload);
    }

    QByteArray startData(4, '\0');
    startData[0] = 0;  // continuous scan
    sendCommand(0x80, 0x12, startData);
    status_.scanning = true;
    emitStatus();
}

void RealDeviceAdapter::stopScan() {
    if (status_.connected) {
        sendCommand(0x80, 0x13, QByteArray(4, '\0'));
    }
    status_.scanning = false;
    emitStatus();
}

void RealDeviceAdapter::calibrateMassAxis() {
    emit calibrationFinished(false, "真实设备暂未定义质量轴校正命令");
}

void RealDeviceAdapter::saveCurrentFrame() {
    if (!lastFrame_.timestamp.isValid()) {
        emit errorOccurred("当前无可保存扫描帧");
        return;
    }
    emit frameReadyToPersist(lastFrame_);
}

void RealDeviceAdapter::sendCommand(quint8 cmd1, quint8 cmd2, const QByteArray &data) {
    if (socket_.state() != QAbstractSocket::ConnectedState) {
        return;
    }
    socket_.write(packFrame(kAddrPc, kAddrService, cmd1, cmd2, data));
}

QByteArray RealDeviceAdapter::packFrame(quint8 source, quint8 target, quint8 cmd1, quint8 cmd2, const QByteArray &data) const {
    QByteArray frame;
    frame.reserve(10 + data.size());
    frame.append(static_cast<char>(kFrameHead));
    frame.append(static_cast<char>(kFrameHead));
    frame.append(static_cast<char>(source));
    frame.append(static_cast<char>(target));
    const quint16 len = static_cast<quint16>(data.size() + 2);
    frame.append(static_cast<char>(len & 0xff));
    frame.append(static_cast<char>((len >> 8) & 0xff));
    frame.append(static_cast<char>(cmd1));
    frame.append(static_cast<char>(cmd2));
    frame.append(data);
    frame.append(static_cast<char>(checksum8(frame, frame.size())));
    frame.append(static_cast<char>(kFrameTail));
    return frame;
}

void RealDeviceAdapter::processIncomingData() {
    quint8 source = 0;
    quint8 target = 0;
    quint8 cmd1 = 0;
    quint8 cmd2 = 0;
    QByteArray data;
    while (tryExtractFrame(&rxBuffer_, &source, &target, &cmd1, &cmd2, &data)) {
        handleFrame(source, target, cmd1, cmd2, data);
    }
}

bool RealDeviceAdapter::tryExtractFrame(QByteArray *buffer, quint8 *source, quint8 *target, quint8 *cmd1, quint8 *cmd2,
                                        QByteArray *data) const {
    int start = buffer->indexOf(QByteArray("\x24\x24", 2));
    if (start < 0) {
        buffer->clear();
        return false;
    }
    if (start > 0) {
        buffer->remove(0, start);
    }
    if (buffer->size() < 8) {
        return false;
    }

    const quint16 len = static_cast<quint8>(buffer->at(4)) | (static_cast<quint16>(static_cast<quint8>(buffer->at(5))) << 8);
    const int total = 8 + len;
    if (buffer->size() < total) {
        return false;
    }

    const quint8 tail = static_cast<quint8>(buffer->at(total - 1));
    if (tail != kFrameTail) {
        buffer->remove(0, 2);
        return true;
    }

    const quint8 cs = static_cast<quint8>(buffer->at(total - 2));
    const quint8 expect = checksum8(*buffer, total - 2);
    if (cs != expect) {
        buffer->remove(0, total);
        return true;
    }

    *source = static_cast<quint8>(buffer->at(2));
    *target = static_cast<quint8>(buffer->at(3));
    *cmd1 = static_cast<quint8>(buffer->at(6));
    *cmd2 = static_cast<quint8>(buffer->at(7));
    const int payloadSize = qMax(0, static_cast<int>(len) - 2);
    *data = buffer->mid(8, payloadSize);
    buffer->remove(0, total);
    return true;
}

void RealDeviceAdapter::handleFrame(quint8, quint8, quint8 cmd1, quint8 cmd2, const QByteArray &data) {
    if (cmd1 == 0x06 || cmd1 == 0x15) {
        const quint8 replyCmd1 = data.size() > 0 ? static_cast<quint8>(data[0]) : 0;
        const quint8 replyCmd2 = data.size() > 1 ? static_cast<quint8>(data[1]) : 0;
        handleAckNak(cmd1 == 0x06, replyCmd1, replyCmd2);
        return;
    }

    if (cmd1 != 0x80) {
        return;
    }
    switch (cmd2) {
    case 0x02:
        handleIoState(data);
        break;
    case 0x05:
        handleAdState(data);
        break;
    case 0x14:
        if (data.size() >= 1 && data[0] == 0) {
            emit errorOccurred("扫描参数设置失败");
        }
        break;
    case 0x15:
        handleSimData(data);
        break;
    case 0x16:
        handleRawScanStart(data);
        break;
    case 0x17:
        handleRawScanChunk(data);
        break;
    case 0x18:
        handleRawScanEnd();
        break;
    case 0x30:
        // version frame
        break;
    default:
        break;
    }
}

void RealDeviceAdapter::handleAckNak(bool ack, quint8 replyCmd1, quint8 replyCmd2) {
    if (!ack) {
        emit errorOccurred(QString("设备返回NAK: cmd=%1 %2")
                               .arg(replyCmd1, 2, 16, QChar('0'))
                               .arg(replyCmd2, 2, 16, QChar('0')));
    }
}

void RealDeviceAdapter::handleIoState(const QByteArray &data) {
    // Pairs: [channel, state]
    for (int i = 0; i + 1 < data.size(); i += 2) {
        const int channel = static_cast<quint8>(data[i]);
        const bool on = static_cast<quint8>(data[i + 1]) != 0;
        switch (channel) {
        case 6:
            status_.switchStates[InstrumentSwitch::ForePump] = on;
            break;
        case 10:
            status_.switchStates[InstrumentSwitch::ForeValve] = on;
            break;
        case 5:
            status_.switchStates[InstrumentSwitch::MolecularPump] = on;
            break;
        case 4:
            status_.switchStates[InstrumentSwitch::InletValve] = on;
            break;
        case 3:
            status_.switchStates[InstrumentSwitch::Filament] = on;
            break;
        case 1:
            status_.switchStates[InstrumentSwitch::Multiplier] = on;
            break;
        default:
            break;
        }
    }
    emitStatus();
}

void RealDeviceAdapter::handleAdState(const QByteArray &data) {
    // Layout: [ch_lo ch_hi value_lo value_hi]...
    for (int i = 0; i + 3 < data.size(); i += 4) {
        const quint16 channel = readLe16(data, i);
        const quint16 raw = readLe16(data, i + 2);
        if (channel == 6) {
            // rough proxy: map ion pump current raw value to vacuum Pa
            status_.vacuum.valuePa = qMax(1e-9, raw / 1.0e8);
        }
    }
    emitStatus();
}

void RealDeviceAdapter::handleSimData(const QByteArray &data) {
    lastFrame_ = decodeSpectrumFromPayload(data, ScanMode::SelectedIon);
    if (!lastFrame_.timestamp.isValid()) {
        return;
    }
    const SpectrumFrame processed = applyDataProcessing(lastFrame_, dataProcessingSettings_);
    emit frameUpdated(processed);
}

void RealDeviceAdapter::handleRawScanStart(const QByteArray &data) {
    expectedPackageCount_ = data.size() >= 4 ? readLe32(data, 0) : 0;
    expectedPayloadSize_ = data.size() >= 8 ? readLe32(data, 4) : 0;
    scanChunks_.clear();
}

void RealDeviceAdapter::handleRawScanChunk(const QByteArray &data) {
    if (data.size() < 4) {
        return;
    }
    const int index = readLe32(data, 0);
    scanChunks_[index] = data.mid(4);
}

void RealDeviceAdapter::handleRawScanEnd() {
    QByteArray payload;
    for (int i = 1; i <= expectedPackageCount_; ++i) {
        payload.append(scanChunks_.value(i));
    }
    if (expectedPayloadSize_ > 0 && payload.size() > expectedPayloadSize_) {
        payload.truncate(expectedPayloadSize_);
    }
    lastFrame_ = decodeSpectrumFromPayload(payload, scanSettings_.mode);
    if (lastFrame_.timestamp.isValid()) {
        const SpectrumFrame processed = applyDataProcessing(lastFrame_, dataProcessingSettings_);
        emit frameUpdated(processed);
    }
    scanChunks_.clear();
}

SpectrumFrame RealDeviceAdapter::decodeSpectrumFromPayload(const QByteArray &payload, ScanMode mode) const {
    SpectrumFrame frame;
    if (payload.isEmpty()) {
        return frame;
    }
    frame.timestamp = QDateTime::currentDateTime();
    frame.scanMode = mode;
    frame.detector = tuneParameters_.detector;

    QVector<double> intensities;
    if (payload.size() % 4 == 0) {
        const int count = payload.size() / 4;
        intensities.reserve(count);
        for (int i = 0; i < count; ++i) {
            intensities.append(readLe32(payload, i * 4));
        }
    } else if (payload.size() % 2 == 0) {
        const int count = payload.size() / 2;
        intensities.reserve(count);
        for (int i = 0; i < count; ++i) {
            intensities.append(readLe16(payload, i * 2));
        }
    } else {
        return frame;
    }

    const int n = intensities.size();
    frame.intensities = intensities;
    frame.masses.resize(n);

    if (mode == ScanMode::SelectedIon && !scanSettings_.targetIons.isEmpty()) {
        for (int i = 0; i < n; ++i) {
            frame.masses[i] = scanSettings_.targetIons[i % scanSettings_.targetIons.size()];
        }
    } else {
        const double startMz = scanSettings_.massStart;
        const double stopMz = scanSettings_.massEnd;
        for (int i = 0; i < n; ++i) {
            frame.masses[i] = n > 1 ? (startMz + (stopMz - startMz) * i / static_cast<double>(n - 1)) : startMz;
        }
    }

    const auto appendPeakNear = [&](double targetMass) {
        if (frame.masses.isEmpty()) {
            return;
        }
        int best = 0;
        double bestDist = qAbs(frame.masses[0] - targetMass);
        for (int i = 1; i < frame.masses.size(); ++i) {
            const double d = qAbs(frame.masses[i] - targetMass);
            if (d < bestDist) {
                bestDist = d;
                best = i;
            }
        }
        frame.peaks.append({frame.masses[best], frame.intensities[best]});
    };
    appendPeakNear(18.0);
    appendPeakNear(28.0);
    appendPeakNear(44.0);
    return frame;
}

int RealDeviceAdapter::ioChannelFromSwitch(InstrumentSwitch instrumentSwitch) {
    switch (instrumentSwitch) {
    case InstrumentSwitch::ForePump:
        return 6;
    case InstrumentSwitch::ForeValve:
        return 10;
    case InstrumentSwitch::MolecularPump:
        return 5;
    case InstrumentSwitch::InletValve:
        return 4;
    case InstrumentSwitch::Filament:
        return 3;
    case InstrumentSwitch::Multiplier:
        return 1;
    }
    return 0;
}

quint16 RealDeviceAdapter::encodeDaValue(double value, double rate) {
    if (qFuzzyIsNull(rate)) {
        return 0;
    }
    const double v1 = value / rate;
    const double digital = ((v1 + 10.0) / 20.0) * 4095.0;
    return static_cast<quint16>(qBound(0, static_cast<int>(qRound(digital)), 4095));
}

void RealDeviceAdapter::appendLe16(QByteArray *buffer, quint16 value) {
    buffer->append(static_cast<char>(value & 0xff));
    buffer->append(static_cast<char>((value >> 8) & 0xff));
}

void RealDeviceAdapter::appendLe32(QByteArray *buffer, qint32 value) {
    buffer->append(static_cast<char>(value & 0xff));
    buffer->append(static_cast<char>((value >> 8) & 0xff));
    buffer->append(static_cast<char>((value >> 16) & 0xff));
    buffer->append(static_cast<char>((value >> 24) & 0xff));
}

quint16 RealDeviceAdapter::readLe16(const QByteArray &data, int offset) {
    if (offset + 1 >= data.size()) {
        return 0;
    }
    return static_cast<quint8>(data[offset]) | (static_cast<quint16>(static_cast<quint8>(data[offset + 1])) << 8);
}

qint32 RealDeviceAdapter::readLe32(const QByteArray &data, int offset) {
    if (offset + 3 >= data.size()) {
        return 0;
    }
    return static_cast<qint32>(static_cast<quint8>(data[offset])) |
           (static_cast<qint32>(static_cast<quint8>(data[offset + 1])) << 8) |
           (static_cast<qint32>(static_cast<quint8>(data[offset + 2])) << 16) |
           (static_cast<qint32>(static_cast<quint8>(data[offset + 3])) << 24);
}

quint8 RealDeviceAdapter::checksum8(const QByteArray &data, int count) {
    quint32 sum = 0;
    for (int i = 0; i < count && i < data.size(); ++i) {
        sum += static_cast<quint8>(data[i]);
    }
    return static_cast<quint8>(sum & 0xff);
}

void RealDeviceAdapter::emitStatus() {
    emit statusUpdated(status_);
}

}  // namespace deviceapp

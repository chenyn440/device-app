#pragma once

#include <memory>

#include "app/services.h"
#include "device/device_adapter.h"

namespace deviceapp {

struct ApplicationContext {
    std::unique_ptr<IDeviceAdapter> deviceAdapter;
    std::unique_ptr<SettingsRepository> settingsRepository;
    std::unique_ptr<MethodRepository> methodRepository;
    std::unique_ptr<FrameRepository> frameRepository;
    std::unique_ptr<ConnectionService> connectionService;
    std::unique_ptr<ScanControlService> scanControlService;
    std::unique_ptr<TuneService> tuneService;
    std::unique_ptr<MonitorService> monitorService;
    std::unique_ptr<PersistenceService> persistenceService;
    std::unique_ptr<SettingsService> settingsService;
    std::unique_ptr<AiAssistantService> aiAssistantService;

    ApplicationContext();
};

}  // namespace deviceapp

#include "app/application_context.h"

#include "device/mock_device_adapter.h"
#include "device/real_device_adapter.h"

namespace deviceapp {

ApplicationContext::ApplicationContext() {
    const QString adapterName = qEnvironmentVariable("DEVICE_APP_ADAPTER").trimmed().toLower();
    if (adapterName == "real" || qEnvironmentVariableIntValue("DEVICE_APP_USE_REAL") == 1) {
        deviceAdapter = std::make_unique<RealDeviceAdapter>();
    } else {
        deviceAdapter = std::make_unique<MockDeviceAdapter>();
    }
    settingsRepository = std::make_unique<SettingsRepository>();
    methodRepository = std::make_unique<MethodRepository>();
    frameRepository = std::make_unique<FrameRepository>();
    connectionService = std::make_unique<ConnectionService>(deviceAdapter.get(), settingsRepository.get());
    scanControlService = std::make_unique<ScanControlService>(deviceAdapter.get());
    tuneService = std::make_unique<TuneService>(deviceAdapter.get(), settingsRepository.get());
    monitorService = std::make_unique<MonitorService>(deviceAdapter.get(), methodRepository.get());
    persistenceService = std::make_unique<PersistenceService>(frameRepository.get());
    settingsService = std::make_unique<SettingsService>(settingsRepository.get());
    aiAssistantService = std::make_unique<AiAssistantService>();
}

}  // namespace deviceapp

#include "app/application_context.h"

namespace deviceapp {

ApplicationContext::ApplicationContext() {
    deviceAdapter = std::make_unique<MockDeviceAdapter>();
    settingsRepository = std::make_unique<SettingsRepository>();
    methodRepository = std::make_unique<MethodRepository>();
    frameRepository = std::make_unique<FrameRepository>();
    connectionService = std::make_unique<ConnectionService>(deviceAdapter.get(), settingsRepository.get());
    scanControlService = std::make_unique<ScanControlService>(deviceAdapter.get());
    tuneService = std::make_unique<TuneService>(deviceAdapter.get(), settingsRepository.get());
    monitorService = std::make_unique<MonitorService>(deviceAdapter.get(), methodRepository.get());
    persistenceService = std::make_unique<PersistenceService>(frameRepository.get());
    settingsService = std::make_unique<SettingsService>(settingsRepository.get());
}

}  // namespace deviceapp

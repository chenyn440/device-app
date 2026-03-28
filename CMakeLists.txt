cmake_minimum_required(VERSION 3.21)

project(DeviceApp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Network Qml Quick QuickControls2)

qt_standard_project_setup()

set(APP_SOURCES
    src/app/main.cpp
    src/app/application_context.h
    src/app/application_context.cpp
    src/core/types.h
    src/core/types.cpp
    src/core/app_settings.h
    src/core/app_settings.cpp
    src/device/device_adapter.h
    src/device/mock_device_adapter.h
    src/device/mock_device_adapter.cpp
    src/device/real_device_adapter.h
    src/device/real_device_adapter.cpp
    src/storage/repositories.h
    src/storage/repositories.cpp
    src/app/services.h
    src/app/services.cpp
    src/ui/main_window.h
    src/ui/main_window.cpp
    src/ui/pages/tune_page.h
    src/ui/pages/tune_page.cpp
    src/ui/pages/monitor_page.h
    src/ui/pages/monitor_page.cpp
    src/ui/pages/data_page.h
    src/ui/pages/data_page.cpp
    src/ui/pages/settings_page.h
    src/ui/pages/settings_page.cpp
    src/ui/pages/workbench_placeholder_page.h
    src/ui/pages/workbench_placeholder_page.cpp
    src/ui/widgets/spectrum_view.h
    src/ui/widgets/spectrum_view.cpp
    src/ui/widgets/parameter_slider_row.h
    src/ui/widgets/parameter_slider_row.cpp
    src/ui/widgets/page_action_header.h
    src/ui/widgets/page_action_header.cpp
    src/ui/widgets/instrument_status_panel.h
    src/ui/widgets/instrument_status_panel.cpp
    src/ui/widgets/dual_spectrum_panel.h
    src/ui/widgets/dual_spectrum_panel.cpp
    src/ui/dialogs/connection_dialog.h
    src/ui/dialogs/connection_dialog.cpp
    src/ui/dialogs/scan_settings_dialog.h
    src/ui/dialogs/scan_settings_dialog.cpp
    src/ui/dialogs/frame_viewer_dialog.h
    src/ui/dialogs/frame_viewer_dialog.cpp
    src/ui/dialogs/instrument_control_dialog.h
    src/ui/dialogs/instrument_control_dialog.cpp
    src/ui/dialogs/data_processing_dialog.h
    src/ui/dialogs/data_processing_dialog.cpp
    src/ui/dialogs/system_settings_dialog.h
    src/ui/dialogs/system_settings_dialog.cpp
    src/ui/dialogs/chart_settings_dialog.h
    src/ui/dialogs/chart_settings_dialog.cpp
)

set(GATEWAY_SOURCES
    src/server/main.cpp
    src/server/gateway_server.h
    src/server/gateway_server.cpp
    src/app/application_context.h
    src/app/application_context.cpp
    src/app/services.h
    src/app/services.cpp
    src/core/types.h
    src/core/types.cpp
    src/core/app_settings.h
    src/core/app_settings.cpp
    src/device/device_adapter.h
    src/device/mock_device_adapter.h
    src/device/mock_device_adapter.cpp
    src/device/real_device_adapter.h
    src/device/real_device_adapter.cpp
    src/storage/repositories.h
    src/storage/repositories.cpp
)

set(QML_APP_SOURCES
    src/qml/main.cpp
    src/qml/app_state.h
    src/qml/app_state.cpp
    src/qml/qml_resources.qrc
    src/app/application_context.h
    src/app/application_context.cpp
    src/app/services.h
    src/app/services.cpp
    src/core/types.h
    src/core/types.cpp
    src/core/app_settings.h
    src/core/app_settings.cpp
    src/device/device_adapter.h
    src/device/mock_device_adapter.h
    src/device/mock_device_adapter.cpp
    src/device/real_device_adapter.h
    src/device/real_device_adapter.cpp
    src/storage/repositories.h
    src/storage/repositories.cpp
)

set(MOC_HEADERS
    src/device/device_adapter.h
    src/device/mock_device_adapter.h
    src/device/real_device_adapter.h
    src/storage/repositories.h
    src/app/services.h
    src/ui/main_window.h
    src/ui/pages/tune_page.h
    src/ui/pages/monitor_page.h
    src/ui/pages/data_page.h
    src/ui/pages/settings_page.h
    src/ui/widgets/spectrum_view.h
    src/ui/widgets/parameter_slider_row.h
    src/ui/widgets/page_action_header.h
    src/ui/widgets/instrument_status_panel.h
    src/ui/widgets/dual_spectrum_panel.h
    src/ui/dialogs/connection_dialog.h
    src/ui/dialogs/scan_settings_dialog.h
    src/ui/dialogs/frame_viewer_dialog.h
    src/ui/dialogs/instrument_control_dialog.h
    src/ui/dialogs/data_processing_dialog.h
    src/ui/dialogs/system_settings_dialog.h
    src/ui/dialogs/chart_settings_dialog.h
)

qt_wrap_cpp(APP_MOC_SOURCES ${MOC_HEADERS})
set(GATEWAY_MOC_HEADERS
    src/device/device_adapter.h
    src/device/mock_device_adapter.h
    src/device/real_device_adapter.h
    src/storage/repositories.h
    src/app/services.h
    src/server/gateway_server.h
)
qt_wrap_cpp(GATEWAY_MOC_SOURCES ${GATEWAY_MOC_HEADERS})
set(QML_MOC_HEADERS
    src/device/device_adapter.h
    src/device/mock_device_adapter.h
    src/device/real_device_adapter.h
    src/storage/repositories.h
    src/app/services.h
    src/qml/app_state.h
)
qt_wrap_cpp(QML_MOC_SOURCES ${QML_MOC_HEADERS})

qt_add_executable(device-app WIN32 MACOSX_BUNDLE ${APP_SOURCES} ${APP_MOC_SOURCES})
qt_add_executable(device-gateway ${GATEWAY_SOURCES} ${GATEWAY_MOC_SOURCES})
qt_add_executable(device-app-qml ${QML_APP_SOURCES} ${QML_MOC_SOURCES})

target_include_directories(device-app PRIVATE src)
target_include_directories(device-gateway PRIVATE src)
target_include_directories(device-app-qml PRIVATE src)

target_link_libraries(device-app PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)

target_link_libraries(device-gateway PRIVATE
    Qt6::Core
    Qt6::Network
)

target_link_libraries(device-app-qml PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Charts
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

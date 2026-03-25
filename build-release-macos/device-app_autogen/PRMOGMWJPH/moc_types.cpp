/****************************************************************************
** Meta object code from reading C++ file 'types.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/core/types.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'types.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN9deviceappE_t {};
} // unnamed namespace

template <> constexpr inline auto deviceapp::qt_create_metaobjectdata<qt_meta_tag_ZN9deviceappE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "deviceapp",
        "ScanMode",
        "FullScan",
        "SelectedIon",
        "DetectorType",
        "ElectronMultiplier",
        "FaradayCup",
        "InstrumentSwitch",
        "ForePump",
        "ForeValve",
        "MolecularPump",
        "InletValve",
        "Filament",
        "Multiplier"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'ScanMode'
        QtMocHelpers::EnumData<ScanMode>(1, 1, QMC::EnumIsScoped).add({
            {    2, ScanMode::FullScan },
            {    3, ScanMode::SelectedIon },
        }),
        // enum 'DetectorType'
        QtMocHelpers::EnumData<DetectorType>(4, 4, QMC::EnumIsScoped).add({
            {    5, DetectorType::ElectronMultiplier },
            {    6, DetectorType::FaradayCup },
        }),
        // enum 'InstrumentSwitch'
        QtMocHelpers::EnumData<InstrumentSwitch>(7, 7, QMC::EnumIsScoped).add({
            {    8, InstrumentSwitch::ForePump },
            {    9, InstrumentSwitch::ForeValve },
            {   10, InstrumentSwitch::MolecularPump },
            {   11, InstrumentSwitch::InletValve },
            {   12, InstrumentSwitch::Filament },
            {   13, InstrumentSwitch::Multiplier },
        }),
    };
    return QtMocHelpers::metaObjectData<void, qt_meta_tag_ZN9deviceappE_t>(QMC::PropertyAccessInStaticMetaCall, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}

static constexpr auto qt_staticMetaObjectContent_ZN9deviceappE =
    deviceapp::qt_create_metaobjectdata<qt_meta_tag_ZN9deviceappE_t>();
static constexpr auto qt_staticMetaObjectStaticContent_ZN9deviceappE =
    qt_staticMetaObjectContent_ZN9deviceappE.staticData;
static constexpr auto qt_staticMetaObjectRelocatingContent_ZN9deviceappE =
    qt_staticMetaObjectContent_ZN9deviceappE.relocatingData;

Q_CONSTINIT const QMetaObject deviceapp::staticMetaObject = { {
    nullptr,
    qt_staticMetaObjectStaticContent_ZN9deviceappE.stringdata,
    qt_staticMetaObjectStaticContent_ZN9deviceappE.data,
    nullptr,
    nullptr,
    qt_staticMetaObjectRelocatingContent_ZN9deviceappE.metaTypes,
    nullptr
} };

QT_WARNING_POP

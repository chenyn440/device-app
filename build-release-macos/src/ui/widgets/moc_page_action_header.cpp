/****************************************************************************
** Meta object code from reading C++ file 'page_action_header.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/widgets/page_action_header.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'page_action_header.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t {};
} // unnamed namespace

template <> constexpr inline auto deviceapp::PageActionHeader::qt_create_metaobjectdata<qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "deviceapp::PageActionHeader",
        "startRequested",
        "",
        "stopRequested",
        "calibrateRequested",
        "filamentSwitchRequested",
        "saveRequested",
        "refreshRequested",
        "switchStateChanged",
        "InstrumentSwitch",
        "instrumentSwitch",
        "checked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'startRequested'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stopRequested'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'calibrateRequested'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'filamentSwitchRequested'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'saveRequested'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'refreshRequested'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'switchStateChanged'
        QtMocHelpers::SignalData<void(InstrumentSwitch, bool)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 9, 10 }, { QMetaType::Bool, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<PageActionHeader, qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject deviceapp::PageActionHeader::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t>.metaTypes,
    nullptr
} };

void deviceapp::PageActionHeader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PageActionHeader *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->startRequested(); break;
        case 1: _t->stopRequested(); break;
        case 2: _t->calibrateRequested(); break;
        case 3: _t->filamentSwitchRequested(); break;
        case 4: _t->saveRequested(); break;
        case 5: _t->refreshRequested(); break;
        case 6: _t->switchStateChanged((*reinterpret_cast<std::add_pointer_t<InstrumentSwitch>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)()>(_a, &PageActionHeader::startRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)()>(_a, &PageActionHeader::stopRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)()>(_a, &PageActionHeader::calibrateRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)()>(_a, &PageActionHeader::filamentSwitchRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)()>(_a, &PageActionHeader::saveRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)()>(_a, &PageActionHeader::refreshRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (PageActionHeader::*)(InstrumentSwitch , bool )>(_a, &PageActionHeader::switchStateChanged, 6))
            return;
    }
}

const QMetaObject *deviceapp::PageActionHeader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *deviceapp::PageActionHeader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deviceapp16PageActionHeaderE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int deviceapp::PageActionHeader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void deviceapp::PageActionHeader::startRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void deviceapp::PageActionHeader::stopRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void deviceapp::PageActionHeader::calibrateRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void deviceapp::PageActionHeader::filamentSwitchRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void deviceapp::PageActionHeader::saveRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void deviceapp::PageActionHeader::refreshRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void deviceapp::PageActionHeader::switchStateChanged(InstrumentSwitch _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}
QT_WARNING_POP

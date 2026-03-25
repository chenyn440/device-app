/****************************************************************************
** Meta object code from reading C++ file 'monitor_page.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/pages/monitor_page.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'monitor_page.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9deviceapp11MonitorPageE_t {};
} // unnamed namespace

template <> constexpr inline auto deviceapp::MonitorPage::qt_create_metaobjectdata<qt_meta_tag_ZN9deviceapp11MonitorPageE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "deviceapp::MonitorPage",
        "methodSaveRequested",
        "",
        "MonitorMethod",
        "method",
        "methodLoadRequested",
        "fileName",
        "startRequested",
        "stopRequested"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'methodSaveRequested'
        QtMocHelpers::SignalData<void(const MonitorMethod &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'methodLoadRequested'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'startRequested'
        QtMocHelpers::SignalData<void(const MonitorMethod &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'stopRequested'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MonitorPage, qt_meta_tag_ZN9deviceapp11MonitorPageE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject deviceapp::MonitorPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deviceapp11MonitorPageE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deviceapp11MonitorPageE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9deviceapp11MonitorPageE_t>.metaTypes,
    nullptr
} };

void deviceapp::MonitorPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MonitorPage *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->methodSaveRequested((*reinterpret_cast<std::add_pointer_t<MonitorMethod>>(_a[1]))); break;
        case 1: _t->methodLoadRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->startRequested((*reinterpret_cast<std::add_pointer_t<MonitorMethod>>(_a[1]))); break;
        case 3: _t->stopRequested(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MonitorPage::*)(const MonitorMethod & )>(_a, &MonitorPage::methodSaveRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MonitorPage::*)(const QString & )>(_a, &MonitorPage::methodLoadRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (MonitorPage::*)(const MonitorMethod & )>(_a, &MonitorPage::startRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (MonitorPage::*)()>(_a, &MonitorPage::stopRequested, 3))
            return;
    }
}

const QMetaObject *deviceapp::MonitorPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *deviceapp::MonitorPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9deviceapp11MonitorPageE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int deviceapp::MonitorPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void deviceapp::MonitorPage::methodSaveRequested(const MonitorMethod & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void deviceapp::MonitorPage::methodLoadRequested(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void deviceapp::MonitorPage::startRequested(const MonitorMethod & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void deviceapp::MonitorPage::stopRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP

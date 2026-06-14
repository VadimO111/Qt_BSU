/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_MainWindow_t {
    uint offsetsAndSizes[34];
    char stringdata0[11];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[17];
    char stringdata4[7];
    char stringdata5[8];
    char stringdata6[14];
    char stringdata7[17];
    char stringdata8[14];
    char stringdata9[6];
    char stringdata10[19];
    char stringdata11[16];
    char stringdata12[9];
    char stringdata13[15];
    char stringdata14[16];
    char stringdata15[16];
    char stringdata16[21];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 13),  // "ButtonChanged"
        QT_MOC_LITERAL(25, 0),  // ""
        QT_MOC_LITERAL(26, 16),  // "QAbstractButton*"
        QT_MOC_LITERAL(43, 6),  // "button"
        QT_MOC_LITERAL(50, 7),  // "AddItem"
        QT_MOC_LITERAL(58, 13),  // "RefreshTables"
        QT_MOC_LITERAL(72, 16),  // "UpdateDetailView"
        QT_MOC_LITERAL(89, 13),  // "PerformSearch"
        QT_MOC_LITERAL(103, 5),  // "query"
        QT_MOC_LITERAL(109, 18),  // "deleteSelectedItem"
        QT_MOC_LITERAL(128, 15),  // "HandleCoverDrop"
        QT_MOC_LITERAL(144, 8),  // "filePath"
        QT_MOC_LITERAL(153, 14),  // "SelectTextFile"
        QT_MOC_LITERAL(168, 15),  // "ShowReadingView"
        QT_MOC_LITERAL(184, 15),  // "GoBackToDetails"
        QT_MOC_LITERAL(200, 20)   // "openAssignBookDialog"
    },
    "MainWindow",
    "ButtonChanged",
    "",
    "QAbstractButton*",
    "button",
    "AddItem",
    "RefreshTables",
    "UpdateDetailView",
    "PerformSearch",
    "query",
    "deleteSelectedItem",
    "HandleCoverDrop",
    "filePath",
    "SelectTextFile",
    "ShowReadingView",
    "GoBackToDetails",
    "openAssignBookDialog"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   80,    2, 0x08,    1 /* Private */,
       5,    0,   83,    2, 0x08,    3 /* Private */,
       6,    0,   84,    2, 0x08,    4 /* Private */,
       7,    0,   85,    2, 0x08,    5 /* Private */,
       8,    1,   86,    2, 0x08,    6 /* Private */,
      10,    0,   89,    2, 0x08,    8 /* Private */,
      11,    1,   90,    2, 0x08,    9 /* Private */,
      13,    0,   93,    2, 0x08,   11 /* Private */,
      14,    0,   94,    2, 0x08,   12 /* Private */,
      15,    0,   95,    2, 0x08,   13 /* Private */,
      16,    0,   96,    2, 0x08,   14 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSizes,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'ButtonChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QAbstractButton *, std::false_type>,
        // method 'AddItem'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'RefreshTables'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'UpdateDetailView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'PerformSearch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteSelectedItem'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'HandleCoverDrop'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'SelectTextFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'ShowReadingView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'GoBackToDetails'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openAssignBookDialog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->ButtonChanged((*reinterpret_cast< std::add_pointer_t<QAbstractButton*>>(_a[1]))); break;
        case 1: _t->AddItem(); break;
        case 2: _t->RefreshTables(); break;
        case 3: _t->UpdateDetailView(); break;
        case 4: _t->PerformSearch((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->deleteSelectedItem(); break;
        case 6: _t->HandleCoverDrop((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->SelectTextFile(); break;
        case 8: _t->ShowReadingView(); break;
        case 9: _t->GoBackToDetails(); break;
        case 10: _t->openAssignBookDialog(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractButton* >(); break;
            }
            break;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

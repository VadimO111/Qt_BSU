# Определяем C++ библиотеки для каждого модуля
cc_library(
    name = "book",
    srcs = ["book.cpp"],
    hdrs = ["book.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "user",
    srcs = ["user.cpp"],
    hdrs = ["user.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "library",
    srcs = ["library.cpp"],
    hdrs = ["library.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":book",
        ":user",
    ],
)

cc_library(
    name = "mainwindow",
    srcs = ["mainwindow.cpp"],
    hdrs = ["mainwindow.h"],
    deps = [
        ":library",
    ],
)

cc_library(
    name = "assignbookdialog",
    srcs = ["assignbookdialog.cpp"],
    hdrs = ["assignbookdialog.h"],
    deps = [
        ":library",
    ],
)

cc_library(
    name = "coverdroparea",
    srcs = ["coverdroparea.cpp"],
    hdrs = ["coverdroparea.h"],
)

cc_library(
    name = "itemdialog",
    srcs = ["itemdialog.cpp"],
    hdrs = ["itemdialog.h"],
)

# Основной исполняемый файл (программа)
cc_binary(
    name = "l",
    srcs = ["main.cpp"],
    deps = [
        ":assignbookdialog",
        ":coverdroparea",
        ":itemdialog",
        ":library",
        ":mainwindow",
    ],
)

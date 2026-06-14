#pragma once

#include <QDate>
#include "book.h"

class User {
public:
    User(QString name, QString surname, QDate birthday, int id);

    QString GetName() const;

    QString GetSurname() const;

    QDate GetBirthday() const;

    int GetId() const;

private:
    QString name_ = "-";
    QString surname_ = "-";
    QDate birthday_ = {inf, inf, inf};
    int id_ = -1;
};

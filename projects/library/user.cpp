#include "user.h"

User::User(QString name, QString surname, QDate birthday, int id) :
    name_(std::move(name)), surname_(std::move(surname)), birthday_(std::move(birthday)), id_(id) {
}

QString User::GetName() const {
    return name_;
}

QString User::GetSurname() const {
    return surname_;
}

QDate User::GetBirthday() const {
    return birthday_;
}

int User::GetId() const {
    return id_;
}

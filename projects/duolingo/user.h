#pragma once

#include <QString>
#include <QDateTime>

class User
{
public:
    User() = default;

    bool IsValid() const {
        return id_ != -1;
    }

    long long id_ = -1;
    QString name_;
    QString email_;
    QString passwordHash_;
    QString salt_;
    QDateTime createdAt_;

    long long totalStudyTimeSeconds_ = 0;
    int completedExercisesCount_ = 0;
};

#pragma once

#include "user.h"
#include <QDialog>

class QLabel;

class UserStatsDialog : public QDialog {
    Q_OBJECT

public:
    explicit UserStatsDialog(const User& user, QWidget* parent = nullptr);
    ~UserStatsDialog() override;

private:
    void setupUi(const User& user);

    QLabel* usernameLabel_ = nullptr;
    QLabel* registrationDateLabel_ = nullptr;
    QLabel* studyTimeLabel_ = nullptr;
    QLabel* exercisesCompletedLabel_ = nullptr;
    QLabel* totalStarsLabel_ = nullptr;
};
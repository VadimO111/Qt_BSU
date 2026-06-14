#include "userstatsdialog.h"
#include "datamanager.h"
#include "leveldata.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

UserStatsDialog::UserStatsDialog(const User& user, QWidget* parent)
    : QDialog(parent) {
    setupUi(user);
    setWindowTitle("Статистика пользователя: " + user.name_);
    setMinimumWidth(350);
    adjustSize();
}

UserStatsDialog::~UserStatsDialog() {}

void UserStatsDialog::setupUi(const User& user) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* statsLayout = new QFormLayout();

    usernameLabel_ = new QLabel(user.name_, this);
    registrationDateLabel_ =
        new QLabel(user.createdAt_.toString("dd.MM.yyyy hh:mm:ss"), this);

    long long totalSeconds = user.totalStudyTimeSeconds_;
    long long hours = totalSeconds / 3600;
    long long minutes = (totalSeconds % 3600) / 60;
    long long seconds = totalSeconds % 60;
    studyTimeLabel_ = new QLabel(
        QString("%1 ч %2 мин %3 сек").arg(hours).arg(minutes).arg(seconds), this);

    exercisesCompletedLabel_ =
        new QLabel(QString::number(user.completedExercisesCount_), this);

    int totalStarsFromProgress = 0;
    if (user.IsValid()) {
        QMap<long long, UserLessonProgress> allProgress =
            DataManager::Instance().GetAllUserProgress(user.id_);
        for (const UserLessonProgress& p : allProgress) {
            totalStarsFromProgress += p.totalStarsForMenu();
        }
    }
    totalStarsLabel_ =
        new QLabel(QString::number(totalStarsFromProgress), this);
    totalStarsLabel_->setWordWrap(true);

    statsLayout->addRow("Имя пользователя:", usernameLabel_);
    statsLayout->addRow("Дата регистрации:", registrationDateLabel_);
    statsLayout->addRow("Время в приложении:", studyTimeLabel_);
    statsLayout->addRow("Решено наборов упражнений:", exercisesCompletedLabel_);
    statsLayout->addRow("Всего звезд (по урокам):", totalStarsLabel_);

    mainLayout->addLayout(statsLayout);

    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}
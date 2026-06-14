#pragma once

#include "exercisetypes.h"
#include <QList>
#include <QString>

struct Question {
    long long id = -1;
    long long lessonId = -1;
    ExerciseType type = ExerciseType::None;
    QString text;
    QString correctAnswer;
    QList<QString> options;
    QString audioPath;
    QString hint;
    int difficultyOrder = 0;

    bool IsValid() const { return id != -1 && lessonId != -1; }
};

struct Lesson {
    long long id = -1;
    QString name;
    QString description;
    int difficulty = 1;

    bool IsValid() const { return id != -1; }
};

struct UserLessonProgress {
    long long userId = -1;
    long long lessonId = -1;
    int starsTranslation = 0;
    int starsGrammar = 0;
    int starsAudio = 0;

    int totalStarsForMenu() const {
        return starsTranslation + starsGrammar + starsAudio;
    }
    bool IsValid() const { return userId != -1 && lessonId != -1; }
};
#pragma once

#include "leveldata.h"
#include "user.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QSqlDatabase>

class DataManager : public QObject {
    Q_OBJECT

public:
    static DataManager& Instance();
    ~DataManager();

    bool OpenDb(const QString& dbPath = "duolingo_data.db");
    void CloseDb();
    bool IsOpen() const;

    long long AddUser(const User& userData, const QString& password);
    bool UserExists(const QString& username) const;
    User GetUserByUsername(const QString& username);
    User GetUserById(long long userId) const;
    bool CheckPassword(long long userId, const QString& password);
    bool ChangePassword(long long userId, const QString& newPassword);

    void SetCurrentUserId(long long userId);
    long long GetCurrentUserId() const;
    User GetCurrentUser() const;

    QList<Lesson> GetAllLessonsSorted();
    Lesson GetLessonById(long long lessonId);
    QList<Question> GetQuestionsForLesson(long long lessonId,
                                          ExerciseType type = ExerciseType::None,
                                          int count = 0);

    UserLessonProgress GetUserProgressForLesson(long long userId,
                                                long long lessonId);
    bool UpdateUserProgressForLesson(const UserLessonProgress& progress);
    QMap<long long, UserLessonProgress> GetAllUserProgress(long long userId);

    bool UpdateUserStudyTime(long long userId, long long additionalTimeSeconds);
    bool IncrementUserCompletedExercises(long long userId);

    QString GetLastErrorString() const;

signals:
    void DatabaseError(const QString& errorMsg);
    void CurrentUserChanged(long long newUserId);

private:
    explicit DataManager(QObject* parent = nullptr);
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager& other) = delete;

    bool InitSchema();
    bool PopulateInitialData();

    long long addLessonInternal(const QString& name, const QString& description,
                                int difficulty);
    bool addQuestionInternal(long long lessonId, ExerciseType type,
                             const QString& text, const QString& correctAnswer,
                             const QList<QString>& options,
                             const QString& audioPath, const QString& hint,
                             int difficultyOrder);

    mutable QSqlDatabase db_;
    long long currentUserId_ = -1;
    QString lastErrorString_;
};
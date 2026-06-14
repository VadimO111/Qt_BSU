#include "datamanager.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QMetaType>
#include <QRandomGenerator>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

DataManager& DataManager::Instance() {
    static DataManager dmInstance;
    return dmInstance;
}

DataManager::DataManager(QObject* parent)
    : QObject(parent), currentUserId_(-1) {}

DataManager::~DataManager() { CloseDb(); }

QString DataManager::GetLastErrorString() const { return lastErrorString_; }

bool DataManager::OpenDb(const QString& dbPath) {
    if (db_.isOpen() && db_.databaseName() == dbPath) {
        return true;
    }
    if (db_.isOpen()) {
        CloseDb();
    }

    QFileInfo dbFileInfo(dbPath);
    QDir dbDir(dbFileInfo.absolutePath());
    if (!dbDir.exists()) {
        if (!dbDir.mkpath(".")) {
            lastErrorString_ = "Не удалось создать директорию для базы данных: " +
                               dbDir.absolutePath();
            emit DatabaseError(lastErrorString_);
            return false;
        }
    }

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        lastErrorString_ = "Драйвер SQLite (QSQLITE) недоступен.";
        emit DatabaseError(lastErrorString_);
        return false;
    }

    const QString connectionName = QStringLiteral("DuolingoAppConnection_%1")
                                     .arg(QDateTime::currentMSecsSinceEpoch());
    if (QSqlDatabase::contains(connectionName)) {
        db_ = QSqlDatabase::database(connectionName);
    } else {
        db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }
    db_.setDatabaseName(dbPath);

    if (!db_.open()) {
        QSqlError err = db_.lastError();
        QString errorText = err.text().trimmed();
        if (errorText.isEmpty() && err.type() != QSqlError::NoError) {
            errorText =
                QString("SQL Error Type %1 (driver text: %2, native code: %3)")
                    .arg(err.type())
                    .arg(err.driverText().trimmed())
                    .arg(err.nativeErrorCode().trimmed());
        } else if (errorText.isEmpty()) {
            errorText = "Неизвестная ошибка при открытии базы данных.";
        }
        lastErrorString_ =
            QString("Не удалось открыть базу данных '%1': %2")
                .arg(dbPath, errorText);
        emit DatabaseError(lastErrorString_);
        return false;
    }

    QSqlQuery pragmaQuery(db_);
    if (!pragmaQuery.exec("PRAGMA foreign_keys = ON;")) {
        lastErrorString_ =
            "Не удалось включить внешние ключи: " + pragmaQuery.lastError().text();
        emit DatabaseError(lastErrorString_);
    }

    if (!InitSchema()) {
        CloseDb();
        return false;
    }

    QString populatedFlagPath = QCoreApplication::applicationDirPath() +
                                QDir::separator() + dbFileInfo.baseName() +
                                "-populated.flag";
    bool firstRun = !QFile(populatedFlagPath).exists();
    if (firstRun) {
        if (PopulateInitialData()) {
            QFile populatedFlag(populatedFlagPath);
            if (populatedFlag.open(QIODevice::WriteOnly)) {
                populatedFlag.write("populated");
                populatedFlag.close();
            }
        }
    }
    return true;
}

void DataManager::CloseDb() {
    if (db_.isOpen()) {
        QString connectionName = db_.connectionName();
        db_.close();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool DataManager::IsOpen() const { return db_.isOpen(); }

bool DataManager::InitSchema() {
    if (!IsOpen()) {
        lastErrorString_ = "DataManager::InitSchema: База данных не открыта.";
        emit DatabaseError(lastErrorString_);
        return false;
    }
    QSqlQuery query(db_);
    QStringList createTableStatements = {
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE, "
        "email TEXT UNIQUE, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL, "
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "total_study_time_seconds INTEGER DEFAULT 0, "
        "completed_exercises_count INTEGER DEFAULT 0"
        ");",
        "CREATE TABLE IF NOT EXISTS lessons ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE, "
        "description TEXT, "
        "difficulty INTEGER NOT NULL DEFAULT 1"
        ");",
        "CREATE TABLE IF NOT EXISTS questions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "lesson_id INTEGER NOT NULL, "
        "type INTEGER NOT NULL, "
        "text TEXT NOT NULL, "
        "correct_answer TEXT NOT NULL, "
        "options TEXT, "
        "audio_path TEXT, "
        "hint TEXT, "
        "difficulty_order INTEGER DEFAULT 0, "
        "FOREIGN KEY (lesson_id) REFERENCES lessons (id) ON DELETE CASCADE"
        ");",
        "CREATE TABLE IF NOT EXISTS user_lesson_progress ("
        "user_id INTEGER NOT NULL, "
        "lesson_id INTEGER NOT NULL, "
        "stars_translation INTEGER DEFAULT 0, "
        "stars_grammar INTEGER DEFAULT 0, "
        "stars_audio INTEGER DEFAULT 0, "
        "PRIMARY KEY (user_id, lesson_id), "
        "FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE, "
        "FOREIGN KEY (lesson_id) REFERENCES lessons (id) ON DELETE CASCADE"
        ");"};

    if (!db_.transaction()) {
        lastErrorString_ =
            QString(
                "DataManager: Не удалось начать транзакцию для инициализации схемы: %1")
                .arg(db_.lastError().text());
        emit DatabaseError(lastErrorString_);
        return false;
    }
    bool success = true;
    for (const QString& stmt : createTableStatements) {
        if (!query.exec(stmt)) {
            lastErrorString_ =
                QString(
                    "DataManager: Не удалось выполнить оператор схемы: %1\nЗапрос: %2")
                    .arg(query.lastError().text(), stmt);
            emit DatabaseError(lastErrorString_);
            success = false;
            break;
        }
    }
    if (success) {
        if (!db_.commit()) {
            lastErrorString_ =
                QString("DataManager: Не удалось подтвердить транзакцию схемы: %1")
                    .arg(db_.lastError().text());
            emit DatabaseError(lastErrorString_);
            success = false;
        }
    }
    if (!success) {
        db_.rollback();
    }
    return success;
}

long long DataManager::AddUser(const User& userData, const QString& password) {
    if (!IsOpen()) {
        lastErrorString_ = "База данных не открыта для AddUser";
        emit DatabaseError(lastErrorString_);
        return -1;
    }
    if (userData.name_.isEmpty() || password.isEmpty()) {
        lastErrorString_ =
            "Имя пользователя или пароль не могут быть пустыми для AddUser";
        emit DatabaseError(lastErrorString_);
        return -1;
    }
    if (UserExists(userData.name_)) {
        lastErrorString_ =
            "Пользователь с таким именем уже существует: " + userData.name_;
        return -1;
    }

    QByteArray saltBytes(16, 0);
    QRandomGenerator::global()->generate(saltBytes.begin(), saltBytes.end());
    QString saltHex = QString::fromUtf8(saltBytes.toHex());

    QByteArray passwordBytes = password.toUtf8();
    QByteArray saltedPassword = saltBytes + passwordBytes;
    QByteArray hashBytes =
        QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256);
    QString passwordHashHex = QString::fromUtf8(hashBytes.toHex());

    QSqlQuery query(db_);
    query.prepare(
        "INSERT INTO users (name, email, password_hash, salt, created_at, "
        "total_study_time_seconds, completed_exercises_count) "
        "VALUES (:name, :email, :password_hash, :salt, :created_at, 0, 0)");
    query.bindValue(":name", userData.name_);
    query.bindValue(":email",
                    userData.email_.isEmpty() ? QVariant() : userData.email_);
    query.bindValue(":password_hash", passwordHashHex);
    query.bindValue(":salt", saltHex);
    query.bindValue(":created_at",
                    QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        lastErrorString_ =
            QString("DataManager: Не удалось добавить пользователя '%1': %2")
                .arg(userData.name_, query.lastError().text());
        emit DatabaseError(lastErrorString_);
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

bool DataManager::UserExists(const QString& username) const {
    if (!IsOpen() || username.isEmpty()) return false;
    QSqlQuery query(db_);
    query.prepare("SELECT id FROM users WHERE name = :name");
    query.bindValue(":name", username);
    if (query.exec() && query.next()) return true;
    return false;
}

User DataManager::GetUserByUsername(const QString& username) {
    User user;
    if (!IsOpen() || username.isEmpty()) return user;
    QSqlQuery query(db_);
    query.prepare(
        "SELECT id, name, email, password_hash, salt, created_at, "
        "total_study_time_seconds, completed_exercises_count FROM users WHERE "
        "name = :name");
    query.bindValue(":name", username);
    if (query.exec() && query.next()) {
        user.id_ = query.value("id").toLongLong();
        user.name_ = query.value("name").toString();
        user.email_ = query.value("email").toString();
        user.passwordHash_ = query.value("password_hash").toString();
        user.salt_ = query.value("salt").toString();
        user.createdAt_ =
            QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        user.totalStudyTimeSeconds_ =
            query.value("total_study_time_seconds").toLongLong();
        user.completedExercisesCount_ =
            query.value("completed_exercises_count").toInt();
    }
    return user;
}

User DataManager::GetUserById(long long userId) const {
    User user;
    if (!IsOpen() || userId <= 0) return user;
    QSqlQuery query(db_);
    query.prepare(
        "SELECT id, name, email, password_hash, salt, created_at, "
        "total_study_time_seconds, completed_exercises_count FROM users WHERE id "
        "= :id");
    query.bindValue(":id", userId);
    if (query.exec() && query.next()) {
        user.id_ = query.value("id").toLongLong();
        user.name_ = query.value("name").toString();
        user.email_ = query.value("email").toString();
        user.passwordHash_ = query.value("password_hash").toString();
        user.salt_ = query.value("salt").toString();
        user.createdAt_ =
            QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        user.totalStudyTimeSeconds_ =
            query.value("total_study_time_seconds").toLongLong();
        user.completedExercisesCount_ =
            query.value("completed_exercises_count").toInt();
    }
    return user;
}

bool DataManager::CheckPassword(long long userId, const QString& password) {
    if (!IsOpen() || userId <= 0 || password.isEmpty()) return false;
    User user = GetUserById(userId);
    if (!user.IsValid() || user.salt_.isEmpty() ||
        user.passwordHash_.isEmpty())
        return false;

    QByteArray saltBytes = QByteArray::fromHex(user.salt_.toUtf8());
    QByteArray passwordBytes = password.toUtf8();
    QByteArray saltedPassword = saltBytes + passwordBytes;
    QByteArray currentHashBytes =
        QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256);
    QString currentHashHex = QString::fromUtf8(currentHashBytes.toHex());

    return (currentHashHex == user.passwordHash_);
}

bool DataManager::ChangePassword(long long userId,
                                 const QString& newPassword) {
    if (!IsOpen() || userId <= 0 || newPassword.isEmpty()) {
        lastErrorString_ = "Неверные параметры для смены пароля.";
        emit DatabaseError(lastErrorString_);
        return false;
    }

    QByteArray saltBytes(16, 0);
    QRandomGenerator::global()->generate(saltBytes.begin(), saltBytes.end());
    QString saltHex = QString::fromUtf8(saltBytes.toHex());

    QByteArray passwordBytes = newPassword.toUtf8();
    QByteArray saltedPassword = saltBytes + passwordBytes;
    QByteArray hashBytes =
        QCryptographicHash::hash(saltedPassword, QCryptographicHash::Sha256);
    QString passwordHashHex = QString::fromUtf8(hashBytes.toHex());

    QSqlQuery query(db_);
    query.prepare(
        "UPDATE users SET password_hash = :password_hash, salt = :salt WHERE id "
        "= :id");
    query.bindValue(":password_hash", passwordHashHex);
    query.bindValue(":salt", saltHex);
    query.bindValue(":id", userId);

    if (!query.exec()) {
        lastErrorString_ =
            QString("Не удалось сменить пароль для пользователя ID %1: %2")
                .arg(userId)
                .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
        return false;
    }
    return query.numRowsAffected() > 0;
}

void DataManager::SetCurrentUserId(long long userId) {
    if (currentUserId_ != userId) {
        currentUserId_ = userId;
        emit CurrentUserChanged(currentUserId_);
    }
}

long long DataManager::GetCurrentUserId() const { return currentUserId_; }

User DataManager::GetCurrentUser() const {
    if (currentUserId_ > 0) return GetUserById(currentUserId_);
    return User();
}

QList<Lesson> DataManager::GetAllLessonsSorted() {
    QList<Lesson> lessons;
    if (!IsOpen()) return lessons;
    QSqlQuery query(
        "SELECT id, name, description, difficulty FROM lessons ORDER BY "
        "difficulty ASC, name ASC",
        db_);
    if (query.exec()) {
        while (query.next()) {
            Lesson l;
            l.id = query.value("id").toLongLong();
            l.name = query.value("name").toString();
            l.description = query.value("description").toString();
            l.difficulty = query.value("difficulty").toInt();
            lessons.append(l);
        }
    } else {
        lastErrorString_ =
            QString("DataManager: Не удалось получить все уроки: %1")
                .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
    }
    return lessons;
}

Lesson DataManager::GetLessonById(long long lessonId) {
    Lesson lesson;
    if (!IsOpen() || lessonId <= 0) return lesson;
    QSqlQuery query(db_);
    query.prepare(
        "SELECT id, name, description, difficulty FROM lessons WHERE id = :id");
    query.bindValue(":id", lessonId);
    if (query.exec() && query.next()) {
        lesson.id = query.value("id").toLongLong();
        lesson.name = query.value("name").toString();
        lesson.description = query.value("description").toString();
        lesson.difficulty = query.value("difficulty").toInt();
    } else if (query.lastError().isValid()) {
        lastErrorString_ =
            QString("DataManager: Не удалось получить урок по ID %1: %2")
                .arg(lessonId)
                .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
    }
    return lesson;
}

QList<Question> DataManager::GetQuestionsForLesson(long long lessonId,
                                                  ExerciseType type,
                                                  int count) {
    QList<Question> questions;
    if (!IsOpen() || lessonId <= 0) return questions;

    QSqlQuery query(db_);
    QString queryString =
        "SELECT id, lesson_id, type, text, correct_answer, options, "
        "audio_path, hint, difficulty_order "
        "FROM questions WHERE lesson_id = :lesson_id ";
    if (type != ExerciseType::None) {
        queryString += "AND type = :type ";
    }
    queryString += "ORDER BY difficulty_order ASC, RANDOM() ";
    if (count > 0) {
        queryString += "LIMIT :count";
    }

    query.prepare(queryString);
    query.bindValue(":lesson_id", lessonId);
    if (type != ExerciseType::None) {
        query.bindValue(":type", static_cast<int>(type));
    }
    if (count > 0) {
        query.bindValue(":count", count);
    }

    if (query.exec()) {
        while (query.next()) {
            Question q;
            q.id = query.value("id").toLongLong();
            q.lessonId = query.value("lesson_id").toLongLong();
            q.type = static_cast<ExerciseType>(query.value("type").toInt());
            q.text = query.value("text").toString();
            q.correctAnswer = query.value("correct_answer").toString();

            QVariant optionsVariant = query.value("options");
            if (!optionsVariant.isNull()) {
                QString optionsJson = optionsVariant.toString();
                if (!optionsJson.isEmpty() && optionsJson.startsWith('[') &&
                    optionsJson.endsWith(']')) {
                    optionsJson = optionsJson.mid(1, optionsJson.length() - 2);
                    if (!optionsJson.isEmpty()) {
                        QStringList opts =
                            optionsJson.split("\",\"", Qt::SkipEmptyParts);
                        for (const QString& opt : opts) {
                            QString cleanedOpt = opt;
                            if (cleanedOpt.startsWith('\"')) cleanedOpt.remove(0, 1);
                            if (cleanedOpt.endsWith('\"')) cleanedOpt.chop(1);
                            q.options.append(cleanedOpt);
                        }
                    }
                }
            }
            q.audioPath = query.value("audio_path").toString();
            q.hint = query.value("hint").toString();
            q.difficultyOrder = query.value("difficulty_order").toInt();
            questions.append(q);
        }
    } else {
        lastErrorString_ =
            QString("DataManager: Не удалось получить вопросы для урока %1: %2")
                .arg(lessonId)
                .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
    }
    return questions;
}

UserLessonProgress DataManager::GetUserProgressForLesson(long long userId,
                                                         long long lessonId) {
    UserLessonProgress progress;
    progress.userId = userId;
    progress.lessonId = lessonId;
    if (!IsOpen() || userId <= 0 || lessonId <= 0) return progress;

    QSqlQuery query(db_);
    query.prepare(
        "SELECT stars_translation, stars_grammar, stars_audio FROM "
        "user_lesson_progress "
        "WHERE user_id = :user_id AND lesson_id = :lesson_id");
    query.bindValue(":user_id", userId);
    query.bindValue(":lesson_id", lessonId);

    if (query.exec() && query.next()) {
        progress.starsTranslation = query.value("stars_translation").toInt();
        progress.starsGrammar = query.value("stars_grammar").toInt();
        progress.starsAudio = query.value("stars_audio").toInt();
    }
    return progress;
}

bool DataManager::UpdateUserProgressForLesson(
    const UserLessonProgress& progress) {
    if (!IsOpen() || progress.userId <= 0 || progress.lessonId <= 0)
        return false;

    QSqlQuery query(db_);
    query.prepare(
        "INSERT OR REPLACE INTO user_lesson_progress (user_id, lesson_id, "
        "stars_translation, stars_grammar, stars_audio) "
        "VALUES (:user_id, :lesson_id, :stars_translation, :stars_grammar, "
        ":stars_audio)");
    query.bindValue(":user_id", progress.userId);
    query.bindValue(":lesson_id", progress.lessonId);
    query.bindValue(":stars_translation",
                    qBound(0, progress.starsTranslation, 1));
    query.bindValue(":stars_grammar", qBound(0, progress.starsGrammar, 1));
    query.bindValue(":stars_audio", qBound(0, progress.starsAudio, 1));

    if (!query.exec()) {
        lastErrorString_ = QString(
            "DataManager: Не удалось обновить прогресс пользователя для урока %1: %2")
                               .arg(progress.lessonId)
                               .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
        return false;
    }
    return true;
}

QMap<long long, UserLessonProgress> DataManager::GetAllUserProgress(
    long long userId) {
    QMap<long long, UserLessonProgress> allProgress;
    if (!IsOpen() || userId <= 0) return allProgress;

    QSqlQuery query(db_);
    query.prepare(
        "SELECT lesson_id, stars_translation, stars_grammar, stars_audio FROM "
        "user_lesson_progress WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            UserLessonProgress p;
            p.userId = userId;
            p.lessonId = query.value("lesson_id").toLongLong();
            p.starsTranslation = query.value("stars_translation").toInt();
            p.starsGrammar = query.value("stars_grammar").toInt();
            p.starsAudio = query.value("stars_audio").toInt();
            allProgress.insert(p.lessonId, p);
        }
    } else {
        lastErrorString_ = QString(
            "DataManager: Не удалось получить весь прогресс пользователя %1: %2")
                               .arg(userId)
                               .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
    }
    return allProgress;
}

bool DataManager::UpdateUserStudyTime(long long userId,
                                      long long additionalTimeSeconds) {
    if (!IsOpen() || userId <= 0 || additionalTimeSeconds < 0) return false;
    if (additionalTimeSeconds == 0) return true;

    QSqlQuery query(db_);
    query.prepare(
        "UPDATE users SET total_study_time_seconds = "
        "total_study_time_seconds + :time WHERE id = :id");
    query.bindValue(":time", additionalTimeSeconds);
    query.bindValue(":id", userId);

    if (!query.exec()) {
        lastErrorString_ =
            QString(
                "DataManager: Не удалось обновить время обучения для пользователя %1: %2")
                .arg(userId)
                .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
        return false;
    }
    return true;
}

bool DataManager::IncrementUserCompletedExercises(long long userId) {
    if (!IsOpen() || userId <= 0) return false;
    QSqlQuery query(db_);
    query.prepare(
        "UPDATE users SET completed_exercises_count = "
        "completed_exercises_count + 1 WHERE id = :id");
    query.bindValue(":id", userId);
    if (!query.exec()) {
        lastErrorString_ = QString(
            "DataManager: Не удалось увеличить счетчик выполненных упражнений для "
            "пользователя %1: %2")
                               .arg(userId)
                               .arg(query.lastError().text());
        emit DatabaseError(lastErrorString_);
        return false;
    }
    return true;
}

long long DataManager::addLessonInternal(const QString& name,
                                         const QString& description,
                                         int difficulty) {
    QSqlQuery query(db_);
    query.prepare(
        "INSERT INTO lessons (name, description, difficulty) VALUES (:name, "
        ":desc, :diff)");
    query.bindValue(":name", name);
    query.bindValue(":desc", description.isEmpty() ? QVariant() : description);
    query.bindValue(":diff", difficulty);
    if (!query.exec()) {
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

bool DataManager::addQuestionInternal(long long lessonId, ExerciseType type,
                                      const QString& text,
                                      const QString& correctAnswer,
                                      const QList<QString>& options,
                                      const QString& audioPath,
                                      const QString& hint,
                                      int difficultyOrder) {
    QSqlQuery query(db_);
    query.prepare(
        "INSERT INTO questions (lesson_id, type, text, correct_answer, "
        "options, audio_path, hint, difficulty_order) "
        "VALUES (:lesson_id, :type, :text, :correct_answer, :options, "
        ":audio_path, :hint, :diff_order)");
    query.bindValue(":lesson_id", lessonId);
    query.bindValue(":type", static_cast<int>(type));
    query.bindValue(":text", text);
    query.bindValue(":correct_answer", correctAnswer);

    QString optionsJsonString = "[]";
    if (!options.isEmpty()) {
        optionsJsonString = "[";
        for (int i = 0; i < options.size(); ++i) {
            QString currentOption = options[i];
            optionsJsonString += "\"" + currentOption.replace("\"", "\\\"") + "\"";
            if (i < options.size() - 1) optionsJsonString += ",";
        }
        optionsJsonString += "]";
    }
    query.bindValue(":options", optionsJsonString);
    query.bindValue(":audio_path", audioPath.isEmpty() ? QVariant() : audioPath);
    query.bindValue(":hint", hint.isEmpty() ? QVariant() : hint);
    query.bindValue(":diff_order", difficultyOrder);

    if (!query.exec()) {
        return false;
    }
    return true;
}

bool DataManager::PopulateInitialData() {
    if (!IsOpen()) {
        lastErrorString_ = "PopulateInitialData: База данных не открыта.";
        emit DatabaseError(lastErrorString_);
        return false;
    }
    if (!db_.transaction()) {
        lastErrorString_ = "PopulateInitialData: Не удалось начать транзакцию.";
        emit DatabaseError(lastErrorString_);
        return false;
    }
    bool success = true;

    auto createLesson =
        [&](const QString& name, const QString& desc, int diff,
            const QList<QList<QVariant>>& questionsData) {
        if (!success) return;
        long long lessonId = addLessonInternal(name, desc, diff);
        if (lessonId <= 0) {
            success = false;
            lastErrorString_ =
                "PopulateInitialData: Не удалось создать урок '" + name +
                "'. Ошибка БД: " + db_.lastError().text();
            emit DatabaseError(lastErrorString_);
            return;
        }
        for (const auto& qData : questionsData) {
            if (!success) break;
            if (qData.size() < 7) {
                continue;
            }
            QStringList currentOptions;
            if (qData[3].canConvert<QStringList>()) {
                currentOptions = qData[3].toStringList();
            } else if (qData[3].typeId() == QMetaType::QString) {
                if (!qData[3].toString().isEmpty()) {
                    currentOptions.append(qData[3].toString());
                }
            }

            success &= addQuestionInternal(
                lessonId, static_cast<ExerciseType>(qData[0].toInt()),
                qData[1].toString(), qData[2].toString(), currentOptions,
                qData[4].toString(), qData[5].toString(), qData[6].toInt());
            if (!success) {
                lastErrorString_ =
                    "PopulateInitialData: Не удалось добавить вопрос к уроку '" +
                    name + "'. Ошибка БД: " + db_.lastError().text();
                emit DatabaseError(lastErrorString_);
            }
        }
    };

    createLesson("Базовые слова 1 (En)", "Basic Nouns and Verbs.", 1,
                 QList<QList<QVariant>>{
                     {static_cast<int>(ExerciseType::Translation), "Собака", "Dog",
                      QStringList{}, QString("qrc:/audio/dog.mp3"), "Woof-woof!",
                      1},
                     {static_cast<int>(ExerciseType::Translation), "Кошка", "Cat",
                      QStringList{}, QString("qrc:/audio/cat.mp3"), "Meow.", 2},
                     {static_cast<int>(ExerciseType::Translation), "Дом", "House",
                      QStringList{}, QString(), "Where do you live?", 3},
                     {static_cast<int>(ExerciseType::Grammar),
                      "This is ___ apple.", "an",
                      QStringList{"a", "an", "the", "-"}, QString(),
                      "Choose the correct article.", 4},
                     {static_cast<int>(ExerciseType::Audio), "", "Man",
                      QStringList{}, QString("qrc:/audio/man.mp3"),
                      "Listen and type.", 5}});
    createLesson(
        "Базовые слова 2 (En)", "More simple words.", 1,
        QList<QList<QVariant>>{
            {static_cast<int>(ExerciseType::Translation), "Книга", "Book",
             QStringList{}, QString(), "What are you reading?", 1},
            {static_cast<int>(ExerciseType::Translation), "Стол", "Table",
             QStringList{}, QString(), "Furniture.", 2},
            {static_cast<int>(ExerciseType::Translation), "Вода", "Water",
             QStringList{}, QString("qrc:/audio/water.mp3"), "I'm thirsty.",
             3},
            {static_cast<int>(ExerciseType::Grammar), "She ___ fast.", "runs",
             QStringList{"run", "runs", "running"}, QString(),
             "Verb form for 'she'.", 4},
            {static_cast<int>(ExerciseType::Audio), "", "Woman", QStringList{},
             QString("qrc:/audio/woman.mp3"), "Listen and type.", 5}});
    createLesson(
        "Цвета (En)", "Learning colors.", 2,
        QList<QList<QVariant>>{
            {static_cast<int>(ExerciseType::Translation), "Красный", "Red",
             QStringList{}, QString(), "Apple's color.", 1},
            {static_cast<int>(ExerciseType::Translation), "Синий", "Blue",
             QStringList{}, QString("qrc:/audio/blue.mp3"), "Sky's color.", 2},
            {static_cast<int>(ExerciseType::Translation), "Зеленый", "Green",
             QStringList{}, QString(), "", 3},
            {static_cast<int>(ExerciseType::Grammar), "It is a ___ car.", "red",
             QStringList{"car red", "red car", "red", "a red"}, QString(),
             "Adjective placement.", 4}});
    createLesson(
        "Семья (En)", "Family members.", 2,
        QList<QList<QVariant>>{
            {static_cast<int>(ExerciseType::Translation), "Мама", "Mother",
             QStringList{}, QString("qrc:/audio/mother.mp3"),
             "Closest person.", 1},
            {static_cast<int>(ExerciseType::Translation), "Папа", "Father",
             QStringList{}, QString(), "Head of the family.", 2},
            {static_cast<int>(ExerciseType::Translation), "Брат", "Brother",
             QStringList{}, QString(), "Male sibling.", 3},
            {static_cast<int>(ExerciseType::Grammar), "He is my ___.",
             "brother", QStringList{"brother", "sister", "father"}, QString(),
             "Male family member.", 4}});
    createLesson("Еда 1 (En)", "Basic food items.", 3,
                 {
                     {static_cast<int>(ExerciseType::Translation), "Хлеб", "Bread",
                      QStringList{}, QString(), "Base of many meals.", 1},
                     {static_cast<int>(ExerciseType::Translation), "Молоко", "Milk",
                      QStringList{}, QString("qrc:/audio/milk.mp3"),
                      "White drink.", 2},
                     {static_cast<int>(ExerciseType::Translation), "Яблоко", "Apple",
                      QStringList{}, QString(), "Popular fruit.", 3},
                     {static_cast<int>(ExerciseType::Grammar), "We ___ soup now.",
                      "are eating",
                      QStringList{"eat", "eats", "are eating", "is eating"},
                      QString(), "Action happening now.", 4},
                     {static_cast<int>(ExerciseType::Audio), "", "Cheese",
                      QStringList{}, QString("qrc:/audio/cheese.mp3"),
                      "Dairy product.", 5},
                 });
    createLesson("Животные 1 (En)", "Farm animals.", 3,
                 {
                     {static_cast<int>(ExerciseType::Translation), "Лошадь", "Horse",
                      QStringList{}, QString(), "It gallops.", 1},
                     {static_cast<int>(ExerciseType::Translation), "Корова", "Cow",
                      QStringList{}, QString("qrc:/audio/cow.mp3"),
                      "Gives milk.", 2},
                     {static_cast<int>(ExerciseType::Grammar), "A cow ___ moo.",
                      "says", QStringList{"say", "says", "is saying"},
                      QString(), "Sound a cow makes.", 3},
                 });
    createLesson("Числа до 10 (En)", "Learning to count.", 4,
                 {
                     {static_cast<int>(ExerciseType::Translation), "Один", "One",
                      QStringList{}, QString("qrc:/audio/one.mp3"),
                      "Start of counting.", 1},
                     {static_cast<int>(ExerciseType::Translation), "Два", "Two",
                      QStringList{}, QString(), "", 2},
                     {static_cast<int>(ExerciseType::Grammar),
                      "I have two ___.", "apples",
                      QStringList{"apple", "apples", "apple's"}, QString(),
                      "Plural form.", 3},
                     {static_cast<int>(ExerciseType::Translation), "Пять", "Five",
                      QStringList{}, QString("qrc:/audio/five.mp3"), "", 4},
                 });
    createLesson(
        "Простые действия (En)", "Basic action verbs.", 4,
        {
            {static_cast<int>(ExerciseType::Translation), "Идти", "Go",
             QStringList{}, QString(), "Move on foot.", 1},
            {static_cast<int>(ExerciseType::Translation), "Бежать", "Run",
             QStringList{}, QString("qrc:/audio/run.mp3"), "Go fast.", 2},
            {static_cast<int>(ExerciseType::Grammar),
             "He ___ a book every day.", "reads",
             QStringList{"read", "reads", "is reading"}, QString(),
             "Present Simple tense.", 3},
            {static_cast<int>(ExerciseType::Grammar),
             "She ___ to the park yesterday.", "went",
             QStringList{"go", "goes", "went", "gone"}, QString(),
             "Action in the past.", 4},
        });
    createLesson(
        "Английская грамматика 1", "Основные конструкции.", 3,
        QList<QList<QVariant>>{
            {static_cast<int>(ExerciseType::Grammar),
             "There ___ many books on the table.", "are",
             QStringList{"is", "are", "was", "were"}, QString(),
             "'There is/are' construction.", 1},
            {static_cast<int>(ExerciseType::Grammar),
             "I ___ play football when I was young.", "used to",
             QStringList{"use to", "used to", "am used to"}, QString(),
             "Past habits.", 2},
            {static_cast<int>(ExerciseType::Grammar),
             "___ you like a cup of tea?", "Would",
             QStringList{"Do", "Are", "Would", "Will"}, QString(),
             "Polite offer.", 3},
            {static_cast<int>(ExerciseType::Grammar),
             "This book is ___ than that one.", "more interesting",
             QStringList{"interesting", "more interesting", "interestinger",
                         "most interesting"},
             QString(), "Comparative adjectives.", 4},
            {static_cast<int>(ExerciseType::Grammar),
             "If I ___ you, I would study harder.", "were",
             QStringList{"am", "was", "were", "be"}, QString(),
             "Second conditional.", 5}});

    if (success) {
        if (!db_.commit()) {
            lastErrorString_ = "PopulateInitialData: Не удалось подтвердить транзакцию: " +
                               db_.lastError().text();
            emit DatabaseError(lastErrorString_);
            success = false;
        }
    }
    if (!success) {
        db_.rollback();
    }
    return success;
}
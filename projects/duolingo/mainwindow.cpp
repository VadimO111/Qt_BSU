#include "mainwindow.h"
#include "audioexercisewidget.h"
#include "datamanager.h"
#include "leveldata.h"
#include "userstatsdialog.h"

#include <QAction>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QtMultimedia/QAudioOutput>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <algorithm>

class LevelInfoWidget : public QWidget {
public:
    QLabel* infoLabel;
    explicit LevelInfoWidget(QWidget* p = nullptr)
        : QWidget(p), infoLabel(nullptr) {
        infoLabel = new QLabel("Выберите уровень и тип упражнения.", this);
        infoLabel->setAlignment(Qt::AlignCenter);
        infoLabel->setWordWrap(true);
        QVBoxLayout* l = new QVBoxLayout(this);
        l->addWidget(infoLabel, 0, Qt::AlignCenter);
        setLayout(l);
    }
    void setLevelInfo(const QString& name, const QString& desc) {
        if (infoLabel) {
            infoLabel->setText(
                QString("<h2>%1</h2><p>%2</p><p>Выберите тип упражнения "
                        "слева.</p>")
                    .arg(name, desc));
        }
    }
};

class TranslationExerciseWidget : public QWidget {
public:
    QLabel* questionLabel;
    QLineEdit* answerEdit;
    QPushButton* submitButton;
    QLabel* correctAnswerFeedbackLabel;

    explicit TranslationExerciseWidget(QWidget* p = nullptr)
        : QWidget(p),
          questionLabel(nullptr),
          answerEdit(nullptr),
          submitButton(nullptr),
          correctAnswerFeedbackLabel(nullptr) {
        QVBoxLayout* l = new QVBoxLayout(this);
        questionLabel = new QLabel("Переведите:", this);
        questionLabel->setAlignment(Qt::AlignCenter);
        questionLabel->setWordWrap(true);
        answerEdit = new QLineEdit(this);
        correctAnswerFeedbackLabel = new QLabel(this);
        correctAnswerFeedbackLabel->setWordWrap(true);
        correctAnswerFeedbackLabel->setVisible(false);
        submitButton = new QPushButton("Проверить", this);
        l->addWidget(questionLabel);
        l->addWidget(answerEdit);
        l->addWidget(correctAnswerFeedbackLabel);
        l->addStretch();
        l->addWidget(submitButton, 0, Qt::AlignRight);
        setLayout(l);
    }
    void setQuestion(const Question& q) {
        if (questionLabel)
            questionLabel->setText(QString("Переведите: <b>%1</b>").arg(q.text));
        if (answerEdit) {
            answerEdit->clear();
            answerEdit->setReadOnly(false);
            answerEdit->setFocus();
        }
        if (submitButton) submitButton->setEnabled(true);
        if (correctAnswerFeedbackLabel) {
            correctAnswerFeedbackLabel->setVisible(false);
            correctAnswerFeedbackLabel->setText("");
        }
    }
    QString getAnswer() const {
        return answerEdit ? answerEdit->text().trimmed() : QString();
    }

    void showCorrectAnswer(const QString& correctAnswer, bool wasUserCorrect) {
        if (!correctAnswerFeedbackLabel) return;
        if (wasUserCorrect) {
            correctAnswerFeedbackLabel->setStyleSheet(
                "color: green; font-weight: bold;");
            correctAnswerFeedbackLabel->setText("Верно!");
        } else {
            correctAnswerFeedbackLabel->setStyleSheet("color: red;");
            correctAnswerFeedbackLabel->setText(
                QString("Неверно. Правильный ответ: <b>%1</b>")
                    .arg(correctAnswer));
        }
        correctAnswerFeedbackLabel->setVisible(true);
    }
};

class GrammarExerciseWidget : public QWidget {
public:
    QLabel* questionLabel;
    QVBoxLayout* optionsButtonsLayout_;
    QList<QPushButton*> optionButtons_;
    QLabel* correctAnswerFeedbackLabel;

    explicit GrammarExerciseWidget(QWidget* p = nullptr)
        : QWidget(p),
          questionLabel(nullptr),
          optionsButtonsLayout_(nullptr),
          correctAnswerFeedbackLabel(nullptr) {
        QVBoxLayout* l = new QVBoxLayout(this);
        questionLabel = new QLabel("Выберите правильный вариант:", this);
        questionLabel->setAlignment(Qt::AlignCenter);
        questionLabel->setWordWrap(true);
        optionsButtonsLayout_ = new QVBoxLayout();
        optionsButtonsLayout_->setSpacing(10);
        correctAnswerFeedbackLabel = new QLabel(this);
        correctAnswerFeedbackLabel->setWordWrap(true);
        correctAnswerFeedbackLabel->setVisible(false);
        l->addWidget(questionLabel);
        l->addLayout(optionsButtonsLayout_);
        l->addWidget(correctAnswerFeedbackLabel);
        l->addStretch();
        setLayout(l);
    }
    void setQuestion(const Question& q, MainWindow* mainWindowConnect) {
        if (questionLabel) {
            QString questionTextWithVisibleUnderscore = q.text;
            questionLabel->setText(questionTextWithVisibleUnderscore.replace(
                "___", "<u>      </u>"));
        }
        if (!optionsButtonsLayout_) return;
        qDeleteAll(optionButtons_);
        optionButtons_.clear();
        QLayoutItem* item;
        while ((item = optionsButtonsLayout_->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->setParent(nullptr);
                delete item->widget();
            }
            delete item;
        }
        for (const QString& opt : q.options) {
            QPushButton* btn = new QPushButton(opt, this);
            btn->setCheckable(false);
            if (mainWindowConnect) {
                connect(btn, &QPushButton::clicked, mainWindowConnect,
                        &MainWindow::grammarOptionClicked);
            }
            optionsButtonsLayout_->addWidget(btn);
            optionButtons_.append(btn);
        }
        if (correctAnswerFeedbackLabel) {
            correctAnswerFeedbackLabel->setVisible(false);
            correctAnswerFeedbackLabel->setText("");
        }
        enableOptionButtons();
    }
    void showCorrectAnswer(const QString& correctAnswer, bool wasUserCorrect,
                           const QString& userAnswer) {
        if (!correctAnswerFeedbackLabel) return;
        if (wasUserCorrect) {
            correctAnswerFeedbackLabel->setStyleSheet(
                "color: green; font-weight: bold;");
            correctAnswerFeedbackLabel->setText(
                QString("Верно: <b>%1</b>").arg(userAnswer));
        } else {
            correctAnswerFeedbackLabel->setStyleSheet("color: red;");
            correctAnswerFeedbackLabel->setText(
                QString("Вы выбрали: %1<br>Правильный ответ: <b>%2</b>")
                    .arg(userAnswer, correctAnswer));
        }
        correctAnswerFeedbackLabel->setVisible(true);
        for (QPushButton* btn : optionButtons_) {
            if (btn) btn->setEnabled(false);
        }
    }
    void enableOptionButtons() {
        for (QPushButton* btn : optionButtons_) {
            if (btn) btn->setEnabled(true);
        }
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      lives_(MAX_LIVES),
      questionsInCurrentSet_(QUESTIONS_PER_SET),
      timePerExerciseSetSeconds_(120) {
    for (int i = 0; i < 3; ++i) starLabels_[i] = nullptr;

    setupUi();
    setupMenuBar();
    populateLevelMenu();

    QList<Lesson> lessons = DataManager::Instance().GetAllLessonsSorted();
    if (!lessons.isEmpty()) {
        loadLevel(lessons.first().id);
    } else {
        loadLevel(-1);
    }

    exerciseTimer_ = new QTimer(this);
    connect(exerciseTimer_, &QTimer::timeout, this,
            &MainWindow::updateTimerDisplay);

    sessionActivityTimer_.start();
}

MainWindow::~MainWindow() {
    long long userId = DataManager::Instance().GetCurrentUserId();
    if (userId > 0 && sessionActivityTimer_.isValid()) {
        DataManager::Instance().UpdateUserStudyTime(
            userId, sessionActivityTimer_.elapsed() / 1000);
    }
}

void MainWindow::setupUi() {
    setWindowTitle("Duolingo Clone");
    setMinimumSize(800, 600);

    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter_);

    leftPanelWidget_ = new QWidget(mainSplitter_);
    leftPanelWidget_->setObjectName("leftPanel");
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanelWidget_);
    leftLayout->setContentsMargins(15, 15, 15, 15);
    leftLayout->setSpacing(20);

    currentLevelNameLabel_ = new QLabel("Уровень не выбран", leftPanelWidget_);
    currentLevelNameLabel_->setObjectName("levelNameLabel");
    currentLevelNameLabel_->setAlignment(Qt::AlignCenter);
    currentLevelNameLabel_->setWordWrap(true);
    currentLevelNameLabel_->setStyleSheet("font-size: 16px; font-weight: bold;");

    starsLayout_ = new QHBoxLayout();
    starsLayout_->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < 3; ++i) {
        starLabels_[i] = new QLabel("☆", leftPanelWidget_);
        starLabels_[i]->setObjectName("starLabel");
        starLabels_[i]->setStyleSheet("font-size: 28px; color: grey;");
        starsLayout_->addWidget(starLabels_[i]);
    }
    updateStarsDisplay(ExerciseType::None, 0);

    QWidget* exerciseButtonsWidget = new QWidget(leftPanelWidget_);
    QVBoxLayout* exerciseButtonsLayout = new QVBoxLayout(exerciseButtonsWidget);
    exerciseButtonsLayout->setSpacing(10);

    translationButton_ = new QPushButton("Перевод", exerciseButtonsWidget);
    grammarButton_ = new QPushButton("Грамматика", exerciseButtonsWidget);
    audioButton_ = new QPushButton("Аудирование", exerciseButtonsWidget);

    connect(translationButton_, &QPushButton::clicked, this,
            &MainWindow::startTranslationExercise);
    connect(grammarButton_, &QPushButton::clicked, this,
            &MainWindow::startGrammarExercise);
    connect(audioButton_, &QPushButton::clicked, this,
            &MainWindow::startAudioExercise);

    exerciseButtonsLayout->addWidget(translationButton_);
    exerciseButtonsLayout->addWidget(grammarButton_);
    exerciseButtonsLayout->addWidget(audioButton_);
    exerciseButtonsWidget->setLayout(exerciseButtonsLayout);

    leftLayout->addWidget(currentLevelNameLabel_, 1);
    leftLayout->addLayout(starsLayout_);
    leftLayout->addStretch(2);
    leftLayout->addWidget(exerciseButtonsWidget, 2);
    leftPanelWidget_->setLayout(leftLayout);

    rightPanelWidget_ = new QWidget(mainSplitter_);
    rightPanelWidget_->setObjectName("rightPanel");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanelWidget_);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    rightLayout->setSpacing(15);

    exerciseProgressBar_ = new QProgressBar(rightPanelWidget_);
    exerciseProgressBar_->setRange(0, QUESTIONS_PER_SET);
    exerciseProgressBar_->setValue(0);
    exerciseProgressBar_->setTextVisible(true);
    exerciseProgressBar_->setFormat("%v / %m");
    exerciseProgressBar_->setObjectName("exerciseProgressBar");

    exerciseStackedWidget_ = new QStackedWidget(rightPanelWidget_);

    levelInfoWidget_ = new LevelInfoWidget(exerciseStackedWidget_);
    translationExerciseWidget_ =
        new TranslationExerciseWidget(exerciseStackedWidget_);
    grammarExerciseWidget_ = new GrammarExerciseWidget(exerciseStackedWidget_);
    audioExerciseWidget_ = new AudioExerciseWidget(exerciseStackedWidget_);

    exerciseStackedWidget_->addWidget(levelInfoWidget_);
    exerciseStackedWidget_->addWidget(translationExerciseWidget_);
    exerciseStackedWidget_->addWidget(grammarExerciseWidget_);
    exerciseStackedWidget_->addWidget(audioExerciseWidget_);

    if (translationExerciseWidget_ && translationExerciseWidget_->submitButton)
        connect(translationExerciseWidget_->submitButton, &QPushButton::clicked,
                this, &MainWindow::submitAnswerClicked);

    if (audioExerciseWidget_ && audioExerciseWidget_->submitButton_) {
        connect(audioExerciseWidget_->submitButton_, &QPushButton::clicked, this,
                &MainWindow::submitAnswerClicked);
    }

    rightLayout->addWidget(exerciseProgressBar_);
    rightLayout->addWidget(exerciseStackedWidget_, 1);
    rightPanelWidget_->setLayout(rightLayout);

    mainSplitter_->addWidget(leftPanelWidget_);
    mainSplitter_->addWidget(rightPanelWidget_);
    mainSplitter_->setStretchFactor(0, 1);
    mainSplitter_->setStretchFactor(1, 3);
    mainSplitter_->setSizes({250, 550});

    if (!mainSplitter_ || !leftPanelWidget_ || !currentLevelNameLabel_ ||
        !starsLayout_ || !starLabels_[0] || !starLabels_[1] ||
        !starLabels_[2] || !translationButton_ || !grammarButton_ ||
        !audioButton_ || !rightPanelWidget_ || !exerciseProgressBar_ ||
        !exerciseStackedWidget_ || !levelInfoWidget_ ||
        !translationExerciseWidget_ || !grammarExerciseWidget_ ||
        !audioExerciseWidget_) {
        qFatal(
            "Критический элемент UI не удалось инициализировать в setupUi. "
            "Приложение будет закрыто.");
    }
}

void MainWindow::setupMenuBar() {
    QMenuBar* mainMenuBar = menuBar();
    if (!mainMenuBar) {
        mainMenuBar = new QMenuBar(this);
        setMenuBar(mainMenuBar);
    }

    levelsMenu_ = new QMenu("Уровни", mainMenuBar);
    mainMenuBar->addMenu(levelsMenu_);

    QToolBar* rightToolBar = addToolBar("UserToolbar");
    rightToolBar->setObjectName("UserToolbar");
    rightToolBar->setMovable(false);
    rightToolBar->setFloatable(false);

    QWidget* spacer = new QWidget(rightToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    rightToolBar->addWidget(spacer);

    userProfileButton_ = new QToolButton(rightToolBar);
    userProfileButton_->setText("👤");
    userProfileButton_->setToolTip("Профиль пользователя");
    userProfileButton_->setFocusPolicy(Qt::NoFocus);
    userProfileButton_->setObjectName("userProfileButton");
    userProfileButton_->setFixedSize(32, 32);
    userProfileButton_->setStyleSheet(
        "QToolButton { font-size: 20px; border: 1px solid grey; border-radius: "
        "15px; } QToolButton:hover { background-color: lightgrey; }");

    connect(userProfileButton_, &QToolButton::clicked, this,
            &MainWindow::userProfileClicked);
    rightToolBar->addWidget(userProfileButton_);

    if (!levelsMenu_ || !userProfileButton_) {
        qFatal(
            "Критический элемент меню не удалось инициализировать в setupMenuBar. "
            "Приложение будет закрыто.");
    }
}

void MainWindow::populateLevelMenu() {
    if (!levelsMenu_) {
        return;
    }
    levelsMenu_->clear();

    DataManager& db = DataManager::Instance();
    long long currentUserId = db.GetCurrentUserId();

    QList<Lesson> lessons = db.GetAllLessonsSorted();
    QMap<long long, UserLessonProgress> userProgressMap;
    if (currentUserId > 0) {
        userProgressMap = db.GetAllUserProgress(currentUserId);
    }

    if (lessons.isEmpty()) {
        levelsMenu_->addAction("Нет доступных уровней")->setEnabled(false);
        return;
    }

    for (const Lesson& lesson : lessons) {
        UserLessonProgress progress = userProgressMap.value(lesson.id);
        int menuStars = progress.totalStarsForMenu();
        QString starsString;
        for (int i = 0; i < 3; ++i) {
            starsString += (i < menuStars ? QChar(0x2605) : QChar(0x2606));
        }

        QAction* levelAction =
            new QAction(QString("%1 %2").arg(lesson.name).arg(starsString), this);
        levelAction->setData(lesson.id);
        connect(levelAction, &QAction::triggered, this,
                &MainWindow::selectLevelTriggered);
        levelsMenu_->addAction(levelAction);
    }
}

void MainWindow::selectLevelTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        bool ok;
        long long lessonId = action->data().toLongLong(&ok);
        if (ok && lessonId > 0) {
            loadLevel(lessonId);
        }
    }
}

void MainWindow::loadLevel(long long lessonId) {
    resetExerciseState();
    currentSelectedLevelId_ = lessonId;
    DataManager& db = DataManager::Instance();

    if (!currentLevelNameLabel_ || !exerciseStackedWidget_ ||
        !levelInfoWidget_ || !exerciseProgressBar_) {
        return;
    }

    if (lessonId <= 0) {
        currentLesson_ = Lesson();
        currentLevelNameLabel_->setText("Уровень не выбран");
        if (levelInfoWidget_) {
            exerciseStackedWidget_->setCurrentWidget(levelInfoWidget_);
            levelInfoWidget_->setLevelInfo(
                "Нет данных", "Пожалуйста, выберите уровень из меню.");
        }
        updateOverallLevelStarsDisplay(-1);
        if (exerciseProgressBar_) {
            exerciseProgressBar_->setValue(0);
            exerciseProgressBar_->setRange(0, 1);
        }
    } else {
        currentLesson_ = db.GetLessonById(lessonId);
        if (!currentLesson_.IsValid()) {
            currentLevelNameLabel_->setText("Ошибка загрузки уровня");
            if (levelInfoWidget_) {
                exerciseStackedWidget_->setCurrentWidget(levelInfoWidget_);
                levelInfoWidget_->setLevelInfo(
                    "Ошибка", "Не удалось загрузить данные уровня.");
            }
            updateOverallLevelStarsDisplay(lessonId);
        } else {
            currentLevelNameLabel_->setText(currentLesson_.name);
            displayLevelInfo(currentLesson_);
            updateOverallLevelStarsDisplay(lessonId);
        }
    }
}

void MainWindow::displayLevelInfo(const Lesson& lesson) {
    if (!exerciseStackedWidget_ || !levelInfoWidget_) {
        return;
    }
    exerciseStackedWidget_->setCurrentWidget(levelInfoWidget_);
    levelInfoWidget_->setLevelInfo(lesson.name, lesson.description);
}

void MainWindow::updateStarsDisplay(ExerciseType type, int stars) {
    Q_UNUSED(type);
    for (int i = 0; i < 3; ++i) {
        if (starLabels_[i]) {
            starLabels_[i]->setText(i < stars ? QChar(0x2605) : QChar(0x2606));
            starLabels_[i]->setStyleSheet(QString("font-size: 28px; color: %1;")
                                             .arg(i < stars ? "gold" : "grey"));
        }
    }
}
void MainWindow::updateOverallLevelStarsDisplay(long long lessonId) {
    if (lessonId <= 0) {
        updateStarsDisplay(ExerciseType::None, 0);
        return;
    }
    long userId = DataManager::Instance().GetCurrentUserId();
    if (userId <= 0) {
        updateStarsDisplay(ExerciseType::None, 0);
        return;
    }
    UserLessonProgress progress =
        DataManager::Instance().GetUserProgressForLesson(userId, lessonId);
    updateStarsDisplay(ExerciseType::None, progress.totalStarsForMenu());
}

void MainWindow::userProfileClicked() {
    User currentUser = DataManager::Instance().GetCurrentUser();
    if (!currentUser.IsValid()) {
        QMessageBox::warning(
            this, "Ошибка",
            "Не удалось получить данные пользователя. Возможно, вы не вошли в "
            "систему.");
        return;
    }

    if (sessionActivityTimer_.isValid()) {
        long long elapsedSeconds = sessionActivityTimer_.restart() / 1000;
        if (elapsedSeconds > 0) {
            DataManager::Instance().UpdateUserStudyTime(currentUser.id_,
                                                        elapsedSeconds);
            currentUser = DataManager::Instance().GetUserById(currentUser.id_);
        }
    } else {
        sessionActivityTimer_.start();
    }

    UserStatsDialog statsDialog(currentUser, this);
    statsDialog.exec();
}

void MainWindow::startTranslationExercise() {
    startExerciseSet(ExerciseType::Translation);
}
void MainWindow::startGrammarExercise() {
    startExerciseSet(ExerciseType::Grammar);
}
void MainWindow::startAudioExercise() {
    startExerciseSet(ExerciseType::Audio);
}

void MainWindow::startExerciseSet(ExerciseType type) {
    if (currentSelectedLevelId_ <= 0) {
        QMessageBox::information(
            this, "Уровень не выбран",
            "Пожалуйста, сначала выберите уровень из меню.");
        return;
    }

    DataManager& db = DataManager::Instance();
    User currentUser = db.GetCurrentUser();
    if (!currentUser.IsValid()) {
        QMessageBox::warning(
            this, "Ошибка пользователя",
            "Не удалось получить данные текущего пользователя.");
        return;
    }

    resetExerciseState();
    currentExerciseType_ = type;

    currentQuestionSet_ =
        db.GetQuestionsForLesson(currentSelectedLevelId_, type, 0);

    if (!currentQuestionSet_.isEmpty()) {
        std::shuffle(currentQuestionSet_.begin(), currentQuestionSet_.end(),
                     *QRandomGenerator::global());
        if (currentQuestionSet_.size() > QUESTIONS_PER_SET) {
            currentQuestionSet_ = currentQuestionSet_.mid(0, QUESTIONS_PER_SET);
        }
    }

    if (currentQuestionSet_.isEmpty()) {
        QMessageBox::information(
            this, "Нет вопросов",
            "Для этого типа упражнений в данном уровне нет доступных вопросов.");
        currentExerciseType_ = ExerciseType::None;
        if (currentLesson_.IsValid()) displayLevelInfo(currentLesson_);
        return;
    }

    questionsInCurrentSet_ = currentQuestionSet_.size();
    currentQuestionIndex_ = 0;
    correctAnswersInSet_ = 0;
    lives_ = MAX_LIVES;

    if (!exerciseProgressBar_) {
        return;
    }
    exerciseProgressBar_->setRange(0, questionsInCurrentSet_);
    updateProgressBar();
    updateExercise();

    exerciseSetTimer_.start();
    remainingTimeSeconds_ = timePerExerciseSetSeconds_;

    if (!exerciseTimer_) {
        return;
    }
    exerciseTimer_->start(1000);
    updateTimerDisplay();
    if (statusBar()) statusBar()->show();
}

void MainWindow::resetExerciseState() {
    currentExerciseType_ = ExerciseType::None;
    currentQuestionSet_.clear();
    currentQuestionIndex_ = -1;
    lives_ = MAX_LIVES;
    correctAnswersInSet_ = 0;

    if (exerciseTimer_ && exerciseTimer_->isActive()) {
        exerciseTimer_->stop();
    }
    remainingTimeSeconds_ = 0;
    if (statusBar()) statusBar()->clearMessage();

    if (exerciseProgressBar_) {
        exerciseProgressBar_->setValue(0);
        exerciseProgressBar_->setRange(0, QUESTIONS_PER_SET);
        exerciseProgressBar_->setFormat("%v / %m");
    }

    if (currentSelectedLevelId_ > 0) {
        updateOverallLevelStarsDisplay(currentSelectedLevelId_);
    } else {
        updateStarsDisplay(ExerciseType::None, 0);
    }

    if (exerciseStackedWidget_ && levelInfoWidget_) {
        exerciseStackedWidget_->setCurrentWidget(levelInfoWidget_);
        if (currentLesson_.IsValid()) {
            displayLevelInfo(currentLesson_);
        } else {
            levelInfoWidget_->setLevelInfo(
                "Уровень не выбран",
                "Пожалуйста, выберите уровень из меню.");
        }
    }
}

void MainWindow::updateExercise() {
    if (!exerciseStackedWidget_ || !translationExerciseWidget_ ||
        !grammarExerciseWidget_ || !audioExerciseWidget_ || !levelInfoWidget_) {
        QMessageBox::critical(this, "Ошибка интерфейса",
                              "Не удалось обновить упражнение. Попробуйте снова.");
        resetExerciseState();
        return;
    }

    if (currentQuestionIndex_ >= 0 &&
        currentQuestionIndex_ < currentQuestionSet_.size()) {
        const Question& q = currentQuestionSet_.at(currentQuestionIndex_);
        switch (currentExerciseType_) {
            case ExerciseType::Translation:
                translationExerciseWidget_->setQuestion(q);
                exerciseStackedWidget_->setCurrentWidget(
                    translationExerciseWidget_);
                break;
            case ExerciseType::Grammar:
                grammarExerciseWidget_->setQuestion(q, this);
                exerciseStackedWidget_->setCurrentWidget(grammarExerciseWidget_);
                break;
            case ExerciseType::Audio:
                if (audioExerciseWidget_) audioExerciseWidget_->setQuestion(q);
                exerciseStackedWidget_->setCurrentWidget(audioExerciseWidget_);
                break;
            default:
                if (currentLesson_.IsValid())
                    displayLevelInfo(currentLesson_);
                else if (levelInfoWidget_)
                    levelInfoWidget_->setLevelInfo("Ошибка",
                                                 "Тип упражнения не определен.");
                break;
        }
    } else {
        if (!currentQuestionSet_.isEmpty() &&
            currentQuestionIndex_ >= currentQuestionSet_.size()) {
            exerciseFinished(correctAnswersInSet_ == currentQuestionSet_.size() &&
                             lives_ >= 0);
        } else if (currentExerciseType_ != ExerciseType::None) {
            resetExerciseState();
            if (currentLesson_.IsValid()) displayLevelInfo(currentLesson_);
        } else {
            if (currentLesson_.IsValid()) displayLevelInfo(currentLesson_);
        }
    }
}

void MainWindow::submitAnswerClicked() {
    if (currentQuestionIndex_ < 0 ||
        currentQuestionIndex_ >= currentQuestionSet_.size() ||
        currentExerciseType_ == ExerciseType::None) {
        return;
    }
    if (currentExerciseType_ == ExerciseType::Grammar) {
        return;
    }

    QString userAnswer;
    QWidget* currentExercisePage =
        exerciseStackedWidget_ ? exerciseStackedWidget_->currentWidget()
                               : nullptr;
    if (!currentExercisePage) {
        return;
    }

    if (currentExercisePage == translationExerciseWidget_ &&
        translationExerciseWidget_) {
        userAnswer = translationExerciseWidget_->getAnswer();
    } else if (currentExercisePage == audioExerciseWidget_ &&
               audioExerciseWidget_) {
        userAnswer = audioExerciseWidget_->getAnswer();
    } else {
        return;
    }
    checkAnswer(userAnswer, currentExerciseType_);
}

void MainWindow::grammarOptionClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        checkAnswer(button->text(), ExerciseType::Grammar);
    }
}

void MainWindow::checkAnswer(const QString& userAnswer,
                             ExerciseType questionType) {
    Q_UNUSED(questionType);
    if (currentQuestionIndex_ < 0 ||
        currentQuestionIndex_ >= currentQuestionSet_.size()) {
        return;
    }

    const Question& q = currentQuestionSet_.at(currentQuestionIndex_);

    QString normalizedUserAnswer = userAnswer.trimmed().toLower();
    normalizedUserAnswer.remove(QRegularExpression(QStringLiteral("[.,?!\"':;()]")));

    QString normalizedCorrectAnswer = q.correctAnswer.trimmed().toLower();
    normalizedCorrectAnswer.remove(QRegularExpression(QStringLiteral("[.,?!\"':;()]")));

    bool correct = (normalizedUserAnswer == normalizedCorrectAnswer);

    QWidget* currentPage =
        exerciseStackedWidget_ ? exerciseStackedWidget_->currentWidget()
                               : nullptr;
    if (!currentPage) {
        return;
    }

    if (currentPage == translationExerciseWidget_ && translationExerciseWidget_) {
        if (translationExerciseWidget_->submitButton)
            translationExerciseWidget_->submitButton->setEnabled(false);
        if (translationExerciseWidget_->answerEdit)
            translationExerciseWidget_->answerEdit->setReadOnly(true);
    } else if (currentPage == grammarExerciseWidget_ && grammarExerciseWidget_) {
    } else if (currentPage == audioExerciseWidget_ && audioExerciseWidget_) {
        if (audioExerciseWidget_->submitButton_)
            audioExerciseWidget_->submitButton_->setEnabled(false);
        if (audioExerciseWidget_->isAnswerEditValid()) {
            audioExerciseWidget_->setAnswerEditReadOnly(true);
        }
    }

    if (correct) {
        correctAnswersInSet_++;
        if (currentPage == translationExerciseWidget_ &&
            translationExerciseWidget_)
            translationExerciseWidget_->showCorrectAnswer(q.correctAnswer, true);
        else if (currentPage == grammarExerciseWidget_ && grammarExerciseWidget_)
            grammarExerciseWidget_->showCorrectAnswer(q.correctAnswer, true,
                                                    userAnswer);
        else if (currentPage == audioExerciseWidget_ && audioExerciseWidget_)
            audioExerciseWidget_->showCorrectAnswer(q.correctAnswer, true);

        QTimer::singleShot(1200, this, [this, currentPage]() {
            this->currentQuestionIndex_++;
            this->updateProgressBar();
            this->updateExercise();
            if (currentPage == translationExerciseWidget_ &&
                translationExerciseWidget_) {
                if (translationExerciseWidget_->submitButton)
                    translationExerciseWidget_->submitButton->setEnabled(true);
                if (translationExerciseWidget_->answerEdit)
                    translationExerciseWidget_->answerEdit->setReadOnly(false);
            } else if (currentPage == audioExerciseWidget_ &&
                       audioExerciseWidget_) {
                if (audioExerciseWidget_->submitButton_)
                    audioExerciseWidget_->submitButton_->setEnabled(true);
                if (audioExerciseWidget_->isAnswerEditValid()) {
                    audioExerciseWidget_->setAnswerEditReadOnly(false);
                }
            }
        });
    } else {
        lives_--;
        if (currentPage == translationExerciseWidget_ &&
            translationExerciseWidget_)
            translationExerciseWidget_->showCorrectAnswer(q.correctAnswer, false);
        else if (currentPage == grammarExerciseWidget_ && grammarExerciseWidget_)
            grammarExerciseWidget_->showCorrectAnswer(q.correctAnswer, false,
                                                    userAnswer);
        else if (currentPage == audioExerciseWidget_ && audioExerciseWidget_)
            audioExerciseWidget_->showCorrectAnswer(q.correctAnswer, false);

        if (lives_ < 0) {
            QTimer::singleShot(2500, this, [this]() { this->exerciseFinished(false); });
        } else {
            QTimer::singleShot(2500, this, [this, currentPage]() {
                this->currentQuestionIndex_++;
                this->updateProgressBar();
                this->updateExercise();
                if (currentPage == translationExerciseWidget_ &&
                    translationExerciseWidget_) {
                    if (translationExerciseWidget_->submitButton)
                        translationExerciseWidget_->submitButton->setEnabled(true);
                    if (translationExerciseWidget_->answerEdit)
                        translationExerciseWidget_->answerEdit->setReadOnly(false);
                } else if (currentPage == audioExerciseWidget_ &&
                           audioExerciseWidget_) {
                    if (audioExerciseWidget_->submitButton_)
                        audioExerciseWidget_->submitButton_->setEnabled(true);
                    if (audioExerciseWidget_->isAnswerEditValid()) {
                        audioExerciseWidget_->setAnswerEditReadOnly(false);
                    }
                }
            });
        }
    }
    updateProgressBar();
}

void MainWindow::exerciseFinished(bool allCorrectInSetArgument) {
    if (exerciseTimer_ && exerciseTimer_->isActive()) exerciseTimer_->stop();
    if (statusBar()) statusBar()->clearMessage();

    qint64 elapsedSetMilliseconds =
        exerciseSetTimer_.isValid() ? exerciseSetTimer_.elapsed() : 0;
    int elapsedSetSeconds = static_cast<int>(elapsedSetMilliseconds / 1000);

    QString message;
    int starsEarnedThisType = 0;

    bool actuallyAllCorrect = (!currentQuestionSet_.isEmpty() &&
                               correctAnswersInSet_ == currentQuestionSet_.size());

    if (lives_ < 0) {
        message = "Увы, вы допустили слишком много ошибок. Попробуйте снова!";
        starsEarnedThisType = 0;
    } else if (remainingTimeSeconds_ <= 0 && exerciseSetTimer_.isValid() &&
               elapsedSetSeconds >= timePerExerciseSetSeconds_) {
        if (actuallyAllCorrect) {
            message = "Время вышло, но все ответы верны! Хорошая работа!";
            starsEarnedThisType = 1;
            DataManager::Instance().IncrementUserCompletedExercises(
                DataManager::Instance().GetCurrentUserId());
        } else {
            message = "Время вышло! Не все ответы верны.";
            starsEarnedThisType = 0;
        }
    } else if (actuallyAllCorrect) {
        message = "Поздравляем! Набор упражнений выполнен идеально!";
        starsEarnedThisType = 1;
        DataManager::Instance().IncrementUserCompletedExercises(
            DataManager::Instance().GetCurrentUserId());
    } else {
        message = "Набор упражнений завершен, но не все ответы были верны.";
        starsEarnedThisType = 0;
    }

    QMessageBox::information(this, "Упражнение завершено", message);

    User currentUser = DataManager::Instance().GetCurrentUser();
    if (currentUser.IsValid() && currentSelectedLevelId_ > 0 &&
        currentExerciseType_ != ExerciseType::None) {
        UserLessonProgress progress =
            DataManager::Instance().GetUserProgressForLesson(
                currentUser.id_, currentSelectedLevelId_);
        bool progressChanged = false;

        if (currentExerciseType_ == ExerciseType::Translation &&
            progress.starsTranslation == 0 && starsEarnedThisType == 1) {
            progress.starsTranslation = 1;
            progressChanged = true;
        } else if (currentExerciseType_ == ExerciseType::Grammar &&
                   progress.starsGrammar == 0 && starsEarnedThisType == 1) {
            progress.starsGrammar = 1;
            progressChanged = true;
        } else if (currentExerciseType_ == ExerciseType::Audio &&
                   progress.starsAudio == 0 && starsEarnedThisType == 1) {
            progress.starsAudio = 1;
            progressChanged = true;
        }

        if (progressChanged) {
            DataManager::Instance().UpdateUserProgressForLesson(progress);
            updateOverallLevelStarsDisplay(currentSelectedLevelId_);
            populateLevelMenu();
        }
    }

    if (currentUser.IsValid() && elapsedSetSeconds > 0) {
        DataManager::Instance().UpdateUserStudyTime(currentUser.id_,
                                                    elapsedSetSeconds);
        if (sessionActivityTimer_.isValid())
            sessionActivityTimer_.restart();
        else
            sessionActivityTimer_.start();
    }

    resetExerciseState();
    if (currentLesson_.IsValid()) {
        displayLevelInfo(currentLesson_);
    }
}

void MainWindow::updateProgressBar() {
    if (exerciseProgressBar_) {
        int questionsTotal = currentQuestionSet_.isEmpty()
                                 ? QUESTIONS_PER_SET
                                 : currentQuestionSet_.size();
        int currentQuestionDisplay =
            (currentQuestionIndex_ >= 0 &&
             currentQuestionIndex_ < questionsTotal)
                ? currentQuestionIndex_ + 1
                : (questionsTotal > 0 ? 1 : 0);
        if (currentQuestionIndex_ >= questionsTotal && questionsTotal > 0)
            currentQuestionDisplay = questionsTotal;

        exerciseProgressBar_->setValue(currentQuestionIndex_ >= 0
                                           ? currentQuestionIndex_
                                           : 0);
        exerciseProgressBar_->setRange(0, questionsTotal > 0 ? questionsTotal
                                                             : 1);
        exerciseProgressBar_->setFormat(
            QString("Вопрос: %1 / %2 (Ошибок: %3 из %4)")
                .arg(currentQuestionDisplay)
                .arg(questionsTotal)
                .arg(MAX_LIVES - (lives_ < 0 ? 0 : lives_))
                .arg(MAX_LIVES));
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_H &&
        (event->modifiers() & Qt::ControlModifier)) {
        showHint();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QWidget* currentExercisePage =
            exerciseStackedWidget_ ? exerciseStackedWidget_->currentWidget()
                                   : nullptr;
        if (currentExercisePage) {
            if (currentExercisePage == translationExerciseWidget_ &&
                translationExerciseWidget_ &&
                translationExerciseWidget_->submitButton &&
                translationExerciseWidget_->submitButton->isEnabled()) {
                submitAnswerClicked();
            } else if (currentExercisePage == audioExerciseWidget_ &&
                       audioExerciseWidget_ &&
                       audioExerciseWidget_->submitButton_ &&
                       audioExerciseWidget_->submitButton_->isEnabled()) {
                submitAnswerClicked();
            } else {
                QMainWindow::keyPressEvent(event);
            }
        } else {
            QMainWindow::keyPressEvent(event);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::showHint() {
    if (currentExerciseType_ != ExerciseType::None &&
        currentQuestionIndex_ >= 0 &&
        currentQuestionIndex_ < currentQuestionSet_.size()) {
        const Question& q = currentQuestionSet_.at(currentQuestionIndex_);
        if (!q.hint.isEmpty()) {
            QMessageBox::information(this, "Подсказка", q.hint);
        } else {
            QMessageBox::information(this, "Подсказка",
                                     "Для этого задания подсказки нет.");
        }
    } else {
        QMessageBox::information(
            this, "Подсказка",
            "Начните упражнение, чтобы увидеть подсказку (Ctrl+H).");
    }
}

void MainWindow::updateTimerDisplay() {
    if (remainingTimeSeconds_ > 0) {
        remainingTimeSeconds_--;
        int minutes = remainingTimeSeconds_ / 60;
        int seconds = remainingTimeSeconds_ % 60;
        if (statusBar()) {
            statusBar()->showMessage(QString("Осталось времени: %1:%2")
                                         .arg(minutes, 2, 10, QChar('0'))
                                         .arg(seconds, 2, 10, QChar('0')));
        }
    } else {
        if (exerciseTimer_ && exerciseTimer_->isActive()) {
            exerciseTimer_->stop();
            if (currentExerciseType_ != ExerciseType::None) {
                timeRanOut();
            }
        }
    }
}

void MainWindow::timeRanOut() {
    if (statusBar()) statusBar()->clearMessage();
    exerciseFinished(correctAnswersInSet_ == currentQuestionSet_.size() &&
                     lives_ >= 0 && !currentQuestionSet_.isEmpty());
}
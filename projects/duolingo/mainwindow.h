#pragma once

#include "leveldata.h"
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>

class LevelInfoWidget;
class TranslationExerciseWidget;
class GrammarExerciseWidget;
class AudioExerciseWidget;
class UserStatsDialog;
class QKeyEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

public slots:
    void grammarOptionClicked();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void selectLevelTriggered();
    void userProfileClicked();
    void startTranslationExercise();
    void startGrammarExercise();
    void startAudioExercise();
    void submitAnswerClicked();
    void updateExercise();
    void exerciseFinished(bool allCorrectInSet);
    void updateTimerDisplay();
    void timeRanOut();

private:
    void setupUi();
    void setupMenuBar();
    void populateLevelMenu();
    void loadLevel(long long lessonId);
    void displayLevelInfo(const Lesson& lesson);
    void startExerciseSet(ExerciseType type);
    void showHint();
    void updateStarsDisplay(ExerciseType type, int stars);
    void updateOverallLevelStarsDisplay(long long lessonId);
    void updateProgressBar();
    void resetExerciseState();
    void checkAnswer(const QString& userAnswer, ExerciseType questionType);

    QSplitter* mainSplitter_ = nullptr;
    QWidget* leftPanelWidget_ = nullptr;
    QLabel* currentLevelNameLabel_ = nullptr;
    QHBoxLayout* starsLayout_ = nullptr;
    QLabel* starLabels_[3]{};
    QPushButton* translationButton_ = nullptr;
    QPushButton* grammarButton_ = nullptr;
    QPushButton* audioButton_ = nullptr;
    QWidget* rightPanelWidget_ = nullptr;
    QProgressBar* exerciseProgressBar_ = nullptr;
    QStackedWidget* exerciseStackedWidget_ = nullptr;
    LevelInfoWidget* levelInfoWidget_ = nullptr;
    TranslationExerciseWidget* translationExerciseWidget_ = nullptr;
    GrammarExerciseWidget* grammarExerciseWidget_ = nullptr;
    AudioExerciseWidget* audioExerciseWidget_ = nullptr;
    QMenu* levelsMenu_ = nullptr;
    QToolButton* userProfileButton_ = nullptr;

    long long currentSelectedLevelId_ = -1;
    Lesson currentLesson_;
    ExerciseType currentExerciseType_ = ExerciseType::None;
    QList<Question> currentQuestionSet_;
    int currentQuestionIndex_ = -1;
    int lives_ = 3;
    const int MAX_LIVES = 3;
    int questionsInCurrentSet_ = 5;
    const int QUESTIONS_PER_SET = 5;
    int correctAnswersInSet_ = 0;
    QTimer* exerciseTimer_ = nullptr;
    int timePerExerciseSetSeconds_ = 50;
    int remainingTimeSeconds_ = 0;
    QElapsedTimer sessionActivityTimer_;
    QElapsedTimer exerciseSetTimer_;
};
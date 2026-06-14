#include "audioexercisewidget.h"

#include <QCoreApplication>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QPushButton>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

AudioExerciseWidget::AudioExerciseWidget(QWidget* parent)
    : QWidget(parent),
      playButton_(new QPushButton("Прослушать", this)),
      answerEdit_(new QLineEdit(this)),
      submitButton_(new QPushButton("Проверить", this)),
      audioPlayer_(new QMediaPlayer(this)),
      correctAnswerFeedbackLabel_(new QLabel(this)) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    answerEdit_->setPlaceholderText("Введите услышанное");

    correctAnswerFeedbackLabel_->setWordWrap(true);
    correctAnswerFeedbackLabel_->setVisible(false);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audioOutput_ = new QAudioOutput(this);
    audioPlayer_->setAudioOutput(audioOutput_);
#endif

    connect(playButton_, &QPushButton::clicked, this,
            &AudioExerciseWidget::playAudio);

    mainLayout->addWidget(playButton_, 0, Qt::AlignCenter);
    mainLayout->addWidget(answerEdit_);
    mainLayout->addWidget(correctAnswerFeedbackLabel_);
    mainLayout->addStretch();
    mainLayout->addWidget(submitButton_, 0, Qt::AlignRight);
    setLayout(mainLayout);
}

AudioExerciseWidget::~AudioExerciseWidget() {
    if (audioPlayer_) {
        audioPlayer_->stop();
    }
}

void AudioExerciseWidget::setQuestion(const Question& q) {
    currentAudioPath_ = q.audioPath;

    answerEdit_->clear();
    answerEdit_->setReadOnly(false);
    answerEdit_->setFocus();

    bool canPlay = !currentAudioPath_.isEmpty();
    playButton_->setEnabled(canPlay);
    submitButton_->setEnabled(true);

    correctAnswerFeedbackLabel_->setVisible(false);
    correctAnswerFeedbackLabel_->setText("");

    if (canPlay) {
        if (audioPlayer_->playbackState() != QMediaPlayer::StoppedState) {
            audioPlayer_->stop();
        }
        audioPlayer_->setSource(QUrl());
        QTimer::singleShot(50, this, &AudioExerciseWidget::playAudio);
    }
}

QString AudioExerciseWidget::getAnswer() const {
    return answerEdit_->text().trimmed();
}

void AudioExerciseWidget::showCorrectAnswer(const QString& correctAnswer,
                                          bool wasUserCorrect) {
    if (wasUserCorrect) {
        correctAnswerFeedbackLabel_->setStyleSheet(
            "color: green; font-weight: bold;");
        correctAnswerFeedbackLabel_->setText("Верно!");
    } else {
        correctAnswerFeedbackLabel_->setStyleSheet("color: red;");
        correctAnswerFeedbackLabel_->setText(
            QString("Неверно. Правильный ответ: <b>%1</b>")
                .arg(correctAnswer));
    }
    correctAnswerFeedbackLabel_->setVisible(true);
}

void AudioExerciseWidget::setAnswerEditReadOnly(bool readOnly) {
    answerEdit_->setReadOnly(readOnly);
}

bool AudioExerciseWidget::isAnswerEditValid() const {
    return answerEdit_ != nullptr;
}

void AudioExerciseWidget::playAudio() {
    if (currentAudioPath_.isEmpty() || !audioPlayer_) {
        playButton_->setEnabled(false);
        return;
    }

    if (audioPlayer_->playbackState() != QMediaPlayer::StoppedState) {
        audioPlayer_->stop();
    }
    audioPlayer_->setSource(QUrl());

    QUrl targetUrl;
    if (currentAudioPath_.startsWith("qrc:/") ||
        currentAudioPath_.startsWith(":/")) {
        targetUrl = QUrl(currentAudioPath_);
    } else {
        QString audioDirPath = QCoreApplication::applicationDirPath() +
                               QDir::separator() + "audio";
        targetUrl =
            QUrl::fromLocalFile(audioDirPath + QDir::separator() + currentAudioPath_);
    }

    if (!targetUrl.isValid() || targetUrl.isEmpty()) {
         playButton_->setEnabled(false);
         return;
    }

    playButton_->setEnabled(true);

    audioPlayer_->setSource(targetUrl);

    if (audioPlayer_->error() != QMediaPlayer::NoError || (audioPlayer_->source().isEmpty() && !targetUrl.isEmpty())) {
        playButton_->setEnabled(false);
        return;
    }

    audioPlayer_->setPosition(0);
    audioPlayer_->play();
}
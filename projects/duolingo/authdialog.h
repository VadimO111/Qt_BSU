#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>

class AuthDialog : public QDialog {
    Q_OBJECT

public:
    explicit AuthDialog(QWidget* parent = nullptr);
    long long CurrentUserId() const;

private slots:
    void OnLoginButtonClicked();
    void OnRegisterButtonClicked();
    void ClearErrorLabelsOnTextChange();

private:
    void SetupUI();
    void ClearErrorLabels();

    QLineEdit* loginUsernameEdit_ = nullptr;
    QLineEdit* loginPasswordEdit_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QLabel* loginErrorLabel_ = nullptr;

    QLineEdit* registerUsernameEdit_ = nullptr;
    QLineEdit* registerEmailEdit_ = nullptr;
    QLineEdit* registerPasswordEdit_ = nullptr;
    QLineEdit* registerPasswordConfirmEdit_ = nullptr;
    QPushButton* registerButton_ = nullptr;
    QLabel* registerErrorLabel_ = nullptr;

    QTabWidget* tabWidget_ = nullptr;
    QPushButton* cancelButton_ = nullptr;

    long long currentUserId_ = -1;
};
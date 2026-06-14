#include "authdialog.h"
#include "datamanager.h"
#include "user.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>

AuthDialog::AuthDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Вход или Регистрация"));
    setModal(true);
    SetupUI();

    if (loginUsernameEdit_) {
        loginUsernameEdit_->setFocus();
    }
    if (loginButton_) {
        connect(loginButton_, &QPushButton::clicked, this,
                &AuthDialog::OnLoginButtonClicked);
    }
    if (registerButton_) {
        connect(registerButton_, &QPushButton::clicked, this,
                &AuthDialog::OnRegisterButtonClicked);
    }
    if (cancelButton_) {
        connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    }

    if (loginUsernameEdit_)
        connect(loginUsernameEdit_, &QLineEdit::textChanged, this,
                &AuthDialog::ClearErrorLabelsOnTextChange);
    if (loginPasswordEdit_)
        connect(loginPasswordEdit_, &QLineEdit::textChanged, this,
                &AuthDialog::ClearErrorLabelsOnTextChange);
    if (registerUsernameEdit_)
        connect(registerUsernameEdit_, &QLineEdit::textChanged, this,
                &AuthDialog::ClearErrorLabelsOnTextChange);
    if (registerEmailEdit_)
        connect(registerEmailEdit_, &QLineEdit::textChanged, this,
                &AuthDialog::ClearErrorLabelsOnTextChange);
    if (registerPasswordEdit_)
        connect(registerPasswordEdit_, &QLineEdit::textChanged, this,
                &AuthDialog::ClearErrorLabelsOnTextChange);
    if (registerPasswordConfirmEdit_)
        connect(registerPasswordConfirmEdit_, &QLineEdit::textChanged, this,
                &AuthDialog::ClearErrorLabelsOnTextChange);

    setMinimumWidth(380);
    adjustSize();
}

void AuthDialog::SetupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    tabWidget_ = new QTabWidget(this);

    QWidget* loginTab = new QWidget(tabWidget_);
    QFormLayout* loginLayout = new QFormLayout(loginTab);
    loginUsernameEdit_ = new QLineEdit(loginTab);
    loginPasswordEdit_ = new QLineEdit(loginTab);
    loginPasswordEdit_->setEchoMode(QLineEdit::Password);
    loginButton_ = new QPushButton("Войти", loginTab);
    loginErrorLabel_ = new QLabel(loginTab);
    loginErrorLabel_->setStyleSheet("QLabel { color : red; }");
    loginErrorLabel_->setVisible(false);
    loginErrorLabel_->setWordWrap(true);

    loginLayout->addRow("Имя пользователя:", loginUsernameEdit_);
    loginLayout->addRow("Пароль:", loginPasswordEdit_);
    loginLayout->addWidget(loginButton_);
    loginLayout->addWidget(loginErrorLabel_);
    loginTab->setLayout(loginLayout);
    tabWidget_->addTab(loginTab, "Вход");

    QWidget* registerTab = new QWidget(tabWidget_);
    QFormLayout* registerLayout = new QFormLayout(registerTab);
    registerUsernameEdit_ = new QLineEdit(registerTab);
    registerEmailEdit_ = new QLineEdit(registerTab);
    registerEmailEdit_->setPlaceholderText("Необязательно");
    registerPasswordEdit_ = new QLineEdit(registerTab);
    registerPasswordEdit_->setEchoMode(QLineEdit::Password);
    registerPasswordConfirmEdit_ = new QLineEdit(registerTab);
    registerPasswordConfirmEdit_->setEchoMode(QLineEdit::Password);
    registerButton_ = new QPushButton("Зарегистрироваться", registerTab);
    registerErrorLabel_ = new QLabel(registerTab);
    registerErrorLabel_->setStyleSheet("QLabel { color : red; }");
    registerErrorLabel_->setVisible(false);
    registerErrorLabel_->setWordWrap(true);

    registerLayout->addRow("Имя пользователя:", registerUsernameEdit_);
    registerLayout->addRow("Email:", registerEmailEdit_);
    registerLayout->addRow("Пароль:", registerPasswordEdit_);
    registerLayout->addRow("Повторите пароль:", registerPasswordConfirmEdit_);
    registerLayout->addWidget(registerButton_);
    registerLayout->addWidget(registerErrorLabel_);
    registerTab->setLayout(registerLayout);
    tabWidget_->addTab(registerTab, "Регистрация");

    mainLayout->addWidget(tabWidget_);

    cancelButton_ = new QPushButton("Отмена", this);
    QHBoxLayout* bottomButtonLayout = new QHBoxLayout();
    bottomButtonLayout->addStretch();
    bottomButtonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(bottomButtonLayout);

    setLayout(mainLayout);
}

long long AuthDialog::CurrentUserId() const { return currentUserId_; }

void AuthDialog::ClearErrorLabels() {
    if (loginErrorLabel_) {
        loginErrorLabel_->setVisible(false);
        loginErrorLabel_->setText("");
    }
    if (registerErrorLabel_) {
        registerErrorLabel_->setVisible(false);
        registerErrorLabel_->setText("");
    }
}

void AuthDialog::ClearErrorLabelsOnTextChange() { ClearErrorLabels(); }

void AuthDialog::OnLoginButtonClicked() {
    ClearErrorLabels();

    QString username = loginUsernameEdit_->text().trimmed();
    QString password = loginPasswordEdit_->text();

    if (username.isEmpty() || password.isEmpty()) {
        loginErrorLabel_->setText(
            "Имя пользователя и пароль не могут быть пустыми.");
        loginErrorLabel_->setVisible(true);
        return;
    }

    DataManager& dbManager = DataManager::Instance();
    User user = dbManager.GetUserByUsername(username);

    if (!user.IsValid()) {
        loginErrorLabel_->setText("Пользователь с таким именем не найден.");
        loginErrorLabel_->setVisible(true);
        return;
    }

    if (!dbManager.CheckPassword(user.id_, password)) {
        loginErrorLabel_->setText("Неверный пароль.");
        loginErrorLabel_->setVisible(true);
        return;
    }

    currentUserId_ = user.id_;
    accept();
}

void AuthDialog::OnRegisterButtonClicked() {
    ClearErrorLabels();

    QString username = registerUsernameEdit_->text().trimmed();
    QString email = registerEmailEdit_->text().trimmed();
    QString password = registerPasswordEdit_->text();
    QString passwordConfirm = registerPasswordConfirmEdit_->text();

    if (username.isEmpty()) {
        registerErrorLabel_->setText("Имя пользователя не может быть пустым.");
        registerErrorLabel_->setVisible(true);
        return;
    }
    if (username.length() > 50) {
        registerErrorLabel_->setText(
            "Имя пользователя слишком длинное (макс. 50 символов).");
        registerErrorLabel_->setVisible(true);
        return;
    }
    if (password.isEmpty()) {
        registerErrorLabel_->setText("Пароль не может быть пустым.");
        registerErrorLabel_->setVisible(true);
        return;
    }
    if (password.length() < 6) {
        registerErrorLabel_->setText(
            "Пароль слишком короткий (мин. 6 символов).");
        registerErrorLabel_->setVisible(true);
        return;
    }
    if (password != passwordConfirm) {
        registerErrorLabel_->setText("Пароли не совпадают.");
        registerErrorLabel_->setVisible(true);
        return;
    }
    if (!email.isEmpty() && (!email.contains('@') || !email.contains('.'))) {
        registerErrorLabel_->setText("Некорректный формат Email.");
        registerErrorLabel_->setVisible(true);
        return;
    }
    if (!email.isEmpty() && email.length() > 100) {
        registerErrorLabel_->setText(
            "Email слишком длинный (макс. 100 символов).");
        registerErrorLabel_->setVisible(true);
        return;
    }

    DataManager& dbManager = DataManager::Instance();

    if (dbManager.UserExists(username)) {
        registerErrorLabel_->setText(
            "Пользователь с таким именем уже существует.");
        registerErrorLabel_->setVisible(true);
        return;
    }

    User newUserData;
    newUserData.name_ = username;
    newUserData.email_ = email;

    long long newUserId = dbManager.AddUser(newUserData, password);

    if (newUserId != -1) {
        QMessageBox::information(
            this, "Регистрация успешна",
            "Вы успешно зарегистрированы!\nТеперь вы можете войти.");
        tabWidget_->setCurrentIndex(0);
        loginUsernameEdit_->setText(username);
        loginPasswordEdit_->clear();
        loginPasswordEdit_->setFocus();
    } else {
        QString dbError = dbManager.GetLastErrorString();
        if (dbError.contains("UNIQUE constraint failed: users.name") ||
            dbError.contains("already exists")) {
            registerErrorLabel_->setText(
                "Пользователь с таким именем уже существует.");
        } else if (dbError.contains("UNIQUE constraint failed: users.email") &&
                   !email.isEmpty()) {
            registerErrorLabel_->setText(
                "Пользователь с таким Email уже существует.");
        } else {
            registerErrorLabel_->setText("Ошибка регистрации: " + dbError);
        }
        registerErrorLabel_->setVisible(true);
    }
}
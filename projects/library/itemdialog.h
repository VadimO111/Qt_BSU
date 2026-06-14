#pragma once

#include <QDialog>
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QFormLayout>
#include <variant>
#include <QString>
#include <QDate>

enum class Mode {Library, Users};

struct BookData {
    QString title;
    QString author;
    int year;
};

struct UserData {
    QString name;
    QString surname;
    QDate birthday;
};

class ItemDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItemDialog(QWidget* parent = nullptr, Mode mode = Mode::Library);

    std::variant<BookData, UserData> GetInputData() const;

private slots:
    void validateInput();

private:
    QLineEdit* title_edit_ = nullptr;
    QLineEdit* author_edit_ = nullptr;
    QLineEdit* name_edit_ = nullptr;
    QLineEdit* surname_edit_ = nullptr;
    QSpinBox* year_edit_ = nullptr;
    QDateEdit* date_edit_ = nullptr;
    Mode mode_ = Mode::Library;
    QFormLayout* layout_ = nullptr;
    QPushButton* cancel_button_ = nullptr;
    QPushButton* add_button_ = nullptr;
};

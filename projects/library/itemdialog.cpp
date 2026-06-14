#include "itemdialog.h"
#include <QMessageBox>

ItemDialog::ItemDialog(QWidget* parent, Mode mode)
    : QDialog(parent), mode_(mode) {
    setWindowTitle(mode_ == Mode::Library ? "Add Book" : "Add User");
    setFixedSize(400, 250);
    setModal(true);

    layout_ = new QFormLayout(this);

    if (mode_ == Mode::Library) {
        title_edit_ = new QLineEdit(this);
        author_edit_ = new QLineEdit(this);
        year_edit_ = new QSpinBox(this);
        year_edit_->setRange(-4000, QDate::currentDate().year() + 1);
        year_edit_->setValue(QDate::currentDate().year());

        layout_->addRow(tr("Title:"), title_edit_);
        layout_->addRow(tr("Author:"), author_edit_);
        layout_->addRow(tr("Year:"), year_edit_);

        connect(title_edit_, &QLineEdit::textChanged, this, &ItemDialog::validateInput);
        connect(author_edit_, &QLineEdit::textChanged, this, &ItemDialog::validateInput);
    } else {
        name_edit_ = new QLineEdit(this);
        surname_edit_ = new QLineEdit(this);
        date_edit_ = new QDateEdit(this);
        date_edit_->setCalendarPopup(true);
        date_edit_->setDate(QDate::currentDate());
        date_edit_->setMinimumDate(QDate().currentDate().addYears(-150));
        date_edit_->setMaximumDate(QDate::currentDate());

        layout_->addRow(tr("Name:"), name_edit_);
        layout_->addRow(tr("Surname:"), surname_edit_);
        layout_->addRow(tr("Birthday:"), date_edit_);

        connect(name_edit_, &QLineEdit::textChanged, this, &ItemDialog::validateInput);
        connect(surname_edit_, &QLineEdit::textChanged, this, &ItemDialog::validateInput);
    }

    cancel_button_ = new QPushButton(tr("Cancel"), this);
    add_button_ = new QPushButton(tr("Add"), this);
    add_button_->setEnabled(false);

    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
    connect(add_button_, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancel_button_);
    buttonLayout->addWidget(add_button_);
    layout_->addRow(buttonLayout);

    validateInput();
}

void ItemDialog::validateInput() {
    bool valid = false;
    if (mode_ == Mode::Library) {
        valid = !title_edit_->text().trimmed().isEmpty() &&
                !author_edit_->text().trimmed().isEmpty();
    } else {
        valid = !name_edit_->text().trimmed().isEmpty() &&
                !surname_edit_->text().trimmed().isEmpty();
    }
    add_button_->setEnabled(valid);
}

std::variant<BookData, UserData> ItemDialog::GetInputData() const {
    if (mode_ == Mode::Library) {
        return BookData{
            title_edit_->text().trimmed(),
            author_edit_->text().trimmed(),
            year_edit_->value()
        };
    } else {
        return UserData{
            name_edit_->text().trimmed(),
            surname_edit_->text().trimmed(),
            date_edit_->date()
        };
    }
}

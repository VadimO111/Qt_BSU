#pragma once

#include "library.h"
#include <memory>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

class AssignBookDialog : public QDialog {
    Q_OBJECT

public:
    explicit AssignBookDialog(Library& library, QWidget* parent = nullptr);
    ~AssignBookDialog() = default;

    std::weak_ptr<Book> getSelectedBook() const;

private slots:
    void filterBooks(const QString& text);
    void onBookSelected();
    void acceptDialog();

private:
    void populateBookList();

    Library& library_;
    std::vector<std::shared_ptr<Book>> allBooks_;
    QLineEdit* searchEdit_ = nullptr;
    QListWidget* bookListWidget_ = nullptr;
    QPushButton* assignButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    QVBoxLayout* mainLayout_ = nullptr;
    std::weak_ptr<Book> selectedBookWkPtr_;
};

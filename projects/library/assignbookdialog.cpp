#include "assignbookdialog.h"
#include "book.h"

AssignBookDialog::AssignBookDialog(Library& library, QWidget* parent)
    : QDialog(parent), library_(library)
{
    setWindowTitle(tr("Assign Book"));
    setMinimumSize(400, 300);

    mainLayout_ = new QVBoxLayout(this);
    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText(tr("Search books by title or author..."));
    searchEdit_->setClearButtonEnabled(true);

    bookListWidget_ = new QListWidget(this);
    bookListWidget_->setAlternatingRowColors(true);

    assignButton_ = new QPushButton(tr("Assign"), this);
    cancelButton_ = new QPushButton(tr("Cancel"), this);
    assignButton_->setEnabled(false);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addWidget(assignButton_);

    mainLayout_->addWidget(searchEdit_);
    mainLayout_->addWidget(bookListWidget_, 1);
    mainLayout_->addLayout(buttonLayout);

    connect(searchEdit_, &QLineEdit::textChanged, this, &AssignBookDialog::filterBooks);
    connect(bookListWidget_, &QListWidget::currentItemChanged, this, &AssignBookDialog::onBookSelected);
    connect(bookListWidget_, &QListWidget::itemActivated, this, &AssignBookDialog::acceptDialog);
    connect(assignButton_, &QPushButton::clicked, this, &AssignBookDialog::acceptDialog);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);

    populateBookList();
}

void AssignBookDialog::populateBookList() {
    bookListWidget_->clear();
    allBooks_ = library_.GetBooks();

    for (const auto& bookPtr : allBooks_) {
        if (!bookPtr) continue;
        QString displayText = QString("%1  (%2, %3)")
                                  .arg(bookPtr->GetTitle())
                                  .arg(bookPtr->GetAuthor())
                                  .arg(QString::number(bookPtr->GetYear()));
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, QVariant::fromValue(static_cast<int>(bookListWidget_->count())));
        bookListWidget_->addItem(item);
    }
}

void AssignBookDialog::filterBooks(const QString& text) {
    QString lowerText = text.toLower();
    for (int i = 0; i < bookListWidget_->count(); ++i) {
        QListWidgetItem* item = bookListWidget_->item(i);
        bool match = item->text().toLower().contains(lowerText);
        item->setHidden(!match);
    }
    if (bookListWidget_->currentItem() && bookListWidget_->currentItem()->isHidden()) {
        bookListWidget_->setCurrentItem(nullptr);
        assignButton_->setEnabled(false);
        selectedBookWkPtr_.reset();
    } else if (bookListWidget_->currentItem()){
        onBookSelected();
    } else {
        assignButton_->setEnabled(false);
        selectedBookWkPtr_.reset();
    }
}

void AssignBookDialog::onBookSelected() {
    QListWidgetItem* currentItem = bookListWidget_->currentItem();
    assignButton_->setEnabled(currentItem != nullptr);
    if (currentItem) {
        bool ok;
        int index = currentItem->data(Qt::UserRole).toInt(&ok);
        if (ok && index >= 0 && static_cast<size_t>(index) < allBooks_.size()) {
            selectedBookWkPtr_ = allBooks_[index];
        } else {
            qWarning() << "AssignBookDialog: Could not get valid index from list item data.";
            selectedBookWkPtr_.reset();
            assignButton_->setEnabled(false);
        }
    } else {
        selectedBookWkPtr_.reset();
    }
}

void AssignBookDialog::acceptDialog() {
    if (bookListWidget_->currentItem() && !selectedBookWkPtr_.expired()) {
        QDialog::accept();
    } else {
        QMessageBox::warning(this, tr("Selection Error"), tr("Please select a valid book from the list."));
        selectedBookWkPtr_.reset();
        assignButton_->setEnabled(false);
    }
}

std::weak_ptr<Book> AssignBookDialog::getSelectedBook() const {
    return selectedBookWkPtr_;
}

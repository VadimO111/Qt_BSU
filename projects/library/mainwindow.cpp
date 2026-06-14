#include "mainwindow.h"
#include "assignbookdialog.h"
#include <QStatusBar>
#include <QFileDialog>
#include <QHeaderView>

const QString RadioButtonStyle =
    "QRadioButton {"
    "   color: #3E8277;"
    "   font-weight: bold;"
    "   font-size: 12px;"
    "   padding: 5px;"
    "}"
    "QRadioButton::indicator {"
    "   width: 13px;"
    "   height: 13px;"
    "}"
    "QRadioButton::indicator:unchecked {"
    "   border: 2px solid #5D9B90;"
    "   border-radius: 1px;"
    "   background-color: white;"
    "}"
    "QRadioButton::indicator:checked {"
    "   border: 2px solid #5D9B90;"
    "   border-radius: 1px;"
    "   background-color: #5D9B90;"
    "}"
    "QRadioButton:hover {"
    "   color: #7AB9AD;"
    "}"
    "QRadioButton:hover::indicator {"
    "   border-color: #7AB9AD;"
    "}";

const QString TableHeaderStyle =
    "QHeaderView::section {"
    "   background-color: #5D9B90;"
    "   color: white;"
    "   padding: 5px;"
    "   border: 1px solid #3E8277;"
    "   font-weight: bold;"
    "   font-size: 12px;"
    "}"
    "QHeaderView::section:hover {"
    "   background-color: #7AB9AD;"
    "}";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), library_()
{
    SetupUi();
    SetupConnections();

    RefreshTables();
    ClearDetailView();
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::SetupUi() {
    setWindowTitle(tr("Library Manager"));
    resize(950, 650);

    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);
    main_h_layout_ = new QHBoxLayout(central_widget_);

    splitter_ = new QSplitter(Qt::Horizontal, this);
    main_h_layout_->addWidget(splitter_);

    left_panel_widget_ = new QWidget(splitter_);
    left_v_layout_ = new QVBoxLayout(left_panel_widget_);
    left_v_layout_->setContentsMargins(5, 5, 5, 5);

    radio_button_layout_ = new QHBoxLayout();
    books_button_ = new QRadioButton(tr("Books"), this);
    users_button_ = new QRadioButton(tr("Users"), this);
    books_button_->setStyleSheet(RadioButtonStyle);
    users_button_->setStyleSheet(RadioButtonStyle);
    mode_buttons_ = new QButtonGroup(this);
    mode_buttons_->addButton(books_button_);
    mode_buttons_->addButton(users_button_);
    books_button_->setChecked(true);
    radio_button_layout_->addWidget(books_button_);
    radio_button_layout_->addWidget(users_button_);
    radio_button_layout_->addStretch(1);
    left_v_layout_->addLayout(radio_button_layout_);

    search_edit_ = new QLineEdit(this);
    search_edit_->setPlaceholderText(tr("Search..."));
    search_edit_->setClearButtonEnabled(true);
    left_v_layout_->addWidget(search_edit_);

    control_button_layout_ = new QHBoxLayout();
    add_button_ = new QPushButton(tr("Add New"), this);
    add_button_->setStyleSheet("QPushButton { padding: 5px 10px; }");
    delete_button_ = new QPushButton(tr("Delete Selected"), this);
    delete_button_->setEnabled(false);
    delete_button_->setStyleSheet("QPushButton { color: #a00; padding: 5px 10px; }");
    control_button_layout_->addStretch();
    control_button_layout_->addWidget(add_button_);
    control_button_layout_->addWidget(delete_button_);
    left_v_layout_->addLayout(control_button_layout_);

    table_stack_ = new QStackedWidget(this);

    books_table_ = new QTableWidget(0, 3, this);
    QStringList bookHeaders;
    bookHeaders << tr("Title") << tr("Author") << tr("Year");
    books_table_->setHorizontalHeaderLabels(bookHeaders);
    books_table_->horizontalHeader()->setStyleSheet(TableHeaderStyle);
    books_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    books_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    books_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    books_table_->verticalHeader()->setVisible(false);
    books_table_->setAlternatingRowColors(true);
    books_table_->setSortingEnabled(true);
    books_table_->horizontalHeader()->setStretchLastSection(true);
    table_stack_->addWidget(books_table_);

    users_table_ = new QTableWidget(0, 4, this);
    QStringList userHeaders;
    userHeaders << tr("Name") << tr("Surname") << tr("Birthday") << tr("ID");
    users_table_->setHorizontalHeaderLabels(userHeaders);
    users_table_->horizontalHeader()->setStyleSheet(TableHeaderStyle);
    users_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    users_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    users_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    users_table_->verticalHeader()->setVisible(false);
    users_table_->setAlternatingRowColors(true);
    users_table_->setSortingEnabled(true);
    users_table_->setColumnWidth(3, 50);
    users_table_->horizontalHeader()->setStretchLastSection(false);
    users_table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    table_stack_->addWidget(users_table_);

    left_v_layout_->addWidget(table_stack_);

    right_panel_widget_ = new QWidget(splitter_);
    right_v_layout_ = new QVBoxLayout(right_panel_widget_);
    right_v_layout_->setContentsMargins(10, 5, 10, 5);

    detail_stack_ = new QStackedWidget(this);
    right_v_layout_->addWidget(detail_stack_);

    empty_detail_widget_ = new QWidget(this);
    QVBoxLayout* empty_layout = new QVBoxLayout(empty_detail_widget_);
    empty_detail_label_ = new QLabel(tr("Select an item from the list\nto view details."), this);
    empty_detail_label_->setAlignment(Qt::AlignCenter);
    empty_detail_label_->setStyleSheet("QLabel { color: #888; font-style: italic; }");
    empty_layout->addStretch();
    empty_layout->addWidget(empty_detail_label_);
    empty_layout->addStretch();
    detail_stack_->addWidget(empty_detail_widget_);

    book_detail_widget_ = new QWidget(this);
    book_detail_layout_ = new QFormLayout(book_detail_widget_);
    book_detail_layout_->setRowWrapPolicy(QFormLayout::WrapLongRows);
    book_title_label_ = new QLabel("-", this);
    book_title_label_->setWordWrap(true);
    book_author_label_ = new QLabel("-", this);
    book_author_label_->setWordWrap(true);
    book_year_label_ = new QLabel("-", this);
    cover_drop_area_ = new CoverDropArea(this);
    set_text_file_button_ = new QPushButton(tr("Set Text File..."), this);
    start_reading_button_ = new QPushButton(tr("Start Reading"), this);
    set_text_file_button_->setEnabled(false);
    start_reading_button_->setEnabled(false);

    book_detail_layout_->addRow(tr("<b>Title:</b>"), book_title_label_);
    book_detail_layout_->addRow(tr("<b>Author:</b>"), book_author_label_);
    book_detail_layout_->addRow(tr("<b>Year:</b>"), book_year_label_);
    book_detail_layout_->addRow(cover_drop_area_);
    book_detail_layout_->addRow(set_text_file_button_);
    book_detail_layout_->addRow(start_reading_button_);
    detail_stack_->addWidget(book_detail_widget_);

    user_detail_widget_ = new QWidget(this);
    user_detail_main_layout_ = new QVBoxLayout(user_detail_widget_);

    user_info_layout_ = new QFormLayout();
    user_name_label_ = new QLabel("-", this);
    user_surname_label_ = new QLabel("-", this);
    user_birthday_label_ = new QLabel("-", this);
    user_id_label_ = new QLabel("-", this);
    user_info_layout_->addRow(tr("<b>Name:</b>"), user_name_label_);
    user_info_layout_->addRow(tr("<b>Surname:</b>"), user_surname_label_);
    user_info_layout_->addRow(tr("<b>Birthday:</b>"), user_birthday_label_);
    user_info_layout_->addRow(tr("<b>ID:</b>"), user_id_label_);
    user_detail_main_layout_->addLayout(user_info_layout_);

    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    user_detail_main_layout_->addWidget(separator);

    user_borrowed_label_ = new QLabel(tr("<b>Borrowed Books:</b>"), this);
    user_detail_main_layout_->addWidget(user_borrowed_label_);
    user_borrowed_list_ = new QListWidget(this);
    user_borrowed_list_->setAlternatingRowColors(true);
    user_borrowed_list_->setStyleSheet("QListWidget { border: 1px solid #ccc; border-radius: 3px; }");
    user_detail_main_layout_->addWidget(user_borrowed_list_, 1);

    assignBookDialogButton_ = new QPushButton(tr("Assign Book..."), this);
    assignBookDialogButton_->setEnabled(false);
    user_detail_main_layout_->addWidget(assignBookDialogButton_);

    detail_stack_->addWidget(user_detail_widget_);

    reading_widget_ = new QWidget(this);
    reading_layout_ = new QVBoxLayout(reading_widget_);
    book_text_edit_ = new QTextEdit(this);
    book_text_edit_->setReadOnly(true);
    book_text_edit_->setPlaceholderText(tr("Book content will appear here..."));
    back_to_details_button_ = new QPushButton(tr("Back to Details"), this);
    reading_layout_->addWidget(book_text_edit_);
    reading_layout_->addWidget(back_to_details_button_);
    detail_stack_->addWidget(reading_widget_);

    splitter_->addWidget(left_panel_widget_);
    splitter_->addWidget(right_panel_widget_);

    splitter_->setStretchFactor(0, 2);
    splitter_->setStretchFactor(1, 3);
}

void MainWindow::SetupConnections() {
    connect(mode_buttons_, &QButtonGroup::buttonClicked, this, &MainWindow::ButtonChanged);
    connect(add_button_, &QPushButton::clicked, this, &MainWindow::AddItem);
    connect(delete_button_, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);
    connect(books_table_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::UpdateDetailView);
    connect(users_table_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::UpdateDetailView);
    connect(cover_drop_area_, &CoverDropArea::coverDropped, this, &MainWindow::HandleCoverDrop);
    connect(search_edit_, &QLineEdit::textChanged, this, &MainWindow::PerformSearch);
    connect(set_text_file_button_, &QPushButton::clicked, this, &MainWindow::SelectTextFile);
    connect(start_reading_button_, &QPushButton::clicked, this, &MainWindow::ShowReadingView);
    connect(back_to_details_button_, &QPushButton::clicked, this, &MainWindow::GoBackToDetails);
    connect(assignBookDialogButton_, &QPushButton::clicked, this, &MainWindow::openAssignBookDialog);
}

void MainWindow::ButtonChanged(QAbstractButton* button) {
    if (button == books_button_) {
        mode_ = Mode::Library;
        table_stack_->setCurrentIndex(0);
    } else if (button == users_button_) {
        mode_ = Mode::Users;
        table_stack_->setCurrentIndex(1);
    }
    search_edit_->clear();
    RefreshTables();
    ClearDetailView();
}

void MainWindow::AddItem() {
    ItemDialog dialog(this, mode_);
    if (dialog.exec() == QDialog::Accepted) {
        auto result = dialog.GetInputData();

        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, BookData>) {
                library_.AddBook(arg.title, arg.author, arg.year);
                statusBar()->showMessage(tr("Book '%1' added.").arg(arg.title), 3000);
            }
            else if constexpr (std::is_same_v<T, UserData>) {
                library_.AddUser(arg.name, arg.surname, arg.birthday);
                statusBar()->showMessage(tr("User '%1 %2' added.").arg(arg.name).arg(arg.surname), 3000);
            }
        }, result);

        RefreshTables();
    } else {
        statusBar()->showMessage(tr("Add operation cancelled."), 3000);
    }
}

void MainWindow::deleteSelectedItem() {
    QString itemType;
    QString itemName;
    bool itemIsValid = false;

    if (mode_ == Mode::Library) {
        itemType = tr("book");
        if (auto ptr = selected_book_wptr_.lock()) {
            itemName = "'" + ptr->GetTitle() + "'";
            itemIsValid = true;
        }
    } else {
        itemType = tr("user");
        if (auto ptr = selected_user_wptr_.lock()) {
            itemName = "'" + ptr->GetName() + " " + ptr->GetSurname() + "'";
            itemIsValid = true;
        }
    }

    if (!itemIsValid) {
        QMessageBox::warning(this, tr("Deletion Error"), tr("No valid item selected to delete."));
        qWarning() << "deleteSelectedItem: No valid item selected or item expired.";
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm Deletion"),
                                  tr("Are you sure you want to permanently delete the selected %1 %2?").arg(itemType).arg(itemName),
                                  QMessageBox::Yes | QMessageBox::Cancel,
                                  QMessageBox::Cancel);

    if (reply != QMessageBox::Yes) {
        statusBar()->showMessage(tr("Deletion cancelled."), 3000);
        return;
    }

    bool success = false;
    if (mode_ == Mode::Library) {
        success = library_.RemoveBook(selected_book_wptr_);
    } else {
        if (auto userPtr = selected_user_wptr_.lock()) {
            success = library_.RemoveUserById(userPtr->GetId());
        } else {
            qCritical() << "deleteSelectedItem: User weak_ptr expired between check and delete call.";
            QMessageBox::critical(this, tr("Internal Error"), tr("Could not identify the user to delete."));
        }
    }

    if (success) {
        QString successMessage = tr("%1 %2 deleted successfully.")
                                     .arg(itemType.replace(0, 1, itemType[0].toUpper()))
                                     .arg(itemName);
        statusBar()->showMessage(successMessage, 3000);
        RefreshTables();
        ClearDetailView();
    } else {
        QMessageBox::critical(this, tr("Deletion Failed"), tr("Could not delete the selected %1. The item might have been removed already, or an internal error occurred (check logs).").arg(itemType));
        qCritical() << "Failed to delete" << itemType << itemName;
        RefreshTables();
        ClearDetailView();
    }
}

void MainWindow::RefreshTables() {
    books_table_->selectionModel()->blockSignals(true);
    users_table_->selectionModel()->blockSignals(true);

    QTableWidget* currentTable = (mode_ == Mode::Library) ? books_table_ : users_table_;
    QString currentQuery = search_edit_->text();
    currentTable->setSortingEnabled(false);
    currentTable->clearContents();

    if (mode_ == Mode::Library) {
        auto books = library_.SearchBooks(currentQuery);
        books_table_->setRowCount(books.size());
        int row = 0;
        for (const auto& bookPtr : books) {
            if (!bookPtr) continue;

            QTableWidgetItem* titleItem = new QTableWidgetItem(bookPtr->GetTitle());
            QTableWidgetItem* authorItem = new QTableWidgetItem(bookPtr->GetAuthor());
            QTableWidgetItem* yearItem = new QTableWidgetItem(QString::number(bookPtr->GetYear()));
            titleItem->setData(Qt::UserRole, row);

            books_table_->setItem(row, 0, titleItem);
            books_table_->setItem(row, 1, authorItem);
            books_table_->setItem(row, 2, yearItem);
            row++;
        }
    } else {
        auto users = library_.SearchUsers(currentQuery);
        users_table_->setRowCount(users.size());
        int row = 0;
        for (const auto& userPtr : users) {
            if (!userPtr) continue;

            QTableWidgetItem* nameItem = new QTableWidgetItem(userPtr->GetName());
            QTableWidgetItem* surnameItem = new QTableWidgetItem(userPtr->GetSurname());
            QTableWidgetItem* birthdayItem = new QTableWidgetItem(userPtr->GetBirthday().toString("dd.MM.yyyy"));
            QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(userPtr->GetId()));
            idItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            nameItem->setData(Qt::UserRole, userPtr->GetId());

            users_table_->setItem(row, 0, nameItem);
            users_table_->setItem(row, 1, surnameItem);
            users_table_->setItem(row, 2, birthdayItem);
            users_table_->setItem(row, 3, idItem);
            row++;
        }
    }

    currentTable->resizeColumnsToContents();
    if (mode_ == Mode::Users && users_table_->columnWidth(3) > 80) {
        users_table_->setColumnWidth(3, 80);
    }
    currentTable->setSortingEnabled(true);

    books_table_->selectionModel()->blockSignals(false);
    users_table_->selectionModel()->blockSignals(false);
}

void MainWindow::UpdateDetailView() {
    QTableWidget* currentTable = (mode_ == Mode::Library) ? books_table_ : users_table_;
    QItemSelectionModel* selectionModel = currentTable->selectionModel();

    selected_book_wptr_.reset();
    selected_user_wptr_.reset();

    bool itemSelectedSuccessfully = false;

    if (selectionModel->hasSelection() && !selectionModel->selectedRows().isEmpty()) {
        int selectedRowInView = selectionModel->selectedRows().first().row();

        if (mode_ == Mode::Library) {
            QVariant rowData = books_table_->item(selectedRowInView, 0)->data(Qt::UserRole);
            if (rowData.isValid() && rowData.canConvert<int>()) {
                int indexInFilteredList = rowData.toInt();
                auto books = library_.SearchBooks(search_edit_->text());
                if (indexInFilteredList >= 0 && static_cast<size_t>(indexInFilteredList) < books.size()) {
                    selected_book_wptr_ = books[indexInFilteredList];
                    if (auto bookPtr = selected_book_wptr_.lock()) {
                        DisplayBookDetails(bookPtr);
                        itemSelectedSuccessfully = true;
                    } else {
                        qWarning() << "UpdateDetailView (Books): Could not lock weak_ptr for selected book index:" << indexInFilteredList;
                    }
                } else {
                    qWarning() << "UpdateDetailView (Books): Invalid index obtained from table item data:" << indexInFilteredList << "for list size:" << books.size();
                }
            } else {
                qWarning() << "UpdateDetailView (Books): Could not get valid data from table item.";
            }
        } else {
            QVariant idData = users_table_->item(selectedRowInView, 0)->data(Qt::UserRole);
            if (idData.isValid() && idData.canConvert<int>()) {
                int userId = idData.toInt();
                selected_user_wptr_ = library_.FindUserById(userId);
                if (auto userPtr = selected_user_wptr_.lock()) {
                    DisplayUserDetails(userPtr);
                    itemSelectedSuccessfully = true;
                } else {
                    qWarning() << "UpdateDetailView (Users): Could not lock weak_ptr for selected user ID:" << userId;
                }
            } else {
                qWarning() << "UpdateDetailView (Users): Could not get valid user ID from table item data.";
            }
        }
    }

    if (!itemSelectedSuccessfully) {
        ClearDetailView();
    }
    UpdateActionButtonsState();
}

void MainWindow::HandleCoverDrop(const QString& filePath) {
    if (detail_stack_->currentWidget() == book_detail_widget_) {
        if (auto bookPtr = selected_book_wptr_.lock()) {
            bookPtr->SetCoverPath(filePath);
            qInfo() << "Cover path set for book '" << bookPtr->GetTitle() << "' to:" << filePath;
            DisplayBookDetails(bookPtr);
            statusBar()->showMessage(tr("Cover updated for '%1'.").arg(bookPtr->GetTitle()), 3000);
        } else {
            QMessageBox::warning(this, tr("Drop Error"), tr("Cannot set cover: The selected book is no longer available."));
            ClearDetailView();
        }
    } else {
        qWarning() << "HandleCoverDrop called while book detail view is not active.";
    }
}

void MainWindow::PerformSearch(const QString&) {
    RefreshTables();
    ClearDetailView();
}

void MainWindow::SelectTextFile() {
    if (detail_stack_->currentWidget() == book_detail_widget_) {
        if (auto bookPtr = selected_book_wptr_.lock()) {
            QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Select Text File for '%1'").arg(bookPtr->GetTitle()),
                QString(),
                tr("Text Files (*.txt);;All Files (*)")
                );

            if (!filePath.isEmpty()) {
                bookPtr->SetTextFilePath(filePath);
                statusBar()->showMessage(tr("Text file set: %1").arg(QFileInfo(filePath).fileName()), 5000);
                qInfo() << "Text file path set for book '" << bookPtr->GetTitle() << "' to:" << filePath;
            } else {
                statusBar()->showMessage(tr("Text file selection cancelled."), 3000);
            }
        } else {
            QMessageBox::warning(this, tr("Selection Error"), tr("Cannot set text file: The selected book is no longer available."));
            ClearDetailView();
        }
    } else {
        qWarning() << "SelectTextFile called while book detail view is not active.";
    }
}

void MainWindow::ShowReadingView() {
    if (detail_stack_->currentWidget() == book_detail_widget_) {
        if (auto bookPtr = selected_book_wptr_.lock()) {
            QString textFilePath = bookPtr->GetTextFilePath();

            if (textFilePath.isEmpty()) {
                QMessageBox::information(this, tr("Reading"), tr("No text file has been set for this book.\nPlease use the 'Set Text File...' button first."));
                return;
            }

            QFile file(textFilePath);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, tr("File Error"), tr("Could not open the text file:\n%1\n\nError: %2")
                                                                 .arg(QDir::toNativeSeparators(textFilePath))
                                                                 .arg(file.errorString()));
                return;
            }

            QTextStream in(&file);
            QString content = in.readAll();
            file.close();

            if (in.status() != QTextStream::Ok) {
                qWarning() << "QTextStream status error after reading. File might not be UTF-8:" << textFilePath;
                QMessageBox::warning(this, tr("Reading Warning"), tr("Could not read the file content correctly using UTF-8. The file might be in a different encoding or corrupted. Displaying potentially incorrect content."));
            }

            book_text_edit_->setPlainText(content);
            detail_stack_->setCurrentWidget(reading_widget_);
            statusBar()->showMessage(tr("Reading '%1'...").arg(bookPtr->GetTitle()));

        } else {
            QMessageBox::warning(this, tr("Reading Error"), tr("Cannot read: The selected book is no longer available."));
            ClearDetailView();
        }
    } else {
        qWarning() << "ShowReadingView called while book detail view is not active.";
    }
}

void MainWindow::GoBackToDetails() {
    UpdateDetailView();
}

void MainWindow::openAssignBookDialog() {
    auto userPtr = selected_user_wptr_.lock();
    if (!userPtr) {
        QMessageBox::warning(this, tr("Assignment Error"), tr("Please select a user first."));
        return;
    }

    AssignBookDialog dialog(library_, this);
    if (dialog.exec() == QDialog::Accepted) {
        std::weak_ptr<Book> selectedBookWkPtr = dialog.getSelectedBook();

        if (!selectedBookWkPtr.expired()) {
            bool success = library_.AssignBookToUser(userPtr->GetId(), selectedBookWkPtr);
            if (success) {
                statusBar()->showMessage(tr("Book assigned successfully!"), 3000);
                DisplayUserDetails(userPtr);
            } else {
                QMessageBox::warning(this, tr("Assignment Failed"), tr("Could not assign the selected book. It might already be assigned to this user, or an error occurred (check logs)."));
            }
        } else {
            QMessageBox::critical(this, tr("Internal Error"), tr("Selected book data is invalid after dialog close."));
            qCritical() << "openAssignBookDialog: Selected book weak_ptr is expired after dialog accept.";
        }
    } else {
        statusBar()->showMessage(tr("Book assignment cancelled."), 3000);
    }
}

void MainWindow::ClearDetailView() {
    selected_book_wptr_.reset();
    selected_user_wptr_.reset();

    set_text_file_button_->setEnabled(false);
    start_reading_button_->setEnabled(false);

    book_title_label_->setText("-");
    book_author_label_->setText("-");
    book_year_label_->setText("-");
    user_name_label_->setText("-");
    user_surname_label_->setText("-");
    user_birthday_label_->setText("-");
    user_id_label_->setText("-");
    book_text_edit_->clear();
    user_borrowed_list_->clear();

    cover_drop_area_->clear();
    cover_drop_area_->setText(tr("Drop cover here"));
    cover_drop_area_->setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 10px; color: #555; }");
    cover_drop_area_->setToolTip("");

    detail_stack_->setCurrentWidget(empty_detail_widget_);
    UpdateActionButtonsState();
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::DisplayBookDetails(std::shared_ptr<Book> bookPtr) {
    book_title_label_->setText(bookPtr->GetTitle());
    book_author_label_->setText(bookPtr->GetAuthor());
    book_year_label_->setText(QString::number(bookPtr->GetYear()));

    QString coverPath = bookPtr->GetCoverPath();
    if (!coverPath.isEmpty() && QFileInfo::exists(coverPath)) {
        QPixmap pixmap(coverPath);
        if (!pixmap.isNull()) {
            cover_drop_area_->setPixmap(pixmap.scaled(cover_drop_area_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            cover_drop_area_->setStyleSheet("QLabel { border: 1px solid #ccc; border-radius: 5px; }");
            cover_drop_area_->setToolTip(QDir::toNativeSeparators(coverPath));
        } else {
            cover_drop_area_->clear();
            cover_drop_area_->setText(tr("Cannot load\ncover image"));
            cover_drop_area_->setStyleSheet("QLabel { border: 2px dashed #cc0000; border-radius: 10px; color: #cc0000; }");
            cover_drop_area_->setToolTip(tr("Invalid image file: %1").arg(QDir::toNativeSeparators(coverPath)));
            qWarning() << "Could not load cover image:" << coverPath;
        }
    } else {
        cover_drop_area_->clear();
        cover_drop_area_->setText(tr("Drop cover here"));
        cover_drop_area_->setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 10px; color: #555; }");
        cover_drop_area_->setToolTip("");
    }

    set_text_file_button_->setEnabled(true);
    start_reading_button_->setEnabled(true);

    detail_stack_->setCurrentWidget(book_detail_widget_);
    statusBar()->showMessage(tr("Selected book: '%1'").arg(bookPtr->GetTitle()));
    UpdateActionButtonsState();
}

void MainWindow::DisplayUserDetails(std::shared_ptr<User> userPtr) {
    user_name_label_->setText(userPtr->GetName());
    user_surname_label_->setText(userPtr->GetSurname());
    user_birthday_label_->setText(userPtr->GetBirthday().toString("dd MMMM yyyy"));
    user_id_label_->setText(QString::number(userPtr->GetId()));

    user_borrowed_list_->clear();
    std::vector<std::shared_ptr<Book>> borrowedBooks = library_.GetUserBorrowedBooks(userPtr->GetId());
    if (borrowedBooks.empty()) {
        QListWidgetItem* noBooksItem = new QListWidgetItem(tr("No books borrowed."));
        noBooksItem->setForeground(Qt::gray);
        noBooksItem->setFlags(noBooksItem->flags() & ~Qt::ItemIsSelectable);
        user_borrowed_list_->addItem(noBooksItem);
    } else {
        std::sort(borrowedBooks.begin(), borrowedBooks.end(), [](const std::shared_ptr<Book>& a, const std::shared_ptr<Book>& b) {
            if (!a || !b) return false;
            return a->GetTitle().compare(b->GetTitle(), Qt::CaseInsensitive) < 0;
        });

        for (const auto& book : borrowedBooks) {
            if (!book) continue;
            QString bookDisplay = QString("%1  (%2)").arg(book->GetTitle()).arg(book->GetAuthor());
            QListWidgetItem* bookItem = new QListWidgetItem(bookDisplay);
            bookItem->setToolTip(tr("Year: %1").arg(book->GetYear()));
            user_borrowed_list_->addItem(bookItem);
        }
    }

    set_text_file_button_->setEnabled(false);
    start_reading_button_->setEnabled(false);
    cover_drop_area_->clear();
    cover_drop_area_->setText("");
    cover_drop_area_->setStyleSheet("");
    cover_drop_area_->setToolTip("");

    detail_stack_->setCurrentWidget(user_detail_widget_);
    UpdateActionButtonsState();
    statusBar()->showMessage(tr("Selected user: %1 %2").arg(userPtr->GetName()).arg(userPtr->GetSurname()));
}

void MainWindow::UpdateActionButtonsState() {
    bool userSelected = !selected_user_wptr_.expired();
    assignBookDialogButton_->setEnabled(userSelected);
    if (userSelected) {
        assignBookDialogButton_->setToolTip(tr("Click to open a dialog and select a book to assign to this user."));
    } else {
        assignBookDialogButton_->setToolTip(tr("Select a user in the list to enable book assignment."));
    }

    bool itemSelected = !selected_book_wptr_.expired() || !selected_user_wptr_.expired();
    delete_button_->setEnabled(itemSelected);
    if (itemSelected) {
        QString itemType = selected_book_wptr_.expired() ? tr("user") : tr("book");
        delete_button_->setToolTip(tr("Delete the currently selected %1 from the list.").arg(itemType));
    } else {
        delete_button_->setToolTip(tr("Select an item in the list to enable deletion."));
    }
}

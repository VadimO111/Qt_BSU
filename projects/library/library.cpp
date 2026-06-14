#include "library.h"
#include <algorithm>
#include <utility>
#include <vector>
#include <QDebug>

Library::Library() {
    AddBook("The Hitchhiker's Guide to the Galaxy", "Douglas Adams", 2005);
    AddBook("Hamlet", "William Shakespeare", 1601);
    AddBook("Geometry and Algebra", "Georgy Razmyslovich", 1987);

    AddUser("Vadim", "Lepeev", QDate(2007, 4, 12));
    AddUser("Elizabeth", "Podoprigo", QDate(2007, 7, 10));
    AddUser("Igor", "Shmygalev", QDate(2007, 7, 4));
    AddUser("Yauheni", "Sheshukou", QDate(2006, 9, 5));
    AddUser("Ivan", "Krokhau", QDate(2007, 1, 11));
}

void Library::AddBook(const QString& title, const QString& author, int year, const QString& coverPath, const QString& textPath) {
    auto newBook = std::make_shared<Book>(title, author, year);
    if (!coverPath.isEmpty()) {
        newBook->SetCoverPath(coverPath);
    }
    if (!textPath.isEmpty()) {
        newBook->SetTextFilePath(textPath);
    }
    books_.push_back(std::move(newBook));
}

std::weak_ptr<Book> Library::FindBookByTitle(const QString& title) {
    auto it = std::find_if(books_.begin(), books_.end(),
                           [&title](const std::shared_ptr<Book>& bookPtr) {
                               return bookPtr && bookPtr->GetTitle().compare(title, Qt::CaseInsensitive) == 0;
                           });
    if (it != books_.end()) {
        return *it;
    }
    return {};
}

std::weak_ptr<Book> Library::FindBookByIndex(int indexInCurrentView, const std::vector<std::shared_ptr<Book>>& currentViewList) {
    if (indexInCurrentView >= 0 && static_cast<size_t>(indexInCurrentView) < currentViewList.size()) {
        return currentViewList[indexInCurrentView];
    }
    qWarning() << "FindBookByIndex: Index out of bounds:" << indexInCurrentView << "for list size:" << currentViewList.size();
    return {};
}

const std::vector<std::shared_ptr<Book>>& Library::GetBooks() const {
    return books_;
}

std::vector<std::shared_ptr<Book>> Library::SearchBooks(const QString& query) const {
    std::vector<std::shared_ptr<Book>> results;
    if (query.isEmpty()) {
        return books_;
    }
    results.reserve(books_.size());
    for (const auto& book : books_) {
        if (book && (book->GetTitle().contains(query, Qt::CaseInsensitive) ||
                     book->GetAuthor().contains(query, Qt::CaseInsensitive))) {
            results.push_back(book);
        }
    }
    return results;
}

void Library::AddUser(const QString& name, const QString& surname, const QDate& birthday) {
    users_.push_back(std::make_shared<User>(name, surname, birthday, next_user_id_++));
}

std::weak_ptr<User> Library::FindUserById(int id) {
    auto it = std::find_if(users_.begin(), users_.end(),
                           [id](const std::shared_ptr<User>& userPtr) {
                               return userPtr && userPtr->GetId() == id;
                           });
    if (it != users_.end()) {
        return *it;
    }
    return {};
}

std::weak_ptr<User> Library::FindUserByIndex(int indexInCurrentView, const std::vector<std::shared_ptr<User>>& currentViewList) {
    if (indexInCurrentView >= 0 && static_cast<size_t>(indexInCurrentView) < currentViewList.size()) {
        return currentViewList[indexInCurrentView];
    }
    qWarning() << "FindUserByIndex: Index out of bounds:" << indexInCurrentView << "for list size:" << currentViewList.size();
    return {};
}


const std::vector<std::shared_ptr<User>>& Library::GetUsers() const {
    return users_;
}

std::vector<std::shared_ptr<User>> Library::SearchUsers(const QString& query) const {
    std::vector<std::shared_ptr<User>> results;
    if (query.isEmpty()) {
        return users_;
    }
    results.reserve(users_.size());
    bool isId = false;
    int queryId = query.toInt(&isId);

    for (const auto& user : users_) {
        if (!user) continue;

        if ((isId && user->GetId() == queryId) ||
            (!isId && (user->GetName().contains(query, Qt::CaseInsensitive) ||
                       user->GetSurname().contains(query, Qt::CaseInsensitive))))
        {
            results.push_back(user);
        }
    }
    return results;
}

bool Library::AssignBookToUser(int userId, std::weak_ptr<Book> bookWkPtr) {
    auto lockedBook = bookWkPtr.lock();
    if (!lockedBook) {
        qWarning() << "AssignBookToUser: Attempt to assign an expired book weak_ptr.";
        return false;
    }

    if (FindUserById(userId).expired()) {
        qWarning() << "AssignBookToUser: User with ID" << userId << "not found.";
        return false;
    }

    auto& borrowedList = user_borrowed_books_[userId];

    for (const auto& existingWkPtr : borrowedList) {
        if (!existingWkPtr.owner_before(bookWkPtr) && !bookWkPtr.owner_before(existingWkPtr)) {
            qInfo() << "AssignBookToUser: Book" << lockedBook->GetTitle() << "already assigned to user ID" << userId;
            return false;
        }
    }

    borrowedList.push_back(bookWkPtr);
    qInfo() << "AssignBookToUser: Assigned book" << lockedBook->GetTitle() << "to user ID" << userId;
    return true;
}

std::vector<std::shared_ptr<Book>> Library::GetUserBorrowedBooks(int userId) const {
    std::vector<std::shared_ptr<Book>> validBooks;
    auto it = user_borrowed_books_.find(userId);
    if (it != user_borrowed_books_.end()) {
        const auto& borrowedList = it->second;
        validBooks.reserve(borrowedList.size());

        for (const auto& bookWkPtr : borrowedList) {
            if (auto bookPtr = bookWkPtr.lock()) {
                validBooks.push_back(bookPtr);
            }
        }
    }
    return validBooks;
}

bool Library::RemoveBook(std::weak_ptr<Book> bookWkPtr) {
    auto bookToRemove = bookWkPtr.lock();
    if (!bookToRemove) {
        qWarning() << "RemoveBook: Attempt to remove an expired book weak_ptr.";
        return false;
    }

    auto it = std::remove_if(books_.begin(), books_.end(),
                             [&bookToRemove](const std::shared_ptr<Book>& currentBook) {
                                 return currentBook == bookToRemove;
                             });

    if (it != books_.end()) {
        books_.erase(it, books_.end());
        qInfo() << "RemoveBook: Book '" << bookToRemove->GetTitle() << "' removed.";
        return true;
    } else {
        qWarning() << "RemoveBook: Book '" << bookToRemove->GetTitle() << "' not found in the library vector.";
        return false;
    }
}

bool Library::RemoveUserById(int userId) {
    auto it = std::remove_if(users_.begin(), users_.end(),
                             [userId](const std::shared_ptr<User>& currentUser) {
                                 return currentUser && currentUser->GetId() == userId;
                             });

    if (it != users_.end()) {
        QString userName = (*it)->GetName();
        QString userSurname = (*it)->GetSurname();
        users_.erase(it, users_.end());

        user_borrowed_books_.erase(userId);

        qInfo() << "RemoveUserById: User '" << userName << " " << userSurname << "' (ID:" << userId << ") removed.";
        return true;
    } else {
        qWarning() << "RemoveUserById: User with ID" << userId << "not found.";
        return false;
    }
}

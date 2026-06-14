#pragma once

#include "book.h"
#include "user.h"
#include <vector>
#include <memory>
#include <QString>
#include <QDate>
#include <map>

class Library {
public:
    Library();

    void AddBook(const QString& title, const QString& author, int year, const QString& coverPath = "", const QString& textPath = "");
    std::weak_ptr<Book> FindBookByTitle(const QString& title);
    std::weak_ptr<Book> FindBookByIndex(int indexInCurrentView, const std::vector<std::shared_ptr<Book>>& currentViewList);
    const std::vector<std::shared_ptr<Book>>& GetBooks() const;
    std::vector<std::shared_ptr<Book>> SearchBooks(const QString& query) const;

    void AddUser(const QString& name, const QString& surname, const QDate& birthday);
    std::weak_ptr<User> FindUserById(int id);
    std::weak_ptr<User> FindUserByIndex(int indexInCurrentView, const std::vector<std::shared_ptr<User>>& currentViewList);
    const std::vector<std::shared_ptr<User>>& GetUsers() const;
    std::vector<std::shared_ptr<User>> SearchUsers(const QString& query) const;

    bool AssignBookToUser(int userId, std::weak_ptr<Book> bookWkPtr);

    std::vector<std::shared_ptr<Book>> GetUserBorrowedBooks(int userId) const;

    bool RemoveBook(std::weak_ptr<Book> bookWkPtr);
    bool RemoveUserById(int userId);


private:
    std::vector<std::shared_ptr<Book>> books_;
    std::vector<std::shared_ptr<User>> users_;
    int next_user_id_ = 1;
    std::map<int, std::vector<std::weak_ptr<Book>>> user_borrowed_books_;
};

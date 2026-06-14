#include "book.h"

Book::Book(QString title, QString author, int year) :
    title_(std::move(title)), author_(std::move(author)), year_(year) {
}

QString Book::GetTitle() const {
    return title_;
}

QString Book::GetAuthor() const {
    return author_;
}

int Book::GetYear() const {
    return year_;
}

QString Book::GetCoverPath() const {
    return cover_path_;
}

QString Book::GetTextFilePath() const {
    return text_file_path_;
}

void Book::SetCoverPath(const QString& path) {
    cover_path_ = path;
}

void Book::SetTextFilePath(const QString& path) {
    text_file_path_ = path;
}

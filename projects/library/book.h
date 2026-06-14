#pragma once

#include <QString>

const int inf = 2e9;

class Book {
public:
    Book(QString title, QString author, int year);

    QString GetTitle() const;
    QString GetAuthor() const;
    int GetYear() const;
    QString GetCoverPath() const;
    QString GetTextFilePath() const;

    void SetCoverPath(const QString& path);
    void SetTextFilePath(const QString& path);

private:
    QString title_ = "-";
    QString author_ = "-";
    int year_ = inf;
    QString cover_path_ = "";
    QString text_file_path_ = "";
};

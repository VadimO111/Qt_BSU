#pragma once

#include <QLabel>
#include <QString>

class CoverDropArea : public QLabel {
    Q_OBJECT

public:
    explicit CoverDropArea(QWidget* parent = nullptr);
    ~CoverDropArea() = default;

signals:
    void coverDropped(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;

private:
    bool IsImageFile(const QString &filePath);
};

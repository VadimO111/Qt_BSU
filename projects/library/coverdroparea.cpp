#include "coverdroparea.h"
#include <QWidget>
#include <QPixmap>
#include <QDropEvent>
#include <QMimeData>

CoverDropArea::CoverDropArea(QWidget* parent) : QLabel(parent) {
    setFixedSize(180, 320);
    setAlignment(Qt::AlignCenter);
    setText(tr("Drag book\ncover here"));
    setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 10px; color: #555; }");
    setAcceptDrops(true);
    setScaledContents(false);
}

void CoverDropArea::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        if (event->mimeData()->urls().count() == 1) {
            QUrl url = event->mimeData()->urls().first();
            if (url.isLocalFile() && IsImageFile(url.toLocalFile())) {
                event->acceptProposedAction();
                setStyleSheet("QLabel { border: 2px solid #5D9B90; background-color: #E0F2F1; border-radius: 10px; color: #3E8277; }");
                return;
            }
        }
    }
    event->ignore();
    setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 10px; color: #555; }");
}

void CoverDropArea::dragLeaveEvent(QDragLeaveEvent *event) {
    setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 10px; color: #555; }");
    QLabel::dragLeaveEvent(event);
}


void CoverDropArea::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QUrl url = mimeData->urls().first();
        QString filePath = url.toLocalFile();
        if (IsImageFile(filePath)) {
            QPixmap pixmap(filePath);
            if (!pixmap.isNull()) {
                setPixmap(pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                emit coverDropped(filePath);
                event->acceptProposedAction();
                setStyleSheet("QLabel { border: 2px solid #5D9B90; border-radius: 10px; }");
                return;
            }
        }
    }
    event->ignore();
    setStyleSheet("QLabel { border: 2px dashed #aaa; border-radius: 10px; color: #555; }");
}

bool CoverDropArea::IsImageFile(const QString& filePath) {
    QStringList imageExtensions = {".jpg", ".jpeg", ".png", ".bmp", ".gif"};
    for (const QString& ext : imageExtensions) {
        if (filePath.endsWith(ext, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

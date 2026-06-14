#pragma once

#include "library.h"
#include "coverdroparea.h"
#include "itemdialog.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QTextEdit>
#include <QListWidget>

extern const QString RadioButtonStyle;
extern const QString TableHeaderStyle;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void ButtonChanged(QAbstractButton* button);
    void AddItem();
    void RefreshTables();
    void UpdateDetailView();
    void PerformSearch(const QString& query);
    void deleteSelectedItem();

    void HandleCoverDrop(const QString& filePath);
    void SelectTextFile();
    void ShowReadingView();
    void GoBackToDetails();

    void openAssignBookDialog();


private:
    void SetupUi();
    void SetupConnections();
    void ClearDetailView();
    void DisplayBookDetails(std::shared_ptr<Book> bookPtr);
    void DisplayUserDetails(std::shared_ptr<User> userPtr);
    void UpdateActionButtonsState();

    Library library_;
    Mode mode_ = Mode::Library;
    std::weak_ptr<Book> selected_book_wptr_;
    std::weak_ptr<User> selected_user_wptr_;
    QWidget* central_widget_ = nullptr;
    QHBoxLayout* main_h_layout_ = nullptr;
    QSplitter* splitter_ = nullptr;
    QWidget* left_panel_widget_ = nullptr;
    QVBoxLayout* left_v_layout_ = nullptr;
    QHBoxLayout* radio_button_layout_ = nullptr;
    QLineEdit* search_edit_ = nullptr;
    QHBoxLayout* control_button_layout_ = nullptr;
    QPushButton* add_button_ = nullptr;
    QPushButton* delete_button_ = nullptr;
    QButtonGroup* mode_buttons_ = nullptr;
    QRadioButton* books_button_ = nullptr;
    QRadioButton* users_button_ = nullptr;
    QStackedWidget* table_stack_ = nullptr;
    QTableWidget* books_table_ = nullptr;
    QTableWidget* users_table_ = nullptr;
    QWidget* right_panel_widget_ = nullptr;
    QVBoxLayout* right_v_layout_ = nullptr;
    QStackedWidget* detail_stack_ = nullptr;
    QWidget* book_detail_widget_ = nullptr;
    QFormLayout* book_detail_layout_ = nullptr;
    QLabel* book_title_label_ = nullptr;
    QLabel* book_author_label_ = nullptr;
    QLabel* book_year_label_ = nullptr;
    CoverDropArea* cover_drop_area_ = nullptr;
    QPushButton* set_text_file_button_ = nullptr;
    QPushButton* start_reading_button_ = nullptr;
    QWidget* user_detail_widget_ = nullptr;
    QVBoxLayout* user_detail_main_layout_ = nullptr;
    QFormLayout* user_info_layout_ = nullptr;
    QLabel* user_name_label_ = nullptr;
    QLabel* user_surname_label_ = nullptr;
    QLabel* user_birthday_label_ = nullptr;
    QLabel* user_id_label_ = nullptr;
    QLabel* user_borrowed_label_ = nullptr;
    QListWidget* user_borrowed_list_ = nullptr;
    QPushButton* assignBookDialogButton_ = nullptr;
    QWidget* reading_widget_ = nullptr;
    QVBoxLayout* reading_layout_ = nullptr;
    QTextEdit* book_text_edit_ = nullptr;
    QPushButton* back_to_details_button_ = nullptr;
    QWidget* empty_detail_widget_ = nullptr;
    QLabel* empty_detail_label_ = nullptr;
};

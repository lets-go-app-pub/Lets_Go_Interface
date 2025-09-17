//
// Created by jeremiah on 9/5/21.
//

#include <sstream>
#include <QMessageBox>
#include <QFileDialog>
#include <QBuffer>
#include <QImageReader>

#include "user_login_info.h"
#include "set_fields_window.h"
#include "ui_set_fields_window.h"
#include "homewindow.h"
#include "general_utility.h"

SetFieldsWindow::SetFieldsWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::SetFieldsWindow) {
    ui->setupUi(this);

    ui->setFieldsScrollArea->setVisible(false);
    ui->viewFieldsTitleLabel->setVisible(false);

    ui->activityNameLineEdit->setMaxLength(globals->maximum_activity_or_category_name_size());

    ui->activityMinAgeSpinBox->setMinimum(globals->lowest_allowed_age());
    ui->activityMinAgeSpinBox->setMaximum(globals->highest_allowed_age());
    ui->activityMinAgeSpinBox->setValue(globals->lowest_allowed_age());

    ui->basicIconDisplayIconLabel->setMaximumSize(
            (int) globals->activity_icon_width_in_pixels(),
            (int) globals->activity_icon_height_in_pixels()
    );

    ui->basicIconDisplayIconLabel->setFixedSize(
            (int) globals->activity_icon_width_in_pixels(),
            (int) globals->activity_icon_height_in_pixels()
    );

    if (user_admin_privileges.update_activities_and_categories()) {
        ui->setFieldsScrollArea->setVisible(true);
        ui->viewFieldsTitleLabel->setVisible(true);
        ui->selectTypeActivityRadioButton->setVisible(true);
        ui->selectTypeCategoryRadioButton->setVisible(true);
    } else {
        ui->selectTypeActivityRadioButton->setVisible(false);
        ui->selectTypeCategoryRadioButton->setVisible(false);
    }

    if (user_admin_privileges.update_icons()) {
        ui->setFieldsScrollArea->setVisible(true);
        ui->viewFieldsTitleLabel->setVisible(true);
        ui->selectTypeIconRadioButton->setVisible(true);
    } else {
        ui->selectTypeIconRadioButton->setVisible(false);
    }

    ui->basicIconTitleLabel->setText(QString("Icon Image (%1x%2)").arg(globals->activity_icon_height_in_pixels()).arg(
            globals->activity_icon_width_in_pixels()));

    connect(ui->viewFieldsTreeWidget, &QTreeWidget::itemDoubleClicked, this,
            &SetFieldsWindow::slot_treeWidgetItemDoubleClicked);

    connect(&request_icons_object, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &SetFieldsWindow::slot_requestCompleted);
    connect(&request_icons_object, &RequestIconsObject::signal_sendWarning, this,
            &SetFieldsWindow::slot_sendWarning);
    connect(&request_icons_object, &RequestIconsObject::signal_requestCanceled, this,
            &SetFieldsWindow::slot_requestCanceled);

    connect(&request_activities_categories_object, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &SetFieldsWindow::slot_requestCompleted);
    connect(&request_activities_categories_object, &RequestIconsObject::signal_sendWarning, this,
            &SetFieldsWindow::slot_sendWarning);
    connect(&request_activities_categories_object, &RequestIconsObject::signal_requestCanceled, this,
            &SetFieldsWindow::slot_requestCanceled);

    connect(&set_server_activities, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &SetFieldsWindow::slot_requestCompleted);
    connect(&set_server_activities, &RequestIconsObject::signal_sendWarning, this,
            &SetFieldsWindow::slot_sendWarning);
    connect(&set_server_activities, &RequestIconsObject::signal_requestCanceled, this,
            &SetFieldsWindow::slot_requestCanceled);

    connect(&set_server_category, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &SetFieldsWindow::slot_requestCompleted);
    connect(&set_server_category, &RequestIconsObject::signal_sendWarning, this,
            &SetFieldsWindow::slot_sendWarning);
    connect(&set_server_category, &RequestIconsObject::signal_requestCanceled, this,
            &SetFieldsWindow::slot_requestCanceled);

    connect(&set_server_icon, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &SetFieldsWindow::slot_requestCompleted);
    connect(&set_server_icon, &RequestIconsObject::signal_sendWarning, this,
            &SetFieldsWindow::slot_sendWarning);
    connect(&set_server_icon, &RequestIconsObject::signal_requestCanceled, this,
            &SetFieldsWindow::slot_requestCanceled);

    ui->viewFieldsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->viewFieldsTreeWidget->setExpandsOnDoubleClick(false);
    ui->viewFieldsTreeWidget->setVerticalScrollMode(QTreeView::ScrollPerPixel);

    ui->selectTypeActivityRadioButton->setChecked(true);
    setupUIForSelectTypeRadioButton();
}

void SetFieldsWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

void SetFieldsWindow::setupUIForSelectTypeRadioButton() {

    if (ui->selectTypeCategoryRadioButton->isChecked()) {
        setupUIForCategoryTypeRadioButton();
    } else if (ui->selectTypeIconRadioButton->isChecked()) {
        setupUIForIconTypeRadioButton();
    } else if (ui->selectTypeActivityRadioButton->isChecked()) {
        setupUIForActivityTypeRadioButton();
    } else {

        ui->setFieldsScrollArea->setVisible(false);
        QMessageBox::warning(this,
                             "Error",
                             "None of the radio buttons inside layout selectTypeVerticalWidget were selected."
        );
        return;
    }

    setupListViewWidgetForActivities();
}

void SetFieldsWindow::addQTreeWidgetItemForActivity(
        const std::string& icon_string,
        const unsigned int icon_string_length,
        const int activity_index,
        const QString& text
) {

    auto* my_item = new QTreeWidgetItem(ui->viewFieldsTreeWidget);

    QPixmap pixmap;
    pixmap.loadFromData(reinterpret_cast<const uchar*>(icon_string.c_str()), icon_string_length);
    my_item->setIcon(0, pixmap);
    my_item->setText(1, text);
    my_item->setData(0, Qt::UserRole, QPoint(QTreeWidgetItemType::TREE_ITEM_TYPE_ACTIVITY, activity_index));
    my_item->setData(1, Qt::UserRole, QPoint(QTreeWidgetItemType::TREE_ITEM_TYPE_ACTIVITY, activity_index));
}

void SetFieldsWindow::addQTreeWidgetItemForCategory(
        const std::string& color_string,
        const QString& text,
        const int category_index,
        const int number_columns
) {

    auto* my_item = new QTreeWidgetItem(ui->viewFieldsTreeWidget);

    my_item->setFlags(my_item->flags() & ~Qt::ItemIsSelectable);

    my_item->setText(0, text);
    for (int i = 0; i < number_columns; i++) {
        my_item->setForeground(i, QColorConstants::White);
        my_item->setBackground(i, QColor(color_string.c_str()));
        my_item->setData(i, Qt::UserRole, QPoint(QTreeWidgetItemType::TREE_ITEM_TYPE_CATEGORY, category_index));
    }

    bool activity_set = false;
    auto* activity_row_item = new QTreeWidgetItem();
    activity_row_item->setFlags(my_item->flags() & ~Qt::ItemIsSelectable);
    int column_number = 0;
    std::unique_lock<std::mutex> activities_lock(activities_mutex);
    for (const auto& activity: activities) {
        if (activity.category_index() == category_index) {
            activity_set = true;

            QPixmap pixmap;
            pixmap.loadFromData(reinterpret_cast<const uchar*>(icons[activity.icon_index()].icon_in_bytes().c_str()),
                                icons[activity.icon_index()].icon_size_in_bytes());

            activity_row_item->setText(column_number, activity.display_name().c_str());
            activity_row_item->setIcon(column_number, pixmap);
            activity_row_item->setData(column_number, Qt::UserRole,
                                       QPoint(QTreeWidgetItemType::TREE_ITEM_TYPE_ACTIVITY, activity.index()));

            column_number++;
            if (column_number >= number_columns) {
                column_number = 0;
                my_item->addChild(activity_row_item);

                activity_set = false;
                activity_row_item = new QTreeWidgetItem();
                activity_row_item->setFlags(my_item->flags() & ~Qt::ItemIsSelectable);
            }
        }
    }
    activities_lock.unlock();

    if (activity_set) {
        my_item->addChild(activity_row_item);
    }
}

void SetFieldsWindow::addQTreeWidgetItemForIcon(
        const request_fields::ServerIconsResponse& icons_message,
        const unsigned long icon_index,
        const QString& text
) {

    auto* my_item = new QTreeWidgetItem(ui->viewFieldsTreeWidget);
    QPixmap pixmap;

    my_item->setText(0, text);

    if (icons_message.is_active()) { //icon is active
        pixmap.loadFromData(reinterpret_cast<const uchar*>(icons_message.icon_in_bytes().c_str()),
                            icons_message.icon_size_in_bytes());
        my_item->setIcon(1, pixmap);
    } else { //icon is inactive
        my_item->setText(1, "Icon is inactive.");
    }

    for (int i = 0; i < 4; i++) {
        my_item->setData(i, Qt::UserRole, QPoint(QTreeWidgetItemType::TREE_ITEM_TYPE_ICON, (int) icon_index));
    }

}

void SetFieldsWindow::slot_treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column) {

    std::cout << "slot_treeWidgetItemDoubleClicked() running\n";

    //stored these in a type point instead of a custom type because just needed
    // 2 numbers to pass through
    QPoint data = item->data(column, Qt::UserRole).toPoint();
    auto item_type = QTreeWidgetItemType(data.x());
    int index_num = data.y();

    std::cout << "QTreeWidgetItemType: " << item_type << " index_num: " << index_num << "\n";

    switch (item_type) {
        case TREE_ITEM_TYPE_ACTIVITY: {
            if (index_num < activities.size()) {
                ui->activityNameLineEdit->setText(activities[index_num].display_name().c_str());
                ui->iconDisplayNameLineEdit->setText(activities[index_num].icon_display_name().c_str());
                ui->activityMinAgeSpinBox->setValue(activities[index_num].min_age());
                ui->categoryIndexSpinBox->setValue(activities[index_num].category_index());
                ui->iconIndexSpinBox->setValue(activities[index_num].icon_index());
                ui->deleteActivityCheckBox->setChecked(false);
                ui->selectTypeActivityRadioButton->setChecked(true);
            }
            break;
        }
        case TREE_ITEM_TYPE_CATEGORY: {
            if (index_num < categories.size()) {
                ui->activityNameLineEdit->setText(categories[index_num].display_name().c_str());
                ui->categoryOrderNumberDoubleSpinBox->setValue(categories[index_num].order_number());
                ui->activityMinAgeSpinBox->setValue(categories[index_num].min_age());
                ui->colorCodeLineEdit->setText(categories[index_num].color().c_str());
                ui->deleteActivityCheckBox->setChecked(false);
                ui->selectTypeCategoryRadioButton->setChecked(true);
            }
            break;
        }
        case TREE_ITEM_TYPE_ICON: {
            if (index_num < (int) icons.size()) {
                ui->selectOperationUpdateRadioButton->setChecked(true);
                ui->iconIndexSpinBox->setValue((int) icons[index_num].index_number());

                if (icons[index_num].is_active()) { //icon is active

                    auto setup_image = [](
                            const std::string& image_string,
                            int image_size,
                            QByteArray& bytes
                    ) -> QImage {
                        QImage image;
                        image.loadFromData(reinterpret_cast<const uchar*>(image_string.c_str()), image_size, "PNG");
                        image = image.scaled(
                                (int) globals->activity_icon_width_in_pixels(),
                                (int) globals->activity_icon_height_in_pixels(),
                                Qt::AspectRatioMode::KeepAspectRatio);

                        bytes.clear();

                        QBuffer buffer(&bytes);
                        buffer.open(QIODevice::WriteOnly);

                        //NOTE: do NOT set a quality rating for .save(), leave it at -1. It will increase the
                        // size of the file needlessly.
                        image.save(&buffer, "PNG");

                        return image;
                    };

                    ui->basicIconDisplayIconLabel->setPixmap(
                            QPixmap::fromImage(
                                    setup_image(
                                            icons[index_num].icon_in_bytes(),
                                            icons[index_num].icon_size_in_bytes(),
                                            basic_image_bytes
                                    )
                            )
                    );

                } else { //icon is inactive
                    basic_image_bytes.clear();

                    QPixmap pixmap(":/empty_icon.png");

                    ui->basicIconDisplayIconLabel->setPixmap(pixmap);
                }

                ui->iconActiveCheckBox->setChecked(icons[index_num].is_active());
                ui->selectTypeIconRadioButton->setChecked(true);
            }
            break;
        }
        default: {
            QMessageBox::warning(
                    this,
                    "Error",
                    QString(
                            "Invalid QTreeWidgetItemType found from data() inside of"
                            " slot_treeWidgetItemDoubleClicked(). Type of %1."
                    )
                            .arg(item_type)
            );
            return;
        }
    }
}

void SetFieldsWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void SetFieldsWindow::slot_requestCompleted(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void SetFieldsWindow::slot_sendWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
        completed_lambda();
    }
    std::cout << "title: " << title.toStdString();
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(this,
                         title,
                         text
    );
}

void SetFieldsWindow::setupUIForActivityTypeRadioButton() {
    ui->selectOperationTitleLabel->setVisible(false);
    ui->selectOperationVerticalWidget->setVisible(false);

    ui->activityNameTitleLabel->setVisible(true);
    ui->activityNameTitleLabel->setText(
            (ACTIVITY + SET_FIELDS_NAME_TITLE_MIDDLE_ONE + "and category index match" +
             SET_FIELDS_NAME_TITLE_MIDDLE_TWO + ACTIVITY_LOWER + SET_FIELDS_NAME_TITLE_END).c_str());
    ui->activityNameLineEdit->setVisible(true);

    ui->iconDisplayNameTitleLabel->setVisible(true);
    ui->iconDisplayNameLineEdit->setVisible(true);

    ui->activityMinAgeTitleLabel->setVisible(true);
    ui->activityMinAgeTitleLabel->setText((ACTIVITY + SET_FIELDS_MIN_AGE_TITLE).c_str());
    ui->activityMinAgeSpinBox->setVisible(true);

    ui->categoryOrderNumberLabel->setVisible(false);
    ui->categoryOrderNumberDoubleSpinBox->setVisible(false);

    ui->colorCodeTitleLabel->setVisible(false);
    ui->colorCodeHorizontalWidget->setVisible(false);

    ui->categoryIndexTitleLabel->setVisible(true);
    ui->categoryIndexSpinBox->setVisible(true);

    ui->iconIndexTitleLabel->setVisible(true);
    ui->iconIndexSpinBox->setVisible(true);

    ui->deleteActivityCheckBox->setVisible(true);
    ui->deleteActivityCheckBox->setText((SET_FIELDS_DELETE_TEXT + ACTIVITY).c_str());

    ui->iconActiveCheckBox->setVisible(false);

    ui->basicIconTitleLabel->setVisible(false);
    ui->basicIconHorizontalWidget->setVisible(false);

}

void SetFieldsWindow::setupUIForCategoryTypeRadioButton() {
    ui->selectOperationTitleLabel->setVisible(false);
    ui->selectOperationVerticalWidget->setVisible(false);

    ui->activityNameTitleLabel->setVisible(true);
    ui->activityNameTitleLabel->setText(
            (CATEGORY + SET_FIELDS_NAME_TITLE_MIDDLE_ONE + "matches" + SET_FIELDS_NAME_TITLE_MIDDLE_TWO +
             CATEGORY_LOWER + SET_FIELDS_NAME_TITLE_END).c_str());
    ui->activityNameLineEdit->setVisible(true);

    ui->iconDisplayNameTitleLabel->setVisible(false);
    ui->iconDisplayNameLineEdit->setVisible(false);

    ui->activityMinAgeTitleLabel->setVisible(true);
    ui->activityMinAgeTitleLabel->setText((CATEGORY + SET_FIELDS_MIN_AGE_TITLE).c_str());
    ui->activityMinAgeSpinBox->setVisible(true);

    ui->categoryOrderNumberLabel->setVisible(true);
    ui->categoryOrderNumberDoubleSpinBox->setVisible(true);

    ui->colorCodeTitleLabel->setVisible(true);
    ui->colorCodeHorizontalWidget->setVisible(true);

    ui->categoryIndexTitleLabel->setVisible(false);
    ui->categoryIndexSpinBox->setVisible(false);

    ui->iconIndexTitleLabel->setVisible(false);
    ui->iconIndexSpinBox->setVisible(false);

    ui->deleteActivityCheckBox->setVisible(true);
    ui->deleteActivityCheckBox->setText((SET_FIELDS_DELETE_TEXT + CATEGORY).c_str());

    ui->iconActiveCheckBox->setVisible(false);

    ui->basicIconTitleLabel->setVisible(false);
    ui->basicIconHorizontalWidget->setVisible(false);

}

void SetFieldsWindow::setupUIForIconTypeRadioButton() {
    ui->selectOperationTitleLabel->setVisible(true);
    ui->selectOperationVerticalWidget->setVisible(true);

    ui->activityNameTitleLabel->setVisible(false);
    ui->activityNameLineEdit->setVisible(false);

    ui->iconDisplayNameTitleLabel->setVisible(false);
    ui->iconDisplayNameLineEdit->setVisible(false);

    ui->activityMinAgeTitleLabel->setVisible(false);
    ui->activityMinAgeSpinBox->setVisible(false);

    ui->categoryOrderNumberLabel->setVisible(false);
    ui->categoryOrderNumberDoubleSpinBox->setVisible(false);

    ui->colorCodeTitleLabel->setVisible(false);
    ui->colorCodeHorizontalWidget->setVisible(false);

    ui->categoryIndexTitleLabel->setVisible(false);
    ui->categoryIndexSpinBox->setVisible(false);

    if (ui->selectOperationUpdateRadioButton->isChecked()) {
        ui->iconIndexTitleLabel->setVisible(true);
        ui->iconIndexSpinBox->setVisible(true);
    } else if (ui->selectOperationInsertRadioButton->isChecked()) {
        ui->iconIndexTitleLabel->setVisible(false);
        ui->iconIndexSpinBox->setVisible(false);
    } else {
        QMessageBox::warning(this,
                             "Error",
                             "None of the radio buttons inside layout selectOperationVerticalWidget were selected."
        );
        return;
    }

    ui->deleteActivityCheckBox->setVisible(false);

    ui->iconActiveCheckBox->setVisible(true);

    ui->basicIconTitleLabel->setVisible(true);
    ui->basicIconHorizontalWidget->setVisible(true);

}

void SetFieldsWindow::setupListViewWidgetForActivities() {

    currently_displaying = TreeDisplayingType::DISPLAYING_ACTIVITIES;

    const int number_columns = 2;

    ui->viewFieldsTreeWidget->clear();
    ui->viewFieldsTreeWidget->setColumnCount(number_columns);

    std::unique_lock<std::mutex> activities_lock(activities_mutex);
    for (const auto& activity: activities) {
        std::stringstream text;

        text
                << "Activity: " << activity.display_name() << '\n'
                << "Icon Display Name: " << activity.icon_display_name() << '\n'
                << "Category Name: " << categories[activity.category_index()].display_name() << '\n';

        if (activity.min_age() < 121) {
            text
                    << "Min Age: " << activity.min_age() << '\n';
        } else {
            text
                    << "Min Age: ACTIVITY DISABLED" << '\n';
        }

        text
                << "Activity Index: " << activity.index() << '\n'
                << "Category Index: " << activity.category_index() << '\n'
                << "Icon Index: " << activity.icon_index() << '\n';

        addQTreeWidgetItemForActivity(
                icons[activity.icon_index()].icon_in_bytes(),
                icons[activity.icon_index()].icon_size_in_bytes(),
                activity.index(),
                QString(text.str().c_str())
        );
    }

    ui->viewFieldsTreeWidget->viewport()->update();
}

void SetFieldsWindow::setupListViewWidgetForCategories() {

    currently_displaying = TreeDisplayingType::DISPLAYING_CATEGORIES;

    const int number_columns = 3;

    ui->viewFieldsTreeWidget->clear();
    ui->viewFieldsTreeWidget->setColumnCount(number_columns);

    std::vector<CategoriesIndexValues> categories_by_order_number;

    for (const auto& category: categories) {
        categories_by_order_number.emplace_back(
                category.index(),
                category.order_number()
        );
    }

    //< operator is overloaded inside CategoriesIndexValues for proper ordering
    std::sort(categories_by_order_number.begin(), categories_by_order_number.end());

    std::unique_lock<std::mutex> activities_lock(categories_mutex);
    for (const auto& order_number: categories_by_order_number) {
        std::stringstream text;

        const auto& category = categories[order_number.index];;

        text
                << "Category: " << category.display_name() << '\n';

        if (category.min_age() < 121) {
            text
                    << "Min Age: " << category.min_age() << '\n';
        } else {
            text
                    << "Min Age: CATEGORY DISABLED" << '\n';
        }

        text
                << "Category Index: " << category.index() << '\n'
                << "Order Number: " << category.order_number() << '\n';

        addQTreeWidgetItemForCategory(
                category.color(),
                QString(text.str().c_str()),
                category.index(),
                number_columns
        );
    }

    ui->viewFieldsTreeWidget->viewport()->update();
}

void SetFieldsWindow::setupListViewWidgetForIcons() {

    currently_displaying = TreeDisplayingType::DISPLAYING_ICONS;

    const int number_columns = 2;

    ui->viewFieldsTreeWidget->clear();
    ui->viewFieldsTreeWidget->setColumnCount(number_columns);

    std::unique_lock<std::mutex> activities_lock(icons_mutex);
    for (const auto& icon: icons) {
        std::stringstream text;

        addQTreeWidgetItemForIcon(
                icon,
                icon.index_number(),
                ("Index: " + std::to_string(icon.index_number()) + "\t").c_str());
    }

    ui->viewFieldsTreeWidget->viewport()->update();
}

void SetFieldsWindow::on_selectTypeCategoryRadioButton_toggled(bool checked) {
    if (checked) {
        setupUIForCategoryTypeRadioButton();
    }
}

void SetFieldsWindow::on_selectTypeActivityRadioButton_toggled(bool checked) {
    if (checked) {
        setupUIForActivityTypeRadioButton();
    }
}

void SetFieldsWindow::on_selectTypeIconRadioButton_toggled(bool checked) {
    if (checked) {
        setupUIForIconTypeRadioButton();
    }
}

void SetFieldsWindow::on_selectOperationUpdateRadioButton_toggled(bool checked) {
    if (checked) {
        ui->iconIndexTitleLabel->setVisible(true);
        ui->iconIndexSpinBox->setVisible(true);
        ui->iconActiveCheckBox->setVisible(true);
    }
}

void SetFieldsWindow::on_selectOperationInsertRadioButton_toggled(bool checked) {
    if (checked) {
        ui->iconIndexTitleLabel->setVisible(false);
        ui->iconIndexSpinBox->setVisible(false);
        ui->iconActiveCheckBox->setVisible(false);
        ui->iconActiveCheckBox->setChecked(true);
    }
}

void SetFieldsWindow::on_viewFieldsActivitiesPushButton_clicked() {
    setupListViewWidgetForActivities();
}

void SetFieldsWindow::on_viewFieldsCategoriesPushButton_clicked() {
    setupListViewWidgetForCategories();
}

void SetFieldsWindow::on_viewFieldsIconsPushButton_clicked() {
    setupListViewWidgetForIcons();
}

void SetFieldsWindow::refreshTreeWidget() {
    switch (currently_displaying) {
        case DISPLAYING_ACTIVITIES:
            setupListViewWidgetForActivities();
            break;
        case DISPLAYING_CATEGORIES:
            setupListViewWidgetForCategories();
            break;
        case DISPLAYING_ICONS:
            setupListViewWidgetForIcons();
            break;
    }
}

void SetFieldsWindow::setUpUIForRequestingIconsCompleted() {
    requesting_icons = false;

    if (!requesting_activities_categories) {
        ui->viewFieldsRefreshPushButton->setEnabled(true);
        refreshTreeWidget();
    }
}

void SetFieldsWindow::setUpUIForRequestingActivitiesCategoriesCompleted() {
    requesting_activities_categories = false;

    if (!requesting_icons) {
        ui->viewFieldsRefreshPushButton->setEnabled(true);
        refreshTreeWidget();
    }
}

void SetFieldsWindow::updateActivitiesCategoriesAndIcons() {
    ui->viewFieldsRefreshPushButton->setEnabled(false);

    requesting_icons = true;
    request_icons_object.runRequestIcons([&]() {
        setUpUIForRequestingIconsCompleted();
    });

    requesting_activities_categories = true;
    request_activities_categories_object.runRequestActivitiesCategories([&]() {
        setUpUIForRequestingActivitiesCategoriesCompleted();
    });
}

void SetFieldsWindow::on_viewFieldsRefreshPushButton_clicked() {
    updateActivitiesCategoriesAndIcons();
}

bool SetFieldsWindow::colorCodeIsValid(const std::string& color_code) {

    if (color_code.size() != 7
        || color_code[0] != '#') {
        return false;
    }

    for (unsigned int i = 1; i < color_code.size(); i++) {
        if (!std::isxdigit(color_code[i])) {
            return false;
        }
    }

    return true;
}

void SetFieldsWindow::on_setFieldsSubmitPushButton_clicked() {

    if (ui->selectTypeActivityRadioButton->isChecked()) {

        const std::string name = ui->activityNameLineEdit->text().toStdString();

        if (name.size() < globals->minimum_activity_or_category_name_size()) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Please enter a valid activity name."
            );
            return;
        }

        std::string icon_display_name = ui->iconDisplayNameLineEdit->text().toStdString();

        //icon_display_name can be empty, it will be set inside runSetServerActivity()
        if (!icon_display_name.empty() &&
            icon_display_name.size() < globals->minimum_activity_or_category_name_size()) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Please enter a valid activity name."
            );
            return;
        }

        ui->setFieldsSubmitPushButton->setEnabled(false);

        set_server_activities.runSetServerActivity(
                ui->deleteActivityCheckBox->isChecked(),
                name,
                icon_display_name,
                ui->activityMinAgeSpinBox->value(),
                ui->categoryIndexSpinBox->value(),
                ui->iconIndexSpinBox->value(),
                [&]() {
                    ui->setFieldsSubmitPushButton->setEnabled(true);
                });

    } else if (ui->selectTypeCategoryRadioButton->isChecked()) {

        const std::string name = ui->activityNameLineEdit->text().toStdString();

        if (name.size() < globals->minimum_activity_or_category_name_size()) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Please enter a valid activity name."
            );
            return;
        }

        const double order_number = ui->categoryOrderNumberDoubleSpinBox->value();

        for (const auto& category: categories) {
            if (category.order_number() == order_number) {
                QMessageBox::warning(this,
                                     "Error",
                                     QString("Order number is already used by category '")
                                             .append(QString::fromStdString(category.display_name()))
                                             .append("'.")
                );
                return;
            }
        }

        std::string color_code = ui->colorCodeLineEdit->text().toStdString();

        if (!colorCodeIsValid(color_code)) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Please enter a valid color code."
            );
            return;
        }


        std::string upper_case_color_code;

        //convert string to lower case
        for (const char c: color_code) {
            upper_case_color_code += isalpha(int(c)) ? char(toupper(c)) : c;
        }

        ui->setFieldsSubmitPushButton->setEnabled(false);

        set_server_category.runSetServerCategory(
                ui->deleteActivityCheckBox->isChecked(),
                name,
                name,
                order_number,
                ui->activityMinAgeSpinBox->value(),
                upper_case_color_code,
                [&]() {
                    ui->setFieldsSubmitPushButton->setEnabled(true);
                });

    } else if (ui->selectTypeIconRadioButton->isChecked()) {

        //NOTE: the UI will prevent a lot of problems, for example Icons Active cannot
        // check set to false when Insert is selected

        if (basic_image_bytes.isEmpty()) {
            QMessageBox::warning(this,
                                 "Error",
                                 QString("Please make sure icon is selected.\nIcon image size: %1.").arg(
                                         basic_image_bytes.size())
            );
            return;
        }

        ui->setFieldsSubmitPushButton->setEnabled(false);

        if (ui->iconActiveCheckBox->isChecked()) { //icon is active
            set_server_icon.runSetServerIcon(
                    ui->selectOperationInsertRadioButton->isChecked(),
                    ui->iconIndexSpinBox->value(),
                    ui->iconActiveCheckBox->isChecked(),
                    basic_image_bytes,
                    [&]() {
                        ui->setFieldsSubmitPushButton->setEnabled(true);
                    }
            );
        } else { //icon is inactive

            //sending empty arrays to send back to user
            set_server_icon.runSetServerIcon(
                    ui->selectOperationInsertRadioButton->isChecked(),
                    ui->iconIndexSpinBox->value(),
                    ui->iconActiveCheckBox->isChecked(),
                    QByteArray(),
                    [&]() {
                        ui->setFieldsSubmitPushButton->setEnabled(true);
                    }
            );
        }

    } else {
        QMessageBox::warning(this,
                             "Error",
                             "None of the radio buttons inside layout selectTypeVerticalWidget were selected."
        );
        return;
    }
}

void SetFieldsWindow::on_colorCodeLineEdit_textChanged(const QString& new_text) {
    if (colorCodeIsValid(new_text.toStdString())) {
        const QString style_sheet = QString("QLabel { background-color : %1;}").arg(new_text);
        ui->colorCodeShowColorLabel->setStyleSheet(style_sheet);
    } else {
        ui->colorCodeShowColorLabel->setStyleSheet("QLabel { background-color #000000;}");
    }
}

void SetFieldsWindow::selectIconPressed(
        QLabel* image_label,
        QByteArray& bytes
) {

    const RequestImageFromFileReturnValues request_image_return = requestImageFromFile(
            this,
            std::vector<std::string>{"png"}
    );

    if (request_image_return.canceled) {
        return;
    }

    const QImage& raw_image = request_image_return.image;

    if (raw_image.isNull()) {
        const QString error_string = QString("Error reading image from file.\nReturned error: '")
                .append(request_image_return.error_string)
                .append("'");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (raw_image.width() < (int) globals->activity_icon_width_in_pixels()
        || raw_image.height() < (int) globals->activity_icon_height_in_pixels()
            ) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Image selected must have height larger than %1 and width larger than %2.\nSelected image dimensions are %3x%4.")
                                     .arg(globals->activity_icon_height_in_pixels())
                                     .arg(globals->activity_icon_width_in_pixels())
                                     .arg(raw_image.height())
                                     .arg(raw_image.width())
        );
        return;
    } else if ((int) (globals->activity_icon_width_in_pixels() / globals->activity_icon_height_in_pixels()) !=
               raw_image.width() / raw_image.height()
            ) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Image selected must be same ratio as %1x%2.\nSelected image dimensions are %3x%4.")
                                     .arg(globals->activity_icon_height_in_pixels())
                                     .arg(globals->activity_icon_width_in_pixels())
                                     .arg(raw_image.height())
                                     .arg(raw_image.width())
        );
        return;
    }

    QImage scaled_image = raw_image.scaled(
            (int) globals->activity_icon_width_in_pixels(),
            (int) globals->activity_icon_height_in_pixels(),
            Qt::AspectRatioMode::KeepAspectRatio
    );

    image_label->setPixmap(QPixmap::fromImage(scaled_image));

    bytes.clear();

    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);

    //NOTE: do NOT set a quality rating for .save(), leave it at -1. It will increase the
    // size of the file needlessly.
    scaled_image.save(&buffer, "PNG");
}

void SetFieldsWindow::on_basicIconPushButton_clicked() {
    selectIconPressed(
            ui->basicIconDisplayIconLabel,
            basic_image_bytes
    );
}

void SetFieldsWindow::on_iconActiveCheckBox_toggled(bool checked) {
    ui->basicIconTitleLabel->setVisible(checked);
    ui->basicIconHorizontalWidget->setVisible(checked);
}

SetFieldsWindow::~SetFieldsWindow() {
    delete ui;
}




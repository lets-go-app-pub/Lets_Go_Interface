//
// Created by jeremiah on 3/23/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_add_event_window.h" resolved

#include <QMessageBox>
#include <QDir>
#include <QBuffer>
#include <QImageReader>
#include "add_event_window.h"
#include "ui_add_event_window.h"
#include "homewindow.h"
#include "user_login_info.h"
#include "plain_text_edit_with_max_chars.h"
#include "general_utility.h"
#include "display_user_picture.h"

AddEventWindow::AddEventWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::AddEventWindow) {
    ui->setupUi(this);

    ui->qrCodeMessageVerticalFrame->setVisible(false);

    setupDefaultValuesAndRestrictions();

    setupSignalsAndSlots();

}

AddEventWindow::~AddEventWindow() {
    delete ui;
}

void AddEventWindow::setupSignalsAndSlots() {
    connect(&add_admin_event_object, &AddAdminEventObject::signal_sendWarning, this,
            &AddEventWindow::slot_displayWarning);
    connect(&add_admin_event_object, &AddAdminEventObject::signal_requestCanceled, this,
            &AddEventWindow::slot_requestCanceled);
    connect(&add_admin_event_object, &AddAdminEventObject::signal_addAdminEventSuccessfullyCompleted, this,
            &AddEventWindow::slot_addAdminEventSuccessfullyCompleted);
}

void AddEventWindow::setupDefaultValuesAndRestrictions() {

    ui->addEventWindowScrollArea->setWidgetResizable(true);

    //Some values/restrictions such as location are set directly in the ui file.
    ui->titleLineEdit->setMaxLength(globals->maximum_number_allowed_bytes_event_title());

    auto description_text_edit = new PlainTextEditWithMaxChars();
    description_text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_user_bio());
    description_text_edit->setObjectName(BIO_PLAIN_TEXT_EDIT_NAME);
    description_text_edit->setPlaceholderText(BIO_PLAIN_TEXT_EDIT_PLACEHOLDER);
    description_text_edit->setReadOnly(false);
    ui->descritionInputVerticalFrame->layout()->addWidget(description_text_edit);

    ui->cityLineEdit->setMaxLength((int) globals->maximum_number_allowed_bytes());

    ui->minimumAllowedAgeSpinBox->setMinimum((int) globals->lowest_allowed_age());
    ui->minimumAllowedAgeSpinBox->setMaximum((int) globals->highest_allowed_age());
    ui->minimumAllowedAgeSpinBox->setValue((int) globals->lowest_allowed_age());

    ui->femaleGenderCheckbox->setChecked(true);
    ui->maleGenderCheckbox->setChecked(true);

    ui->activityIndexSpinBox->setMinimum(1);
    //No guarantee activities have all been loaded here. Also new ones could be added, so cannot use the currently
    // stored number of activities as the maximum.
    ui->activityIndexSpinBox->setMaximum(999);

    QDateTime current_date_time = QDateTime::currentDateTime();

    ui->startTimeDateTimeEdit->setMinimumDateTime(current_date_time);
    ui->startTimeDateTimeEdit->setDateTime(current_date_time);
    ui->stopTimeDateTimeEdit->setMinimumDateTime(current_date_time);
    ui->stopTimeDateTimeEdit->setDateTime(current_date_time);

    ui->qrCodeMessageLineEdit->setMaxLength(globals->maximum_number_allowed_bytes_user_qr_code_message());
}

void AddEventWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void AddEventWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

void AddEventWindow::slot_displayWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& lambda
) {

    if (lambda) {
        lambda();
    }

    std::cout << "title: " << title.toStdString() << '\n';
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(
            this,
            title,
            text
    );
}

void AddEventWindow::slot_addAdminEventSuccessfullyCompleted(
        const std::string& event_oid,
        const std::function<void()>& lambda
) {

    if (lambda) {
        lambda();
    }

    ui->titleLineEdit->setText("");
    ui
            ->descritionInputVerticalFrame
            ->findChild<QPlainTextEdit*>(BIO_PLAIN_TEXT_EDIT_NAME)
            ->setPlainText("");
    ui->cityLineEdit->setText("");
    ui->latitudeDoubleSpinBox->setValue(0.0);
    ui->longitudeDoubleSpinBox->setValue(0.0);
    ui->minimumAllowedAgeSpinBox->setValue(globals->lowest_allowed_age());
    ui->maleGenderCheckbox->setChecked(true);
    ui->femaleGenderCheckbox->setChecked(true);
    ui->activityIndexSpinBox->setValue(1);

    QDateTime current_date_time = QDateTime::currentDateTime();
    ui->startTimeDateTimeEdit->setDateTime(current_date_time);
    ui->stopTimeDateTimeEdit->setDateTime(current_date_time);

    for(auto& pic : saved_pictures) {
        slot_removePicture(pic.picture_frame, pic.picture_id);
    }

    slot_removeQrCode(nullptr, "");

    ui->qrCodeMessageLineEdit->setText("");

    QString title = "Success";
    QString text = QString("Successfully added event. New event id is <").append(event_oid.c_str()).append(">.");

    QMessageBox::information(
            this,
            title,
            text
    );
}

std::pair<int, int> calculateHeightAndWidthForBitmap(
        const int max_size_px,
        const int raw_bitmap_height_px,
        const int raw_bitmap_width_px
) {
    //Crop edges if landscape type picture.
    const int cropped_width = std::min(raw_bitmap_height_px, raw_bitmap_width_px);

    //Scale picture to max size.
    const int height = std::min(max_size_px, raw_bitmap_height_px);
    const int width = (int) ((double) height * (double) cropped_width / (double) raw_bitmap_height_px);

    return {height, width};
}

void AddEventWindow::on_addPicturesPushButton_clicked() {

    if ((int) saved_pictures.size() >= globals->number_pictures_stored_per_account()) {
        const QString error_string = "Maximum number of pictures reached. "
                                     "Please delete one in order to add more.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    const RequestImageFromFileReturnValues request_image_return = requestImageFromFile(
            this,
            std::vector<std::string>{"png", "jpg", "jpeg"}
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

    const auto [thumbnail_height, thumbnail_width] = calculateHeightAndWidthForBitmap(
            globals->picture_thumbnail_maximum_cropped_size_px(),
            raw_image.height(),
            raw_image.width()
    );

    const auto [picture_height, picture_width] = calculateHeightAndWidthForBitmap(
            globals->picture_maximum_cropped_size_px(),
            raw_image.height(),
            raw_image.width()
    );

    const QImage picture_image = raw_image.scaled(
            picture_width,
            picture_height,
            Qt::AspectRatioMode::KeepAspectRatio
    );

    const QImage thumbnail_image = raw_image.scaled(
            thumbnail_width,
            thumbnail_height,
            Qt::AspectRatioMode::KeepAspectRatio
    );

    saved_pictures.emplace_back(QUuid::createUuid().toString().toStdString());

    auto& picture = saved_pictures.back();

    picture.picture_bytes.clear();
    picture.thumbnail_bytes.clear();

    QBuffer picture_buffer(&picture.picture_bytes);
    QBuffer thumbnail_buffer(&picture.thumbnail_bytes);
    picture_buffer.open(QIODevice::WriteOnly);
    thumbnail_buffer.open(QIODevice::WriteOnly);

    if (request_image_return.image_format == "PNG") {
        //NOTE: Do NOT set a quality rating for .save(), leave it at -1. It will increase the
        // size of the file needlessly.
        picture_image.save(&picture_buffer, request_image_return.image_format.c_str());
        thumbnail_image.save(&thumbnail_buffer, request_image_return.image_format.c_str());
    } else {
        picture_image.save(&picture_buffer, request_image_return.image_format.c_str(), globals->image_quality_value());
        thumbnail_image.save(&thumbnail_buffer, request_image_return.image_format.c_str(),
                             100);//, globals->image_quality_value());
    }

    picture.picture_frame = new DisplayUserPicture(
            "",
            picture.picture_id,
            false
    );

    picture.picture_frame->showPicture(
            picture.picture_bytes
    );

    connect(picture.picture_frame, &DisplayUserPicture::signal_removeThisIndex, this,
            &AddEventWindow::slot_removePicture);

    ui->picturesHorizontalLayout->addWidget(picture.picture_frame, 0, Qt::AlignHCenter);
}

void AddEventWindow::on_addQrCodePushButton_clicked() {

    const RequestImageFromFileReturnValues request_image_return = requestImageFromFile(
            this,
            std::vector<std::string>{"png"}
    );

    if (request_image_return.canceled) {
        return;
    }

    const QImage& qr_code_image = request_image_return.image;

    if (qr_code_image.isNull()) {
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

    saved_qr_code.clear();

    QBuffer qr_code_buffer(&saved_qr_code);
    qr_code_buffer.open(QIODevice::WriteOnly);

    qr_code_image.save(&qr_code_buffer, request_image_return.image_format.c_str());

    if (qr_code_picture_frame != nullptr) {
        ui->qrCodeHorizontalLayout->removeWidget(qr_code_picture_frame);
        delete qr_code_picture_frame;
    }

    qr_code_picture_frame = new DisplayUserPicture(
            "",
            "",
            false
    );

    qr_code_picture_frame->showPicture(
            saved_qr_code
    );

    connect(qr_code_picture_frame, &DisplayUserPicture::signal_removeThisIndex, this,
            &AddEventWindow::slot_removeQrCode);

    ui->qrCodeHorizontalLayout->addWidget(qr_code_picture_frame, 0, Qt::AlignHCenter);
    ui->qrCodeMessageVerticalFrame->setVisible(true);
}

void AddEventWindow::slot_removeQrCode(QWidget* w [[maybe_unused]], const std::string& picture_oid [[maybe_unused]]) {
    if (qr_code_picture_frame != nullptr) {
        ui->qrCodeHorizontalLayout->removeWidget(qr_code_picture_frame);
        delete qr_code_picture_frame;
        qr_code_picture_frame = nullptr;
    }
    saved_qr_code.clear();
    ui->qrCodeMessageVerticalFrame->setVisible(false);
}

void AddEventWindow::slot_removePicture(QWidget* w, const std::string& picture_oid) {
    for (auto it = saved_pictures.begin(); it != saved_pictures.end(); it++) {
        if (it->picture_id == picture_oid) {
            saved_pictures.erase(it);
            break;
        }
    }
    ui->picturesHorizontalLayout->removeWidget(w);
    delete w;
}

void AddEventWindow::on_submitPushButton_clicked() {

    std::unique_ptr<EventRequestMessage> event_request = std::make_unique<EventRequestMessage>();

    event_request->set_event_title(ui->titleLineEdit->text().toStdString());

    if (event_request->event_title().empty()
        || (int) event_request->event_title().size() > globals->maximum_number_allowed_bytes_event_title()) {
        const QString error_string = QString("Event title must be between 1 and ")
                .append(QString::number(globals->maximum_number_allowed_bytes_event_title()))
                .append(" characters.");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    event_request->set_bio(
            ui
                    ->descritionInputVerticalFrame
                    ->findChild<QPlainTextEdit*>(BIO_PLAIN_TEXT_EDIT_NAME)
                    ->toPlainText()
                    .toStdString()
    );

    if (event_request->bio().empty() ||
        event_request->bio().size() > globals->maximum_number_allowed_bytes_user_bio()) {
        const QString error_string = QString("Description must be between 1 and ")
                .append(QString::number(globals->maximum_number_allowed_bytes_user_bio()))
                .append(" characters.");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    event_request->set_city(ui->cityLineEdit->text().toStdString());

    if (event_request->city().empty() ||
        event_request->city().size() > globals->maximum_number_allowed_bytes()) {
        const QString error_string = QString("City must be between 1 and ")
                .append(QString::number(globals->maximum_number_allowed_bytes()))
                .append(" characters.");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    event_request->set_location_longitude(
            ui->longitudeDoubleSpinBox->value()
    );

    event_request->set_location_latitude(
            ui->latitudeDoubleSpinBox->value()
    );

    if (!isValidLocation(
            event_request->location_longitude(),
            event_request->location_latitude())
            ) {
        const QString error_string = "Invalid location passed, events must have a location.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    event_request->set_min_allowed_age(ui->minimumAllowedAgeSpinBox->value());
    event_request->set_max_allowed_age((int) globals->highest_allowed_age());

    if (event_request->min_allowed_age() < (int) globals->lowest_allowed_age()
        || event_request->min_allowed_age() > (int) globals->highest_allowed_age()) {
        const QString error_string = QString("Minimum allowed age must be between ")
                .append(QString::number((int) globals->lowest_allowed_age()))
                .append(" and ")
                .append(QString::number((int) globals->highest_allowed_age()))
                .append(".");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (ui->maleGenderCheckbox->isChecked()
        && ui->femaleGenderCheckbox->isChecked()) {
        event_request->add_allowed_genders(globals->everyone_gender_string());
    } else if (ui->maleGenderCheckbox->isChecked()) {
        event_request->add_allowed_genders(globals->male_gender_string());
    } else if (ui->femaleGenderCheckbox->isChecked()) {
        event_request->add_allowed_genders(globals->female_gender_string());
    } else {
        const QString error_string = "Please select at least one gender.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    CategoryActivityMessage* activity_message = event_request->mutable_activity();
    activity_message->set_activity_index(ui->activityIndexSpinBox->value());

    if (event_request->activity().activity_index() < 1) {
        const QString error_string = "Activity cannot be set to 0.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    CategoryTimeFrameMessage* time_frame = activity_message->add_time_frame_array();
    time_frame->set_start_time_frame(
            ui->startTimeDateTimeEdit->dateTime().toMSecsSinceEpoch()
    );

    time_frame->set_stop_time_frame(
            ui->stopTimeDateTimeEdit->dateTime().toMSecsSinceEpoch()
    );

    if (
            event_request->activity().time_frame_array(0).start_time_frame() >
            event_request->activity().time_frame_array(0).stop_time_frame()
            ) {
        const QString error_string =
                QString("Event stop time <")
                        .append(ui->stopTimeDateTimeEdit->dateTime().toString())
                        .append("> must come after event start time <")
                        .append(ui->startTimeDateTimeEdit->dateTime().toString())
                        .append(">.");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    const std::chrono::milliseconds current_timestamp = getCurrentTimestamp();

    if (event_request->activity().time_frame_array(0).start_time_frame() <= current_timestamp.count()) {
        const QString error_string =
                QString("Event start time must come after current time.\nstart time: ")
                        .append(ui->startTimeDateTimeEdit->dateTime().toString());

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (event_request->activity().time_frame_array(0).stop_time_frame() <= current_timestamp.count()) {
        const QString error_string =
                QString("Event stop time must come after current time.\nstop time: ")
                        .append(ui->stopTimeDateTimeEdit->dateTime().toString());

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (activities.size() <= event_request->activity().activity_index()) {
        const QString error_string =
                "Selected activity index does not exist inside activities."
                " Please double check activity index or restart program if problem persists.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (event_request->min_allowed_age() < activities[event_request->activity().activity_index()].min_age()) {
        const QString error_string =
                "Selected activity index has a minimum age higher than the selected 'Minimum Allowed Age'.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    const int category_index = activities[event_request->activity().activity_index()].category_index();

    if (categories.size() <= category_index) {
        const QString error_string =
                "Selected activity index does not seem to have a valid category."
                " Please double check activity index or restart program if problem persists.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (category_index < 1) {
        const QString error_string = "Cannot modify an activity contained inside the 'Unknown' category.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (event_request->min_allowed_age() < categories[category_index].min_age()) {
        const QString error_string =
                "The category containing selected activity index has a minimum age higher than the selected 'Minimum Allowed Age'.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if (!saved_qr_code.isEmpty()) {
        if (saved_qr_code.size() > globals->maximum_qr_code_size_in_bytes()) {
            const QString error_string = "QR Code image is too large. Please select a different image.";

            QMessageBox::warning(
                    this,
                    "Error",
                    error_string
            );
            return;
        }

        event_request->set_qr_code_file_in_bytes(std::string(saved_qr_code.constData(), saved_qr_code.size()));
        event_request->set_qr_code_file_size((int) saved_qr_code.size());
        event_request->set_qr_code_message(ui->qrCodeMessageLineEdit->text().toStdString());

        if (!event_request->qr_code_message().empty()
            && (int) event_request->qr_code_message().size() >
               globals->maximum_number_allowed_bytes_user_qr_code_message()) {
            const QString error_string = "QR Code message is too long. Please use less characters.";

            QMessageBox::warning(
                    this,
                    "Error",
                    error_string
            );
            return;
        }
    }

    if (saved_pictures.empty()) {
        const QString error_string = "Please select at least one picture.";

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    if ((int) saved_pictures.size() > globals->number_pictures_stored_per_account()) {
        const QString error_string = QString("Too many pictures selected. Maximum is ")
                .append(QString::number(globals->number_pictures_stored_per_account()))
                .append(".");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    //Running the check first so that no unnecessary copies are made on the ui thread.
    for (size_t i = 0; i < saved_pictures.size(); ++i) {
        if ((int) saved_pictures[i].picture_bytes.size() > globals->maximum_picture_size_in_bytes()
            || (int) saved_pictures[i].thumbnail_bytes.size() > globals->maximum_picture_thumbnail_size_in_bytes()) {
            const QString error_string = QString("Picture number ")
                    .append(QString::number(i + 1))
                    .append(" is too large (in bytes), please choose a different picture.");

            QMessageBox::warning(
                    this,
                    "Error",
                    error_string
            );
            return;
        }
    }

    //Save these last because it has to make copies.
    for (const auto& pic: saved_pictures) {
        EventsPictureMessage* picture_message = event_request->add_pictures();

        picture_message->set_file_size((int) pic.picture_bytes.size());
        picture_message->set_file_in_bytes(std::string(pic.picture_bytes.constData(), pic.picture_bytes.size()));
        picture_message->set_thumbnail_size((int) pic.thumbnail_bytes.size());
        picture_message->set_thumbnail_in_bytes(
                std::string(pic.thumbnail_bytes.constData(), pic.thumbnail_bytes.size()));
    }

    if (saved_qr_code.isEmpty()) {
        const QString error_string = "No QR Code has been entered. Continue anyway?";

        const QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Warning",
                error_string,
                QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No) {
            return;
        }
    } else if (event_request->qr_code_message().empty()) {
        const QString error_string = "A QR code has been set. However, no QR code message has been entered. Continue anyway?";

        const QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Warning",
                error_string,
                QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No) {
            return;
        }
    }

    if (
            event_request->location_longitude() < -113.13223049756054
            || -111.19860753553903 < event_request->location_longitude()
            || event_request->location_latitude() < 32.553168519237275
            || 34.22211299925714 < event_request->location_latitude()
            ) {
        const QString error_string = "The location passed is not inside the greater phoenix area."
                                     " Are you sure you want to Continue?\n\n"
                                     "TIP: Make sure latitude and longitude are not swapped.";

        const QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Warning",
                error_string,
                QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No) {
            return;
        }
    }

    //Enabling all fields and not just the submit button because the fields are cleared when successful.
    setEnabledForMutableFields(false);
    add_admin_event_object.runAddAdminEvent(
            std::move(event_request),
            [this]() {
                setEnabledForMutableFields(true);
            }
    );
}

void AddEventWindow::setEnabledForMutableFields(bool enabled) {
    ui->titleLineEdit->setEnabled(enabled);
    ui
            ->descritionInputVerticalFrame
            ->findChild<QPlainTextEdit*>(BIO_PLAIN_TEXT_EDIT_NAME)
            ->setEnabled(enabled);
    ui->cityLineEdit->setEnabled(enabled);
    ui->latitudeDoubleSpinBox->setEnabled(enabled);
    ui->longitudeDoubleSpinBox->setEnabled(enabled);
    ui->minimumAllowedAgeSpinBox->setEnabled(enabled);
    ui->maleGenderCheckbox->setEnabled(enabled);
    ui->femaleGenderCheckbox->setEnabled(enabled);
    ui->activityIndexSpinBox->setEnabled(enabled);
    ui->startTimeDateTimeEdit->setEnabled(enabled);
    ui->stopTimeDateTimeEdit->setEnabled(enabled);
    ui->addPicturesPushButton->setEnabled(enabled);

    for(auto& pic : saved_pictures) {
        if(pic.picture_frame) {
            pic.picture_frame->setButtonToEnabled(enabled);
        }
    }

    ui->addQrCodePushButton->setEnabled(enabled);
    if(qr_code_picture_frame) {
        qr_code_picture_frame->setButtonToEnabled(enabled);
    }

    ui->qrCodeMessageLineEdit->setEnabled(enabled);
    ui->submitPushButton->setEnabled(enabled);
}
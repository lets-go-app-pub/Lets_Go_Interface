//
// Created by jeremiah on 9/21/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_display_chat_message.h" resolved

#include <QMessageBox>
#include "display_chat_message.h"

#include "general_utility.h"
#include "ui_display_chat_message.h"

DisplayChatMessage::DisplayChatMessage(ChatMessageToClient message, bool displayChatRoomId, bool displayUserOID, QWidget* parent) :
    QWidget(parent), ui(new Ui::DisplayChatMessage), message(std::move(message)), displayChatRoomId(displayChatRoomId), displayUserOID(displayUserOID) {
    ui->setupUi(this);

    setupUIForMessage();
}

void DisplayChatMessage::slot_gifHttpRequestResult(QNetworkReply* reply) {

    ui->primaryMessageLabel->clear();

    if (reply->error() != QNetworkReply::NoError) {

        QMessageBox::warning(this,
                             "Error",
                             QString("Returned Network error.\n%1")
                                     .arg(reply->errorString())
        );

        ui->primaryMessageLabel->setText("Network error occurred.");
        return;
    }

    image_bytes = std::make_unique<QByteArray>(reply->readAll());

    if (message.message().message_specifics().mime_type_message().mime_type() == "image/gif") { //gif
        image_data = std::make_unique<QBuffer>(image_bytes.get());
        image_gif = std::make_unique<QMovie>(image_data.get());

        ui->primaryMessageLabel->setMovie(image_gif.get());
        image_gif->start();
    } else { //image (probably png)

        QPixmap pixmap;
        pixmap.loadFromData(*image_bytes);

        ui->primaryMessageLabel->setPixmap(pixmap);
    }
}

void DisplayChatMessage::displayReply(const ReplyChatMessageInfo& reply_info) {

    ui->replyBodyFrame->setVisible(true);
    ui->replyNameLabel->setText(
            QString("Replied To Message User: %1").arg(QString::fromStdString(reply_info.reply_is_sent_from_user_oid())));
    ui->replyPictureLabel->setFixedSize(QSize(0, 0));

    switch (reply_info.reply_specifics().reply_body_case()) {
        case ReplySpecifics::kTextReply: {
            ui->replyTextLabel->setText(
                    QString::fromStdString(reply_info.reply_specifics().text_reply().message_text()));
            break;
        }
        case ReplySpecifics::kPictureReply: {

            const std::string& bytes = reply_info.reply_specifics().picture_reply().thumbnail_in_bytes();
            unsigned long byte_len = reply_info.reply_specifics().picture_reply().thumbnail_file_size();

            if (bytes.size() != byte_len) {
                ui->replyTextLabel->setText(
                        QString("ERROR: corrupt reply picture found. Expected: %1 Actual: %2")
                                .arg(bytes.size())
                                .arg(byte_len)
                );
            } else {
                ui->replyPictureLabel->setScaledContents(true);
                ui->replyPictureLabel->setFixedSize(QSize(200, 200));

                QPixmap pixmap;
                pixmap.loadFromData(reinterpret_cast<const uchar*>(bytes.c_str()), byte_len);

                ui->replyPictureLabel->setPixmap(pixmap);

                ui->replyTextLabel->setText(QString("Picture Message"));
            }
            break;
        }
        case ReplySpecifics::kLocationReply: {
            ui->replyTextLabel->setText(QString("Location Message"));
            break;
        }
        case ReplySpecifics::kMimeReply: {
            const std::string& bytes = reply_info.reply_specifics().mime_reply().thumbnail_in_bytes();
            unsigned long byte_len = reply_info.reply_specifics().mime_reply().thumbnail_file_size();

            if (bytes.size() != byte_len) {
                ui->replyTextLabel->setText(
                        QString("ERROR: corrupt reply picture found. Expected: %1 Actual: %2")
                                .arg(bytes.size())
                                .arg(byte_len)
                );
            } else {
                ui->replyPictureLabel->setScaledContents(true);
                ui->replyPictureLabel->setFixedSize(QSize(200, 200));

                QPixmap pixmap;
                pixmap.loadFromData(reinterpret_cast<const uchar*>(bytes.c_str()), byte_len);

                ui->replyPictureLabel->setPixmap(pixmap);

                ui->replyTextLabel->setText(
                        QString("Mime Type Message (%1)")
                                .arg(QString::fromStdString(
                                        reply_info.reply_specifics().mime_reply().thumbnail_mime_type())
                                )
                );
            }
            break;
        }
        case ReplySpecifics::kInviteReply: {
            ui->replyTextLabel->setText(QString("Invite Message"));
            break;
        }
        case ReplySpecifics::REPLY_BODY_NOT_SET:
            ui->replyTextLabel->setText(QString("Error invalid reply type received type %1.")
                                                .arg(reply_info.reply_specifics().reply_body_case()));
            break;
    }
}

void DisplayChatMessage::setupUIForMessage() {

    ui->userOIDHorizontalWidget->setVisible(true);
    ui->messageUUIDHorizontalWidget->setVisible(true);
    ui->chatRoomIdHorizontalWidget->setVisible(true);

    ui->replyBodyFrame->setVisible(false);

    ui->userOIDHorizontalWidget->setVisible(displayUserOID);
    ui->chatRoomIdHorizontalWidget->setVisible(displayChatRoomId);

    ui->userOIDLabel->setText(QString::fromStdString(message.sent_by_account_id()));
    ui->messageUUIDLabel->setText(QString::fromStdString(message.message_uuid()));
    ui->chatRoomIdLabel->setText(
            QString::fromStdString(message.message().standard_message_info().chat_room_id_message_sent_from()));

    ui->primaryMessageLabel->clear();

    switch (message.message().message_specifics().message_body_case()) {
        case MessageSpecifics::kTextMessage: {
            ui->primaryMessageLabel->setText(
                    QString::fromStdString(
                            message.message().message_specifics().text_message().message_text()
                    )
            );

            if(message.message().message_specifics().text_message().active_message_info().is_reply()) {
                displayReply(
                        message.message().message_specifics().text_message().active_message_info().reply_info()
                        );
            }

            ui->editedDeletedHorizontalWidget->setVisible(true);
            ui->deletedCheckBox->setVisible(true);
            ui->editedCheckBox->setVisible(true);

            ui->deletedCheckBox->setChecked(
                    message.message().message_specifics().text_message().active_message_info().is_deleted());
            ui->editedCheckBox->setChecked(message.message().message_specifics().text_message().is_edited());

            break;
        }
        case MessageSpecifics::kPictureMessage: {

            const std::string& pic_in_bytes = message.message().message_specifics().picture_message().picture_file_in_bytes();
            unsigned long pic_size_in_bytes = message.message().message_specifics().picture_message().picture_file_size();

            if (pic_in_bytes.size() != pic_size_in_bytes) {
                ui->primaryMessageLabel->setText(
                        QString("ERROR: corrupt picture found.\npic_in_bytes.size(): %1\npic_size_in_bytes: %2")
                                .arg(pic_in_bytes.size())
                                .arg(pic_size_in_bytes)
                );
            } else {

                QPixmap pixmap;
                pixmap.loadFromData(reinterpret_cast<const uchar*>(pic_in_bytes.c_str()), pic_size_in_bytes);

                QSize label_size = generatePictureSize(
                        pixmap.height(),
                        pixmap.width()
                );

                ui->primaryMessageLabel->setScaledContents(true);
                ui->primaryMessageLabel->setFixedSize(label_size);

                ui->primaryMessageLabel->setPixmap(pixmap);
            }

            if(message.message().message_specifics().picture_message().active_message_info().is_reply()) {
                displayReply(
                        message.message().message_specifics().picture_message().active_message_info().reply_info()
                        );
            }

            ui->editedDeletedHorizontalWidget->setVisible(true);
            ui->deletedCheckBox->setVisible(true);
            ui->editedCheckBox->setVisible(false);

            ui->deletedCheckBox->setChecked(
                    message.message().message_specifics().picture_message().active_message_info().is_deleted());

            break;
        }
        case MessageSpecifics::kLocationMessage: {

            ui->primaryMessageLabel->setText(
                    QString("Location message:\nLongitude: %1\nLatitude: %2")
                            .arg(message.message().message_specifics().location_message().longitude())
                            .arg(message.message().message_specifics().location_message().latitude())
            );

            if(message.message().message_specifics().location_message().active_message_info().is_reply()) {
                displayReply(
                        message.message().message_specifics().location_message().active_message_info().reply_info()
                        );
            }

            ui->editedDeletedHorizontalWidget->setVisible(true);
            ui->deletedCheckBox->setVisible(true);
            ui->editedCheckBox->setVisible(false);

            ui->deletedCheckBox->setChecked(
                    message.message().message_specifics().location_message().active_message_info().is_deleted());
            break;
        }
        case MessageSpecifics::kMimeTypeMessage: {

            if (network_access_manager == nullptr) {
                network_access_manager = new QNetworkAccessManager(this);

                connect(network_access_manager, &QNetworkAccessManager::finished,
                        this, &DisplayChatMessage::slot_gifHttpRequestResult);
            }

            //NOTE: size must be set up BEFORE the gif is loaded so that QTreeWidget can properly calculate the required
            // space to fill
            QSize label_size = generatePictureSize(
                    message.message().message_specifics().mime_type_message().image_height(),
                    message.message().message_specifics().mime_type_message().image_width()
                    );

            ui->primaryMessageLabel->setScaledContents(true);
            ui->primaryMessageLabel->setFixedSize(label_size);

            QUrl mime_type_url(QString::fromStdString(
                    message.message().message_specifics().mime_type_message().url_of_download()));
            QNetworkRequest request(mime_type_url);

            network_access_manager->get(request);

            ui->primaryMessageLabel->setText("Loading mime type...");

            if(message.message().message_specifics().mime_type_message().active_message_info().is_reply()) {
                displayReply(
                        message.message().message_specifics().mime_type_message().active_message_info().reply_info()
                        );
            }

            ui->editedDeletedHorizontalWidget->setVisible(true);
            ui->deletedCheckBox->setVisible(true);
            ui->editedCheckBox->setVisible(false);

            ui->deletedCheckBox->setChecked(
                    message.message().message_specifics().mime_type_message().active_message_info().is_deleted());
            break;
        }
        case MessageSpecifics::kInviteMessage: {

            ui->primaryMessageLabel->setText(
                    QString("User %1 invited user %2 (%3) to join a different chat room.\nChat room Id: %4\nChat room name: %5\nChat room password: %6")
                            .arg(
                                    QString::fromStdString(message.sent_by_account_id()),
                                    QString::fromStdString(
                                            message.message().message_specifics().invite_message().invited_user_account_oid()),
                                    QString::fromStdString(
                                            message.message().message_specifics().invite_message().invited_user_name()),
                                    QString::fromStdString(
                                            message.message().message_specifics().invite_message().chat_room_id()),
                                    QString::fromStdString(
                                            message.message().message_specifics().invite_message().chat_room_name()),
                                    QString::fromStdString(
                                            message.message().message_specifics().invite_message().chat_room_password())
                            )
            );

            if(message.message().message_specifics().invite_message().active_message_info().is_reply()) {
                displayReply(
                        message.message().message_specifics().invite_message().active_message_info().reply_info()
                        );
            }

            ui->editedDeletedHorizontalWidget->setVisible(true);
            ui->deletedCheckBox->setVisible(true);
            ui->editedCheckBox->setVisible(false);

            ui->deletedCheckBox->setChecked(
                    message.message().message_specifics().invite_message().active_message_info().is_deleted());
            break;
        }

        case MessageSpecifics::kUserKickedMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: User %1 kicked from chat")
                            .arg(QString::fromStdString(
                                    message.message().message_specifics().user_kicked_message().kicked_account_oid()
                            )));
            break;
        case MessageSpecifics::kUserBannedMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: User %1 banned from chat")
                            .arg(QString::fromStdString(
                                    message.message().message_specifics().user_banned_message().banned_account_oid()
                            )));
            break;
        case MessageSpecifics::kDifferentUserJoinedMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: User %1 joined chat")
                            .arg(QString::fromStdString(
                                    message.message().message_specifics().different_user_joined_message().member_info().user_info().account_oid()
                            )));
            break;
        case MessageSpecifics::kDifferentUserLeftMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: User %1 left chat")
                            .arg(QString::fromStdString(message.sent_by_account_id())));
            break;
        case MessageSpecifics::kChatRoomNameUpdatedMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: Chat room name updated to %1.")
                            .arg(QString::fromStdString(
                                    message.message().message_specifics().chat_room_name_updated_message().new_chat_room_name())));
            break;
        case MessageSpecifics::kChatRoomPasswordUpdatedMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: Chat room name password to %1.")
                            .arg(QString::fromStdString(
                                    message.message().message_specifics().chat_room_password_updated_message().new_chat_room_password())));
            break;
        case MessageSpecifics::kNewAdminPromotedMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(
                    QString("COMMAND: %1 was promoted to admin.")
                            .arg(QString::fromStdString(
                                    message.message().message_specifics().new_admin_promoted_message().promoted_account_oid())));
            break;
        case MessageSpecifics::kMatchCanceledMessage:
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setText(QString("COMMAND: Match was canceled."));
            break;

        case MessageSpecifics::kUpdateObservedTimeMessage:
        case MessageSpecifics::kThisUserJoinedChatRoomStartMessage:
        case MessageSpecifics::kThisUserJoinedChatRoomMemberMessage:
        case MessageSpecifics::kThisUserJoinedChatRoomFinishedMessage:
        case MessageSpecifics::kThisUserLeftChatRoomMessage:
        case MessageSpecifics::kUserActivityDetectedMessage:
        case MessageSpecifics::kHistoryClearedMessage:
        case MessageSpecifics::kNewUpdateTimeMessage:
        case MessageSpecifics::kLoadingMessage:
        case MessageSpecifics::kEditedMessage:
        case MessageSpecifics::kDeletedMessage:
        case MessageSpecifics::kChatRoomCapMessage:
        case MessageSpecifics::MESSAGE_BODY_NOT_SET:
            ui->userOIDHorizontalWidget->setVisible(false);
            ui->messageUUIDHorizontalWidget->setVisible(false);
            ui->chatRoomIdHorizontalWidget->setVisible(false);
            ui->editedDeletedHorizontalWidget->setVisible(false);
            ui->primaryMessageLabel->setVisible(false);
            break;
    }
}

bool DisplayChatMessage::willDisplaySomething(MessageSpecifics::MessageBodyCase message_type) {
    switch (message_type) {
        case MessageSpecifics::kTextMessage:
        case MessageSpecifics::kPictureMessage:
        case MessageSpecifics::kLocationMessage:
        case MessageSpecifics::kMimeTypeMessage:
        case MessageSpecifics::kInviteMessage:
        case MessageSpecifics::kUserKickedMessage:
        case MessageSpecifics::kUserBannedMessage:
        case MessageSpecifics::kDifferentUserJoinedMessage:
        case MessageSpecifics::kDifferentUserLeftMessage:
        case MessageSpecifics::kChatRoomNameUpdatedMessage:
        case MessageSpecifics::kChatRoomPasswordUpdatedMessage:
        case MessageSpecifics::kNewAdminPromotedMessage:
        case MessageSpecifics::kMatchCanceledMessage:
            return true;

        case MessageSpecifics::kUpdateObservedTimeMessage:
        case MessageSpecifics::kThisUserJoinedChatRoomStartMessage:
        case MessageSpecifics::kThisUserJoinedChatRoomMemberMessage:
        case MessageSpecifics::kThisUserJoinedChatRoomFinishedMessage:
        case MessageSpecifics::kThisUserLeftChatRoomMessage:
        case MessageSpecifics::kUserActivityDetectedMessage:
        case MessageSpecifics::kHistoryClearedMessage:
        case MessageSpecifics::kNewUpdateTimeMessage:
        case MessageSpecifics::kLoadingMessage:
        case MessageSpecifics::kEditedMessage:
        case MessageSpecifics::kDeletedMessage:
        case MessageSpecifics::kChatRoomCapMessage:
        case MessageSpecifics::MESSAGE_BODY_NOT_SET:
            return false;
    }

    return false;
}

DisplayChatMessage::~DisplayChatMessage() {
    delete ui;
}

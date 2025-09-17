//
// Created by jeremiah on 9/21/21.
//

#pragma once

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QWidget>
#include <QScreen>
#include <QRect>
#include <QUrl>
#include <QBuffer>
#include <QMovie>

#include <ChatMessageToClientMessage.grpc.pb.h>

QT_BEGIN_NAMESPACE
namespace Ui { class DisplayChatMessage; }
QT_END_NAMESPACE

class DisplayChatMessage : public QWidget {
Q_OBJECT

public:
    DisplayChatMessage(ChatMessageToClient message, bool displayChatRoomId, bool displayUserOID, QWidget* parent = nullptr);

    //returns true if something will be displayed for the passed message type
    static bool willDisplaySomething(MessageSpecifics::MessageBodyCase message_type);

    ~DisplayChatMessage() override;

private slots:
    void slot_gifHttpRequestResult(QNetworkReply *reply);

private:
    Ui::DisplayChatMessage* ui;
    ChatMessageToClient message;
    bool displayChatRoomId = false;
    bool displayUserOID = false;

    QNetworkAccessManager* network_access_manager = nullptr;

    void setupUIForMessage();

    void displayReply(const ReplyChatMessageInfo& reply_info);

    std::unique_ptr<QByteArray> image_bytes = nullptr;
    std::unique_ptr<QBuffer> image_data = nullptr;
    std::unique_ptr<QMovie> image_gif = nullptr;
};


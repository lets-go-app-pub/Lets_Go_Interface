//
// Created by jeremiah on 10/26/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_shutdown_servers.h" resolved

#include <user_login_info.h>
#include <QMessageBox>
#include <general_utility.h>

#include "shutdown_servers.h"
#include "ui_shutdown_servers.h"
#include "homewindow.h"
#include "../../../custom_widgets/display_user_widget_window/files/widgets_generated_names/widgets_generated_names.h"

ShutdownServers::ShutdownServers(QWidget* parent) :
        QWidget(parent), ui(new Ui::ShutdownServers) {
    ui->setupUi(this);

    ui->serversTreeWidget->header()->setStretchLastSection(false);
    ui->serversTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void ShutdownServers::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

ShutdownServers::~ShutdownServers() {
    delete ui;
}

void
ShutdownServers::setupChannelStatus(const std::shared_ptr<grpc::Channel>& passed_channel, QTreeWidgetItem* item) const {

    //channel status
    switch (passed_channel->GetState(false)) {
        case GRPC_CHANNEL_IDLE:
            item->setText(CHANNEL_STATUS_COLUMN_NUMBER, "IDLE");
            break;
        case GRPC_CHANNEL_CONNECTING:
            item->setText(CHANNEL_STATUS_COLUMN_NUMBER, "CONNECTING");
            break;
        case GRPC_CHANNEL_READY:
            item->setText(CHANNEL_STATUS_COLUMN_NUMBER, "READY");
            break;
        case GRPC_CHANNEL_TRANSIENT_FAILURE:
            item->setText(CHANNEL_STATUS_COLUMN_NUMBER, "TRANSIENT_FAILURE");
            break;
        case GRPC_CHANNEL_SHUTDOWN:
            item->setText(CHANNEL_STATUS_COLUMN_NUMBER, "SHUTDOWN");
            break;
    }

}

void ShutdownServers::setupUnaryCallStatus(grpc::StatusCode status,
                                           QTreeWidgetItem* item) const {

    //channel status
    switch (status) {
        case grpc::OK:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "OK");
            break;
        case grpc::CANCELLED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "CANCELLED");
            break;
        case grpc::UNKNOWN:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "UNKNOWN");
            break;
        case grpc::INVALID_ARGUMENT:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "INVALID_ARGUMENT");
            break;
        case grpc::DEADLINE_EXCEEDED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "DEADLINE_EXCEEDED");
            break;
        case grpc::NOT_FOUND:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "NOT_FOUND");
            break;
        case grpc::ALREADY_EXISTS:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "ALREADY_EXISTS");
            break;
        case grpc::PERMISSION_DENIED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "PERMISSION_DENIED");
            break;
        case grpc::UNAUTHENTICATED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "UNAUTHENTICATED");
            break;
        case grpc::RESOURCE_EXHAUSTED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "RESOURCE_EXHAUSTED");
            break;
        case grpc::FAILED_PRECONDITION:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "FAILED_PRECONDITION");
            break;
        case grpc::ABORTED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "ABORTED");
            break;
        case grpc::OUT_OF_RANGE:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "OUT_OF_RANGE");
            break;
        case grpc::UNIMPLEMENTED:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "UNIMPLEMENTED");
            break;
        case grpc::INTERNAL:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "INTERNAL");
            break;
        case grpc::UNAVAILABLE:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "UNAVAILABLE");
            break;
        case grpc::DATA_LOSS:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "DATA_LOSS");
            break;
        case grpc::DO_NOT_USE:
            item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "DO_NOT_USE");
            break;
    }

}

void ShutdownServers::refreshServerList() {

    for (auto& server_channel : server_channels) {
        server_channel->cancelThreads();
    }

    server_channels.clear();
    ui->serversTreeWidget->clear();
    int index = 0;

    for (const auto& address_info : globals->server_address_info()) {

        auto* item = new QTreeWidgetItem(ui->serversTreeWidget);

        std::string address = convertAndroidToLocalIfRequired(address_info.address());

        //NOTE: Make sure to include square brackets [] if necessary around the address, ipv6 format is
        // [::]:<port>.
        if(std::string::npos != address.find(':')) {
            address.insert(0, "[");
            address.push_back(']');
        }

        std::cout << "address: " << convertAndroidToLocalIfRequired(address_info.address()) << '\n';
        std::cout << "port: " << address_info.port() << '\n';

        server_channels.emplace_back(
                std::make_unique<RequestLoadWithChannel>(
                        ui->serversTreeWidget->indexFromItem(item),
                        address,
                        address_info.port()
                )
        );

        connect(
                server_channels.back()->retrieve_server_load.get(),
                &RetrieveServerLoadObject::signal_requestServerLoadFinished,
                this,
                &ShutdownServers::slot_requestServerLoadFinished
        );

        connect(
                server_channels.back()->request_server_shut_down.get(),
                &RequestServerShutDownObject::signal_requestServerShutdownCompleted,
                this,
                &ShutdownServers::slot_requestServerShutdownCompleted
        );

        connect(
                server_channels.back()->request_server_shut_down.get(),
                &RequestServerShutDownObject::signal_sendWarning,
                this,
                &ShutdownServers::slot_sendWarning
        );

        connect(
                server_channels.back()->request_server_shut_down.get(),
                &RequestServerShutDownObject::signal_requestCanceled,
                this,
                &ShutdownServers::slot_requestCanceled
        );

        server_channels.back()->retrieve_server_load->runRetrieveServerLoad(
                index,
                server_channels.back().get(),
                server_channels.back()->channel
        );

        item->setData(0, Qt::UserRole, index);
        item->setData(0, Qt::UserRole + 1,
                      QVariant::fromValue(reinterpret_cast<std::uintptr_t>(server_channels.back().get())));

        //index
        item->setText(0, QString::number(index));

        //address
        item->setText(1, QString::fromStdString(address_info.address()));

        //port
        item->setText(2, QString::number(address_info.port()));

        //channel status
        setupChannelStatus(server_channels.back()->channel, item);

        //unary call status
        item->setText(UNARY_CALL_STATUS_COLUMN_NUMBER, "Connecting...");

        //accepting connections
        item->setText(ACCEPTING_CONNECTIONS_COLUMN_NUMBER, "Connecting...");

        //number connected clients
        item->setText(NUMBER_CONNECTED_CLIENTS_COLUMN_NUMBER, "-1");

        index++;
    }

    ui->serversTreeWidget->viewport()->update();

}

void ShutdownServers::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    refreshServerList();
}

void ShutdownServers::slot_requestServerLoadFinished(int channel_vector_index,
                                                     void* item_id,
                                                     bool status_ok,
                                                     grpc::StatusCode error_code,
                                                     bool accepting_connections,
                                                     int number_connected_clients
) {

    if ((int)server_channels.size() > channel_vector_index
        && server_channels[channel_vector_index].get() == item_id
            ) {

        auto* item = ui->serversTreeWidget->itemFromIndex(server_channels[channel_vector_index]->item_index_in_tree);

        if (item != nullptr) {

            server_channels[channel_vector_index]->server_state = (status_ok
                                                                   && error_code == grpc::StatusCode::OK &&
                                                                   accepting_connections) ?
                    server_channels[channel_vector_index]->server_state = RequestLoadWithChannel::GrpcNaiveServerState::GRPC_NAIVE_SERVER_STATE_ACCEPTING
                                                                                          :
                    server_channels[channel_vector_index]->server_state = RequestLoadWithChannel::GrpcNaiveServerState::GRPC_NAIVE_SERVER_STATE_NOT_ACCEPTING;

            //channel status
            setupChannelStatus(server_channels[channel_vector_index]->channel, item);

            //unary call status
            setupUnaryCallStatus(error_code, item);

            //accepting connections
            item->setText(ACCEPTING_CONNECTIONS_COLUMN_NUMBER, accepting_connections ? "true" : "false");

            //number connected clients
            item->setText(NUMBER_CONNECTED_CLIENTS_COLUMN_NUMBER, QString::number(number_connected_clients));
        }
    }
}

void ShutdownServers::on_shutDownButton_clicked() {
    auto selected_items = ui->serversTreeWidget->selectedItems();

    //single selection is set on the Widget and so this should only have 1 item
    for (const auto& selected_item : selected_items) {
        int index = selected_item->data(0, Qt::UserRole).toInt();
        void* ptr = reinterpret_cast<void*>(selected_item->data(0, Qt::UserRole + 1).toULongLong());

        if ((int)server_channels.size() > index
            && server_channels[index].get() == ptr) {

            switch (server_channels[index]->server_state) {
                case RequestLoadWithChannel::GRPC_NAIVE_SERVER_STATE_LOADING:
                    ui->messageLabel->setText("Server is currently still LOADING info.");
                    break;
                case RequestLoadWithChannel::GRPC_NAIVE_SERVER_STATE_ACCEPTING:
                    server_channels[index]->request_server_shut_down->runRequestServerShutDown(
                            server_channels[index]->channel,
                            convertLocalToAndroidIfRequired(server_channels[index]->address),
                            index,
                            ptr
                    );
                    break;
                case RequestLoadWithChannel::GRPC_NAIVE_SERVER_STATE_NOT_ACCEPTING:
                    ui->messageLabel->setText("Server is currently not accepting connections.");
                    break;
            }

        }
    }
}

void ShutdownServers::on_refreshButton_clicked() {
    refreshServerList();
}

void ShutdownServers::slot_requestServerShutdownCompleted(
        int channel_vector_index,
        void* item_id
) {

    if ((int)server_channels.size() > channel_vector_index
        && server_channels[channel_vector_index].get() == item_id
            ) {
        auto* item = ui->serversTreeWidget->itemFromIndex(server_channels[channel_vector_index]->item_index_in_tree);
        server_channels[channel_vector_index]->server_state = RequestLoadWithChannel::GrpcNaiveServerState::GRPC_NAIVE_SERVER_STATE_NOT_ACCEPTING;

        if(item != nullptr) {
            //channel status
            setupChannelStatus(server_channels[channel_vector_index]->channel, item);

            //unary call status (must be status OK to reach here)
            setupUnaryCallStatus(grpc::StatusCode::OK, item);

            //accepting connections
            item->setText(ACCEPTING_CONNECTIONS_COLUMN_NUMBER, "false");
        }
    }

}

void ShutdownServers::slot_sendWarning(
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

void ShutdownServers::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}



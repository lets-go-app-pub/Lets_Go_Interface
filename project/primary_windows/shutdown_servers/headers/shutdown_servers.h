//
// Created by jeremiah on 10/26/21.
//

#pragma once

#include <QWidget>
#include <grpcpp/create_channel.h>
#include <grpcpp/channel.h>
#include <retrieve_server_load.h>
#include <QTreeWidgetItem>
#include <request_server_shut_down.h>

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class ShutdownServers; }
QT_END_NAMESPACE

class ShutdownServers : public QWidget {
Q_OBJECT

public:
    explicit ShutdownServers(QWidget* parent = nullptr);

    ~ShutdownServers() override;

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

private slots:
    void slot_requestServerLoadFinished(
            int channel_vector_index,
            void* item_id,
            bool status_ok,
            grpc::StatusCode error_code,
            bool accepting_connections,
            int number_connected_clients
            );

    void on_shutDownButton_clicked();

    void on_refreshButton_clicked();

    void slot_requestServerShutdownCompleted(
            int channel_vector_index,
            void* item_id
            );

    void slot_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = [](){}
            );

    void slot_requestCanceled(const std::function<void()>& completed_lambda = [](){});

protected:
    void showEvent(QShowEvent* event) override;

    void closeEvent(QCloseEvent* event) override;

private:
    HomeWindow* home_window_handle = nullptr;
    Ui::ShutdownServers* ui;

    struct RequestLoadWithChannel {
        std::shared_ptr<grpc::Channel> channel = nullptr;
        QModelIndex item_index_in_tree;
        std::shared_ptr<RetrieveServerLoadObject> retrieve_server_load = std::make_shared<RetrieveServerLoadObject>();
        std::shared_ptr<RequestServerShutDownObject> request_server_shut_down = std::make_shared<RequestServerShutDownObject>();
        std::string address;
        int port = -1;

        enum GrpcNaiveServerState {
            GRPC_NAIVE_SERVER_STATE_LOADING,
            GRPC_NAIVE_SERVER_STATE_ACCEPTING,
            GRPC_NAIVE_SERVER_STATE_NOT_ACCEPTING
        };

        GrpcNaiveServerState server_state = GrpcNaiveServerState::GRPC_NAIVE_SERVER_STATE_LOADING;
        grpc::StatusCode error_code = grpc::StatusCode::CANCELLED;

        void cancelThreads() {
            retrieve_server_load->cancelRequest();
            request_server_shut_down->cancelRequest();
        }

        RequestLoadWithChannel() = delete;

        RequestLoadWithChannel(
                QModelIndex _item_index_in_tree,
                const std::string& address,
                int port
                ):
                item_index_in_tree(_item_index_in_tree),
                address(address),
                port(port)
        {
            channel =  setupGrpcChannel(address, port);
        }

        ~RequestLoadWithChannel() {
            cancelThreads();
        }
    };

    //NOTE: using a unique_ptr so the address inside the container stays constant meaning
    // it can be used as an identifier
    std::vector<std::unique_ptr<RequestLoadWithChannel>> server_channels;

    const int CHANNEL_STATUS_COLUMN_NUMBER = 3;
    const int UNARY_CALL_STATUS_COLUMN_NUMBER = 4;
    const int ACCEPTING_CONNECTIONS_COLUMN_NUMBER = 5;
    const int NUMBER_CONNECTED_CLIENTS_COLUMN_NUMBER = 6;

    void refreshServerList();

    void setupChannelStatus(const std::shared_ptr<grpc::Channel>& passed_channel, QTreeWidgetItem* item) const;

    void setupUnaryCallStatus(grpc::StatusCode status, QTreeWidgetItem* item) const;

};


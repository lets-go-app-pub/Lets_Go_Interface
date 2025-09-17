//
// Created by jeremiah on 8/25/21.
//

#pragma once

#include <grpcpp/grpcpp.h>

inline grpc_channel_args* channel_args = nullptr;
inline std::shared_ptr<grpc::Channel> channel = nullptr;

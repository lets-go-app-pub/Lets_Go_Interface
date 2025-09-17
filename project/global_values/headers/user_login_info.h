//
// Created by jeremiah on 8/26/21.
//

#pragma once

#include <RequestMessages.grpc.pb.h>
#include <RequestFields.grpc.pb.h>
#include <AdminLevelEnum.grpc.pb.h>
#include <RequestAdminInfo.grpc.pb.h>

const int DESKTOP_INTERFACE_VERSION_NUMBER = 3;

const std::string USER_NAME = "Testing";
const std::string USER_PASSWORD = "abc";

extern AdminLevelEnum user_admin_level;
extern AdminPrivileges user_admin_privileges;
extern google::protobuf::RepeatedPtrField<ServerActivityOrCategoryMessage> categories;
extern std::mutex categories_mutex;
extern google::protobuf::RepeatedPtrField<ServerActivityOrCategoryMessage> activities;
extern std::mutex activities_mutex;

extern std::vector<request_fields::ServerIconsResponse> icons;
extern std::mutex icons_mutex;

const std::chrono::seconds GRPC_DEADLINE = std::chrono::seconds{15};

//these are set to longer, processing could take a little while
const std::chrono::seconds GRPC_DEADLINE_STATISTICS = std::chrono::seconds{60};

const int MAX_NUMBER_DAYS_TO_SEARCH_FOR_STATISTICS = 365;

extern std::unique_ptr<const request_admin_info::GlobalValuesToPassMessage> globals;

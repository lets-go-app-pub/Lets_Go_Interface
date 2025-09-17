//
// Created by jeremiah on 8/26/21.
//

#include "user_login_info.h"

AdminLevelEnum user_admin_level = AdminLevelEnum::NO_ADMIN_ACCESS;
AdminPrivileges user_admin_privileges;

google::protobuf::RepeatedPtrField<ServerActivityOrCategoryMessage> categories;
std::mutex categories_mutex;
google::protobuf::RepeatedPtrField<ServerActivityOrCategoryMessage> activities;
std::mutex activities_mutex;

std::vector<request_fields::ServerIconsResponse> icons;
std::mutex icons_mutex;

std::unique_ptr<const request_admin_info::GlobalValuesToPassMessage> globals = nullptr;
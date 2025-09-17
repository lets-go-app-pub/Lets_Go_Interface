//
// Created by jeremiah on 8/30/21.
//

#pragma once

#include <LoginToServerBasicInfo.grpc.pb.h>

void setup_login_info(LoginToServerBasicInfo* login_info);

void setup_login_info(LoginToServerBasicInfo* login_info, const std::string& user_account_oid);
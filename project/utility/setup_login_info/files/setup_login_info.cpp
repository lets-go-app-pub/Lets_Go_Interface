//
// Created by jeremiah on 8/30/21.
//

#include "user_login_info.h"
#include "setup_login_info.h"

void setup_login_info(LoginToServerBasicInfo* login_info) {
    login_info->set_admin_info_used(true);
    login_info->set_admin_name(USER_NAME);
    login_info->set_admin_password(USER_PASSWORD);
    login_info->set_lets_go_version(DESKTOP_INTERFACE_VERSION_NUMBER);
}

void setup_login_info(
        LoginToServerBasicInfo* login_info,
        const std::string& user_account_oid
) {
    setup_login_info(login_info);
    login_info->set_current_account_id(user_account_oid);
}
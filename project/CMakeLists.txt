set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/home/jeremiah/Qt/6.1.2/gcc_64")
#set(CMAKE_PREFIX_PATH "/home/jeremiah/Qt/5.12.11/gcc_64")

find_package(
        Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Charts
        Network
        REQUIRED
)

find_package(Threads)

#
# Sources
#
set(SOURCES
        main.cpp

        grpc/grpc_channel/grpc_channel_h/grpc_channel.h
        grpc/grpc_channel/grpc_channel_files/grpc_channel.cpp

        primary_windows/home_window/files/homewindow.ui
        primary_windows/home_window/files/homewindow.cpp
        primary_windows/home_window/headers/homewindow.h

        primary_windows/statistics_window/files/statistics_window.cpp
        primary_windows/statistics_window/headers/statistics_window.h
        primary_windows/statistics_window/files/statistics_window.ui

        global_values/headers/user_login_info.h
        global_values/files/user_downloaded_login_info.cpp

        utility/setup_login_info/files/setup_login_info.cpp
        utility/setup_login_info/headers/setup_login_info.h

        primary_windows/feedback_window/files/feedback_window.cpp
        primary_windows/feedback_window/headers/feedback_window.h
        primary_windows/feedback_window/files/feedback_window.ui

        primary_windows/set_fields_window/files/set_fields_window.cpp
        primary_windows/set_fields_window/headers/set_fields_window.h
        primary_windows/set_fields_window/files/set_fields_window.ui

        Resources/resource.qrc
        utility/request_from_server/request_server_icons/headers/request_server_icons.h
        utility/request_from_server/request_server_icons/files/request_server_icons.cpp
        utility/request_from_server/request_server_activities_categories/files/request_server_activities_categories.cpp
        utility/request_from_server/request_server_activities_categories/headers/request_server_activities_categories.h
        utility/request_from_server/run_server_call_base_class/headers/run_server_call_base_class.h
        utility/request_from_server/run_server_call_base_class/files/run_server_call_base_class.cpp
        utility/request_from_server/set_server_activity/headers/set_server_activity.h
        utility/request_from_server/set_server_activity/files/set_server_activity.cpp
        utility/request_from_server/set_server_category/headers/set_server_category.h
        utility/request_from_server/set_server_category/files/set_server_category.cpp
        utility/request_from_server/set_server_icon/files/set_server_icon.cpp
        utility/request_from_server/set_server_icon/headers/set_server_icon.h
        custom_widgets/display_user_widget_window/files/display_user_window.cpp
        custom_widgets/display_user_widget_window/headers/display_user_window.h
        custom_widgets/display_user_widget_window/files/display_user_window.ui
        utility/general_utility/headers/general_utility.h
        primary_windows/find_user_window/files/find_user_window.cpp
        primary_windows/find_user_window/headers/find_user_window.h
        primary_windows/find_user_window/files/find_user_window.ui
        custom_widgets/display_user_picture_widget/files/display_user_picture.cpp
        custom_widgets/display_user_picture_widget/headers/display_user_picture.h
        custom_widgets/display_user_picture_widget/files/display_user_picture.ui
        custom_widgets/aspect_ratio_pixmap_label/headers/aspect_ratio_pixmap_label.h
        custom_widgets/aspect_ratio_pixmap_label/files/aspect_ratio_pixmap_label.cpp
        utility/general_utility/files/get_date_time_string_from_timestamp.cpp
        utility/general_utility/files/calculate_user_age.cpp
        utility/general_utility/files/initialize_tm_by_date.cpp
        custom_widgets/plain_text_edit_with_max_chars/files/plain_text_edit_with_max_chars.cpp
        custom_widgets/plain_text_edit_with_max_chars/headers/plain_text_edit_with_max_chars.h
        custom_widgets/access_status_invalid_message_dialog/files/access_status_invalid_message_dialog.cpp
        custom_widgets/access_status_invalid_message_dialog/headers/access_status_invalid_message_dialog.h
        custom_widgets/access_status_invalid_message_dialog/files/access_status_invalid_message_dialog.ui
        primary_windows/set_default_picture/files/set_default_picture_window.cpp
        primary_windows/set_default_picture/headers/set_default_picture_window.h
        primary_windows/set_default_picture/files/set_default_picture_window.ui
        global_values/headers/globals.h
        utility/general_utility/files/generate_picture_size.cpp
        utility/request_from_server/set_default_picture/headers/set_default_picture.h
        utility/request_from_server/set_default_picture/files/set_default_picture.cpp
        utility/request_from_server/remove_user_picture/files/remove_user_picture.cpp
        utility/request_from_server/remove_user_picture/headers/remove_user_picture.h
        custom_widgets/display_chat_message_widget/files/display_chat_message.cpp
        custom_widgets/display_chat_message_widget/headers/display_chat_message.h
        custom_widgets/display_chat_message_widget/files/display_chat_message.ui
        primary_windows/reports_window/files/reports_window.cpp
        primary_windows/reports_window/headers/reports_window.h
        primary_windows/reports_window/files/reports_window.ui
        utility/request_from_server/time_out_user/headers/time_out_user.h
        utility/request_from_server/time_out_user/files/time_out_user.cpp
        utility/request_from_server/dismiss_report/files/dismiss_report.cpp
        utility/request_from_server/dismiss_report/headers/dismiss_report.h
        utility/log/log.h
        custom_widgets/button_with_meta_data_widget/files/button_with_meta_data.cpp
        custom_widgets/button_with_meta_data_widget/headers/button_with_meta_data.h
        custom_widgets/button_with_meta_data_widget/files/button_with_meta_data.ui
        utility/general_utility/files/get_current_timestamp.cpp
        custom_widgets/display_user_widget_window/files/widgets_generated_names/widgets_generated_names.cpp
        custom_widgets/display_user_widget_window/files/widgets_generated_names/widgets_generated_names.h
        primary_windows/shutdown_servers/files/shutdown_servers.cpp
        primary_windows/shutdown_servers/headers/shutdown_servers.h
        primary_windows/shutdown_servers/files/shutdown_servers.ui
        utility/request_from_server/retrieve_server_load/files/retrieve_server_load.cpp
        utility/request_from_server/retrieve_server_load/headers/retrieve_server_load.h
        utility/request_from_server/request_server_shut_down/headers/request_server_shut_down.h
        utility/request_from_server/request_server_shut_down/files/request_server_shut_down.cpp
        primary_windows/errors_window/files/errors_window.cpp
        primary_windows/errors_window/headers/errors_window.h
        primary_windows/errors_window/files/errors_window.ui
        utility/request_from_server/search_errors/headers/search_errors.h
        utility/request_from_server/search_errors/files/search_errors.cpp
        utility/general_utility/files/extract_time_t_from_tm.cpp
        utility/request_from_server/extract_errors/headers/extract_errors.h
        utility/request_from_server/extract_errors/files/extract_errors.cpp
        utility/request_from_server/extract_errors/headers/extract_errors_with_indexing.h
        utility/general_utility/files/is_valid_oid.cpp
        utility/request_from_server/delete_single_error/files/delete_single_error.cpp
        utility/request_from_server/delete_single_error/headers/delete_single_error.h
        utility/request_from_server/set_error_to_handled/files/set_error_to_handled.cpp
        utility/request_from_server/set_error_to_handled/headers/set_error_to_handled.h
        utility/general_utility/files/trim_whitespace_functions.cpp
        utility/general_utility/files/convert_android_server_channel_to_local.cpp
        primary_windows/home_window/files/set_default_functions/set_default_categories.cpp
        primary_windows/home_window/files/set_default_functions/set_default_categories.h
        primary_windows/home_window/files/set_default_functions/set_default_icons.cpp
        primary_windows/home_window/files/set_default_functions/set_default_icons.h
        primary_windows/home_window/files/set_default_functions/set_default_activities.h
        primary_windows/home_window/files/set_default_functions/set_default_activities.cpp
        utility/general_utility/files/setup_grpc_channel.cpp
        primary_windows/add_event_window/files/add_event_window.cpp
        primary_windows/add_event_window/headers/add_event_window.h
        primary_windows/add_event_window/files/add_event_window.ui
        utility/request_from_server/add_admin_event/files/add_admin_event.cpp
        utility/request_from_server/add_admin_event/headers/add_admin_event.h
        utility/general_utility/files/is_valid_location.cpp
        utility/general_utility/files/request_image_from_file.cpp
        primary_windows/cancel_event_window/files/cancel_event_window.cpp
        primary_windows/cancel_event_window/headers/cancel_event_window.h
        primary_windows/cancel_event_window/files/cancel_event_window.ui
        utility/request_from_server/get_events/files/get_events.cpp
        utility/request_from_server/get_events/headers/get_events.h
        utility/request_from_server/cancel_event/files/cancel_event.cpp
        utility/request_from_server/cancel_event/headers/cancel_event.h)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(project ${SOURCES})

# TESTING_NOTE: turn off -g3, its some kind of debugging flag https://caiorss.github.io/C-Cpp-Notes/compiler-flags-options.html
if (MSVC)
    # It seems to be that MSVC only goes to O2, not to O3
    # https://learn.microsoft.com/en-us/cpp/build/reference/o-options-optimize-code?view=msvc-160
    target_compile_options(project PRIVATE /O2 "/Zc:__cplusplus")
else ()
#    target_compile_options(project PRIVATE -Wall -Wextra -Wpedantic -g3 -pthread -O0)
    target_compile_options(project PRIVATE -pthread -O3)
endif ()

include_directories(
        global_values/headers
        grpc/grpc_channel/grpc_channel_h

        primary_windows/home_window/headers
        primary_windows/statistics_window/headers
        primary_windows/feedback_window/headers
        primary_windows/set_fields_window/headers
        primary_windows/find_user_window/headers
        primary_windows/set_default_picture/headers
        primary_windows/reports_window/headers
        primary_windows/shutdown_servers/headers
        primary_windows/errors_window/headers
        primary_windows/add_event_window/headers
        primary_windows/cancel_event_window/headers

        custom_widgets/access_status_invalid_message_dialog/headers
        custom_widgets/display_user_widget_window/headers
        custom_widgets/display_user_picture_widget/headers
        custom_widgets/aspect_ratio_pixmap_label/headers
        custom_widgets/plain_text_edit_with_max_chars/headers
        custom_widgets/display_chat_message_widget/headers
        custom_widgets/button_with_meta_data_widget/headers

        utility/setup_login_info/headers
        utility/request_from_server/add_admin_event/headers
        utility/request_from_server/delete_single_error/headers
        utility/request_from_server/request_server_icons/headers
        utility/request_from_server/retrieve_server_load/headers
        utility/request_from_server/request_server_activities_categories/headers
        utility/request_from_server/request_server_shut_down/headers
        utility/request_from_server/run_server_call_base_class/headers
        utility/request_from_server/set_server_activity/headers
        utility/request_from_server/set_server_category/headers
        utility/request_from_server/set_server_icon/headers
        utility/request_from_server/set_default_picture/headers
        utility/request_from_server/set_error_to_handled/headers
        utility/request_from_server/remove_user_picture/headers
        utility/request_from_server/time_out_user/headers
        utility/request_from_server/dismiss_report/headers
        utility/request_from_server/search_errors/headers
        utility/request_from_server/extract_errors/headers
        utility/request_from_server/get_events/headers
        utility/request_from_server/cancel_event/headers
        utility/custom_variant_type/tree_widget_info_type/headers
        utility/general_utility/headers
        utility/log
)

target_link_libraries(project
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Charts
        Qt::Network
        generated_proto_server_specific
        generated_proto_client_server_shared
        )

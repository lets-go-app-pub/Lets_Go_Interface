//
// Created by jeremiah on 11/3/21.
//

#pragma once

#include <HandleErrors.grpc.pb.h>
#include <unordered_map>
#include <string>

struct ExtractedErrorsWithIndexing {
    handle_errors::ExtractErrorsResponse message;
    std::unordered_multimap<char, int> stack_trace_indexing;
    std::unordered_multimap<char, int> error_message_indexing;

    ExtractedErrorsWithIndexing() = delete;

    explicit ExtractedErrorsWithIndexing(handle_errors::ExtractErrorsResponse _message) : message(std::move(_message)) {
        for (int i = 0; i < (int)message.error_message().stack_trace().size(); i++)
            stack_trace_indexing.insert(
                    std::make_pair(
                            (char) std::tolower(message.error_message().stack_trace()[i]),
                            i
                    )
            );

        for (int i = 0; i < (int)message.error_message().error_message().size(); i++)
            error_message_indexing.insert(
                    std::make_pair(
                            (char) std::tolower(message.error_message().error_message()[i]),
                            i
                    )
            );
    }

};
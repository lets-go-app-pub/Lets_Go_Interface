#pragma once

#include <QObject>
#include <HandleErrors.grpc.pb.h>
#include <ErrorOriginEnum.grpc.pb.h>

#include "run_server_call_base_class.h"
#include "extract_errors_with_indexing.h"

class ExtractErrorsObject: public RunServerCallBaseClass  {
Q_OBJECT
public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void
    extractErrors(ErrorUrgencyLevel urgency_level, long max_number_bytes,
                  handle_errors::ErrorParameters* error_parameters,
                  const std::function<void()>& completed_lambda);

signals:

    void signal_extractErrorsSuccessfullyCompleted(
            ErrorOriginType origin,
            ErrorUrgencyLevel urgency_level,
            int version_number,
            const std::string& file_name,
            int line_number,
            const std::string& total_bytes,
            const std::string& extracted_bytes,
            const std::string& total_docs,
            const std::string& extracted_docs,
            std::shared_ptr<std::vector<ExtractedErrorsWithIndexing>> errors,
            const std::function<void()>& completed_lambda
            );
};
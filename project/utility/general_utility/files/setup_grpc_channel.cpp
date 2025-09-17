//
// Created by jeremiah on 1/18/23.
//

#include <fstream>
#include <grpcpp/security/credentials.h>

#include "general_utility.h"

[[maybe_unused]] std::string getTestingPublicCertificate();

[[maybe_unused]] std::string getReleasePublicCertificate();

std::shared_ptr<grpc::Channel> setupGrpcChannel(const std::string &address,
                                                int port) {
  const std::string cert =
#ifdef QT_DEBUG
      getTestingPublicCertificate();
#else
      getReleasePublicCertificate();
#endif

  grpc::SslCredentialsOptions ssl_opts{};
  ssl_opts.pem_root_certs = cert;

  // SetMaxReceiveMessageSize() is set because sending user accounts to the
  // desktop interface
  //  uses a list of pictures instead of streaming them back
  grpc::ChannelArguments ch_args;
  ch_args.SetMaxReceiveMessageSize(100L * 1024L * 1024L); // 100Mb
  ch_args.SetSslTargetNameOverride("LetsGo");             // TLS certificate CN

  return grpc::CreateCustomChannel(address + ":" + std::to_string(port),
                                   grpc::SslCredentials(ssl_opts), ch_args);
}

[[maybe_unused]] std::string getTestingPublicCertificate() {
  return "-----BEGIN CERTIFICATE-----\n"
         "{redacted}"
         "-----END CERTIFICATE-----";
}

[[maybe_unused]] std::string getReleasePublicCertificate() {
  return "-----BEGIN CERTIFICATE-----\n"
         "{redacted}"
         "-----END CERTIFICATE-----";
}

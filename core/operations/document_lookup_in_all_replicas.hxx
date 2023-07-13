/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *   Copyright 2020-2021 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#pragma once

#include "core/error_context/key_value.hxx"
#include "core/impl/get_replica.hxx"
#include "core/impl/subdoc/command.hxx"
#include "core/operations/operation_traits.hxx"
#include "core/protocol/cmd_lookup_in_replica.hxx"
#include "core/utils/movable_function.hxx"
#include "couchbase/codec/encoded_value.hxx"
#include "couchbase/error_codes.hxx"
#include "document_get_all_replicas.hxx"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace couchbase::core::operations
{

struct lookup_in_all_replicas_response {
    struct entry {
        struct lookup_in_entry {
            std::string path;
            codec::binary value;
            std::size_t original_index;
            bool exists;
            protocol::subdoc_opcode opcode;
            key_value_status_code status;
            std::error_code ec{};
        };
        couchbase::cas cas{};
        bool deleted{ false };
        bool is_replica{ true };
        std::vector<lookup_in_entry> lookup_in_entries{};
    };
    subdocument_error_context ctx;
    std::vector<entry> entries{};
};

struct lookup_in_all_replicas_request {
    using response_type = lookup_in_all_replicas_response;
    using encoded_request_type = core::protocol::client_request<core::protocol::lookup_in_replica_request_body>;
    using encoded_response_type = core::protocol::client_response<core::protocol::lookup_in_replica_response_body>;

    core::document_id id;
    std::vector<couchbase::core::impl::subdoc::command> specs{};
    std::optional<std::chrono::milliseconds> timeout{};
    // read replica true ?

    template<typename Core, typename Handler>
    void execute(Core core, Handler handler)
    {
        //TODO: fix/complete this
        core->with_bucket_configuration(
          id.bucket(),
          [core, id = id, timeout = timeout, h = std::forward<Handler>(handler)](std::error_code ec,
                                                                               const topology::configuration& config) mutable {
              if (ec) {
                  std::optional<std::string> first_error_path{};
                  std::optional<std::size_t> first_error_index{};
                  return h(response_type{ make_subdocument_error_context(make_key_value_error_context(ec, id), ec, first_error_path, first_error_index, false)});
              }
              using handler_type = utils::movable_function<void(response_type)>;

              struct replica_context {
                  replica_context(handler_type handler, std::uint32_t expected_responses)
                  : handler_(std::move(handler))
                  , expected_responses_(expected_responses)
                  {
                  }

                  handler_type handler_;
                  std::uint32_t expected_responses_;
                  bool done_{ false };
                  std::mutex mutex_{};
                  std::vector<lookup_in_all_replicas_response::entry> result_{};
              };


          }

          )
    }
};

template<>
struct is_compound_operation<lookup_in_all_replicas_request> : public std::true_type {
};
} // namespace couchbase::core::operations

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

#include <couchbase/platform/uuid.h>

#include <couchbase/io/http_context.hxx>
#include <couchbase/io/http_message.hxx>
#include <couchbase/timeout_defaults.hxx>

#include <couchbase/error_context/http.hxx>

namespace couchbase::operations::management
{
struct query_index_get_all_response {
    struct query_index {
        bool is_primary{ false };
        std::string id;
        std::string name;
        std::string state;
        std::string datastore_id;
        std::string keyspace_id;
        std::string namespace_id;
        std::string collection_name;
        std::string type;
        std::vector<std::string> index_key{};
        std::optional<std::string> partition{};
        std::optional<std::string> condition{};
        std::optional<std::string> bucket_id{};
        std::optional<std::string> scope_id{};
    };
    error_context::http ctx;
    std::string status{};
    std::vector<query_index> indexes{};
};

struct query_index_get_all_request {
    using response_type = query_index_get_all_response;
    using encoded_request_type = io::http_request;
    using encoded_response_type = io::http_response;
    using error_context_type = error_context::http;

    static const inline service_type type = service_type::query;

    std::string client_context_id{ uuid::to_string(uuid::random()) };
    std::string bucket_name;
    std::chrono::milliseconds timeout{ timeout_defaults::management_timeout };

    [[nodiscard]] std::error_code encode_to(encoded_request_type& encoded, http_context& context) const;

    [[nodiscard]] query_index_get_all_response make_response(error_context::http&& ctx, const encoded_response_type& encoded) const;
};
} // namespace couchbase::operations::management
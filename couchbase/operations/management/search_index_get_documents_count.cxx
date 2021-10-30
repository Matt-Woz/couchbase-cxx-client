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

#include <couchbase/operations/management/search_index_get_documents_count.hxx>

#include <couchbase/errors.hxx>

#include <couchbase/utils/json.hxx>

namespace couchbase::operations::management
{
std::error_code
search_index_get_documents_count_request::encode_to(encoded_request_type& encoded, http_context& /* context */) const
{
    encoded.method = "GET";
    encoded.path = fmt::format("/api/index/{}/count", index_name);
    return {};
}

search_index_get_documents_count_response
search_index_get_documents_count_request::make_response(error_context::http&& ctx, const encoded_response_type& encoded) const
{
    search_index_get_documents_count_response response{ std::move(ctx) };
    if (!response.ctx.ec) {
        switch (encoded.status_code) {
            case 200: {
                tao::json::value payload{};
                try {
                    payload = utils::json::parse(encoded.body);
                } catch (const tao::pegtl::parse_error&) {
                    response.ctx.ec = error::common_errc::parsing_failure;
                    return response;
                }
                response.status = payload.at("status").get_string();
                if (response.status == "ok") {
                    response.count = payload.at("count").get_unsigned();
                    return response;
                }
            } break;
            case 400:
            case 500: {
                tao::json::value payload{};
                try {
                    payload = utils::json::parse(encoded.body);
                } catch (const tao::pegtl::parse_error&) {
                    response.ctx.ec = error::common_errc::parsing_failure;
                    return response;
                }
                response.status = payload.at("status").get_string();
                response.error = payload.at("error").get_string();
                if (response.error.find("index not found") != std::string::npos) {
                    response.ctx.ec = error::common_errc::index_not_found;
                    return response;
                }
                if (response.error.find("no planPIndexes for indexName") != std::string::npos) {
                    response.ctx.ec = error::search_errc::index_not_ready;
                    return response;
                }
            } break;
        }
        response.ctx.ec = error::common_errc::internal_server_failure;
    }
    return response;
}
} // namespace couchbase::operations::management
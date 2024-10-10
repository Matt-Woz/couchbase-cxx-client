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

#include "collection_update.hxx"

#include "core/utils/json.hxx"
#include "core/utils/url_codec.hxx"
#include "error_utils.hxx"

#include <fmt/core.h>
#include <tao/json/value.hpp>

#include <regex>

namespace couchbase::core::operations::management
{
auto
collection_update_request::encode_to(encoded_request_type& encoded,
                                     http_context& /*context*/) const -> std::error_code
{
  encoded.method = "PATCH";
  encoded.path = fmt::format("/pools/default/buckets/{}/scopes/{}/collections/{}",
                             utils::string_codec::v2::path_escape(bucket_name),
                             utils::string_codec::v2::path_escape(scope_name),
                             utils::string_codec::v2::path_escape(collection_name));
  encoded.headers["content-type"] = "application/x-www-form-urlencoded";
  std::map<std::string, std::string> values{};
  if (max_expiry.has_value()) {
    if (max_expiry.value() >= -1) {
      values["maxTTL"] = std::to_string(max_expiry.value());
    } else {
      return errc::common::invalid_argument;
    }
  }
  if (history.has_value()) {
    values["history"] = history.value() ? "true" : "false";
  }
  encoded.body = utils::string_codec::v2::form_encode(values);
  return {};
}

auto
collection_update_request::make_response(error_context::http&& ctx,
                                         const encoded_response_type& encoded) const
  -> collection_update_response
{
  collection_update_response response{ std::move(ctx) };
  if (!response.ctx.ec) {
    switch (encoded.status_code) {
      case 400: {
        response.ctx.ec = errc::common::invalid_argument;
      } break;
      case 404: {
        const std::regex scope_not_found("Scope with name .+ is not found");
        const std::regex collection_not_found("Collection with name .+ is not found");
        if (std::regex_search(encoded.body.data(), collection_not_found)) {
          response.ctx.ec = errc::common::collection_not_found;
        } else if (std::regex_search(encoded.body.data(), scope_not_found)) {
          response.ctx.ec = errc::common::scope_not_found;
        } else {
          response.ctx.ec = errc::common::bucket_not_found;
        }
      } break;
      case 200: {
        tao::json::value payload{};
        try {
          payload = utils::json::parse(encoded.body.data());
        } catch (const tao::pegtl::parse_error&) {
          response.ctx.ec = errc::common::parsing_failure;
          return response;
        }
        response.uid = std::stoull(payload.at("uid").get_string(), nullptr, 16);
      } break;
      default:
        response.ctx.ec = extract_common_error_code(encoded.status_code, encoded.body.data());
        break;
    }
  }
  return response;
}
} // namespace couchbase::core::operations::management

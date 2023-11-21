/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *   Copyright 2020-Present Couchbase, Inc.
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

#include <couchbase/manager_error_context.hxx>
#include <couchbase/search_index_manager.hxx>

#include "core/cluster.hxx"
#include "core/operations/management/search_index_analyze_document.hxx"

#include <utility>

namespace couchbase::core::impl
{
template<typename Response>
static manager_error_context
build_context(Response& resp)
{
    return manager_error_context(internal_manager_error_context{ resp.ctx.ec,
                                                                 resp.ctx.last_dispatched_to,
                                                                 resp.ctx.last_dispatched_from,
                                                                 resp.ctx.retry_attempts,
                                                                 std::move(resp.ctx.retry_reasons),
                                                                 std::move(resp.ctx.client_context_id),
                                                                 resp.ctx.http_status,
                                                                 std::move(resp.ctx.http_body),
                                                                 std::move(resp.ctx.path) });
}

static core::operations::management::search_index_analyze_document_request
build_analyze_document_request(std::string index_name, std::string encoded_document, const analyze_document_options::built& options)
{

    core::operations::management::search_index_analyze_document_request request{
        std::move(index_name), std::move(encoded_document), {}, options.timeout
    };
    return request;
}

void
initiate_analyze_document_operation(std::shared_ptr<couchbase::core::cluster> core,
                                    std::string index_name,
                                    std::string document,
                                    analyze_document_options::built options,
                                    analyze_document_handler&& handler)
{
    auto request = build_analyze_document_request(std::move(index_name), std::move(document), options);

    core->execute(std::move(request),
                  [handler = std::move(handler)](operations::management::search_index_analyze_document_response resp) mutable {
                      return handler(build_context(resp), resp.analysis);
                  });
}
} // namespace couchbase::core::impl

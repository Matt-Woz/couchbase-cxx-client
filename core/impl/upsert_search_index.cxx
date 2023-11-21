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
#include "core/operations/management/search_index_upsert.hxx"

#include <utility>

namespace couchbase
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

static core::operations::management::search_index_upsert_request
build_upsert_index_request(couchbase::core::management::search::index index, const upsert_search_index_options::built& options)
{
    core::operations::management::search_index_upsert_request request{ index, {}, options.timeout };
    return request;
}

static couchbase::core::management::search::index
map_search_index(const couchbase::management::search::index& index)
{
    couchbase::core::management::search::index search_index{};
    search_index.uuid = index.uuid;
    search_index.name = index.name;
    search_index.type = index.type;
    search_index.params_json = index.params_json;

    search_index.source_name = index.source_name;
    search_index.source_params_json = index.source_params_json;
    search_index.source_type = index.source_type;
    search_index.source_uuid = index.source_uuid;

    search_index.plan_params_json = index.plan_params_json;

    return search_index;
}

void
search_index_manager::upsert_index(const management::search::index& search_index,
                                   const couchbase::upsert_search_index_options& options,
                                   couchbase::upsert_search_index_handler&& handler) const
{
    auto request = build_upsert_index_request(map_search_index(search_index), options.build());

    core_->execute(std::move(request),
                   [handler = std::move(handler)](core::operations::management::search_index_upsert_response resp) mutable {
                       return handler(build_context(resp));
                   });
}

auto
search_index_manager::upsert_index(const management::search::index& search_index,
                                   const couchbase::upsert_search_index_options& options) const -> std::future<manager_error_context>
{
    auto barrier = std::make_shared<std::promise<manager_error_context>>();
    upsert_index(search_index, options, [barrier](auto ctx) mutable { barrier->set_value(std::move(ctx)); });
    return barrier->get_future();
}
} // namespace couchbase

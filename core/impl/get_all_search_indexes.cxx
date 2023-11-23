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
#include "core/operations/management/search_index_get_all.hxx"

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

static core::operations::management::search_index_get_all_request
build_get_all_indexes_request(const get_all_search_indexes_options::built& options)
{
    core::operations::management::search_index_get_all_request request{ {}, options.timeout };
    return request;
}

static couchbase::management::search::index
map_search_index(const couchbase::core::management::search::index& index)
{
    couchbase::management::search::index public_index{};
    public_index.uuid = index.uuid;
    public_index.name = index.name;
    public_index.type = index.type;
    public_index.params_json = index.params_json;

    public_index.source_name = index.source_name;
    public_index.source_type = index.source_type;
    public_index.source_uuid = index.source_uuid;

    if (index.plan_params_json != "{}") {
        public_index.plan_params_json = index.plan_params_json;
    }
    if (index.source_params_json != "{}") {
        public_index.source_params_json = index.source_params_json;
    }
    return public_index;
}

static std::vector<couchbase::management::search::index>
map_all_search_indexes(const std::vector<couchbase::core::management::search::index>& indexes)
{
    std::vector<couchbase::management::search::index> search_indexes{};
    for (const auto& index : indexes) {
        auto converted_index = map_search_index(index);
        search_indexes.emplace_back(converted_index);
    }
    return search_indexes;
}

void
search_index_manager::get_all_indexes(const couchbase::get_all_search_indexes_options& options,
                                      couchbase::get_all_search_indexes_handler&& handler) const
{
    auto request = build_get_all_indexes_request(options.build());

    core_->execute(std::move(request),
                   [handler = std::move(handler)](core::operations::management::search_index_get_all_response resp) mutable {
                       return handler(build_context(resp), map_all_search_indexes(resp.indexes));
                   });
}

auto
search_index_manager::get_all_indexes(const couchbase::get_all_search_indexes_options& options) const
  -> std::future<std::pair<manager_error_context, std::vector<management::search::index>>>
{
    auto barrier = std::make_shared<std::promise<std::pair<manager_error_context, std::vector<management::search::index>>>>();
    get_all_indexes(options,
                    [barrier](auto ctx, auto result) mutable { barrier->set_value(std::make_pair(std::move(ctx), std::move(result))); });
    return barrier->get_future();
}
} // namespace couchbase

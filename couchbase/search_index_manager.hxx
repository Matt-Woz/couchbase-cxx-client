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

#pragma once

#include <couchbase/allow_querying_search_index_options.hxx>
#include <couchbase/analyze_document_options.hxx>
#include <couchbase/disallow_querying_search_index_options.hxx>
#include <couchbase/drop_search_index_options.hxx>
#include <couchbase/freeze_plan_search_index_options.hxx>
#include <couchbase/get_all_search_indexes_options.hxx>
#include <couchbase/get_indexed_search_index_options.hxx>
#include <couchbase/get_search_index_options.hxx>
#include <couchbase/pause_ingest_search_index_options.hxx>
#include <couchbase/resume_ingest_search_index_options.hxx>
#include <couchbase/unfreeze_plan_search_index_options.hxx>
#include <couchbase/upsert_search_index_options.hxx>

#include <couchbase/codec/json_transcoder.hxx>

#include "core/logger/logger.hxx"

#include <future>

#ifndef COUCHBASE_CXX_CLIENT_DOXYGEN
namespace couchbase::core
{
class cluster;
} // namespace couchbase::core
#endif

namespace couchbase
{
class cluster;

class search_index_manager
{
  public:
    void get_index(std::string index_name, const get_search_index_options& options, get_search_index_handler&& handler) const;

    [[nodiscard]] auto get_index(std::string index_name, const get_search_index_options& options = {}) const
    -> std::future<std::pair<manager_error_context, management::search::index>>;

    void get_all_indexes(const get_all_search_indexes_options& options, get_all_search_indexes_handler&& handler) const;

    [[nodiscard]] auto get_all_indexes(const get_all_search_indexes_options& options = {}) const
    -> std::future<std::pair<manager_error_context, std::vector<management::search::index>>>;

    void upsert_index(const management::search::index& search_index, const upsert_search_index_options& options, upsert_search_index_handler&& handler) const;

    [[nodiscard]] auto upsert_index(const management::search::index& search_index, const upsert_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void drop_index(std::string index_name, const drop_search_index_options& options, drop_search_index_handler&& handler) const;

    [[nodiscard]] auto drop_index(std::string index_name, const drop_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void get_indexed_documents_count(std::string index_name, const get_indexed_search_index_options& options, get_indexed_search_index_handler&& handler) const;

    [[nodiscard]] auto get_indexed_documents_count(std::string index_name, const get_indexed_search_index_options& options = {}) const
    -> std::future<std::pair<manager_error_context, std::uint64_t>>;

    void pause_ingest(std::string index_name, const pause_ingest_search_index_options& options, pause_ingest_search_index_handler&& handler) const;

    [[nodiscard]] auto pause_ingest(std::string index_name, const pause_ingest_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void resume_ingest(std::string index_name, const resume_ingest_search_index_options& options, resume_ingest_search_index_handler&& handler) const;

    [[nodiscard]] auto resume_ingest(std::string index_name, const resume_ingest_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void allow_querying(std::string index_name, const allow_querying_search_index_options& options, allow_querying_search_index_handler&& handler) const;

    [[nodiscard]] auto allow_querying(std::string index_name, const allow_querying_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void disallow_querying(std::string index_name, const disallow_querying_search_index_options& options, disallow_querying_search_index_handler&& handler) const;

    [[nodiscard]] auto disallow_querying(std::string index_name, const disallow_querying_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void freeze_plan(std::string index_name, const freeze_plan_search_index_options& options, freeze_plan_search_index_handler&& handler) const;

    [[nodiscard]] auto freeze_plan(std::string index_name, const freeze_plan_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    void unfreeze_plan(std::string index_name, const unfreeze_plan_search_index_options& options, unfreeze_plan_search_index_handler&& handler) const;

    [[nodiscard]] auto unfreeze_plan(std::string index_name, const unfreeze_plan_search_index_options& options = {}) const
    -> std::future<manager_error_context>;

    template<typename Document>
    void analyze_document(std::string index_name, Document document, const analyze_document_options& options, analyze_document_handler&& handler) const {

        auto encoded = couchbase::codec::default_json_transcoder::encode(document); //Encode as JSON and convert to string

        auto decoded = std::string{ reinterpret_cast<const char*>(encoded.data.data()), encoded.data.size() };

        return core::impl::initiate_analyze_document_operation(
          core_, std::move(index_name), decoded, options.build(), std::forward<analyze_document_handler>(handler)
          );
    }

    template<typename Document>
    [[nodiscard]] auto analyze_document(std::string index_name, Document document, const analyze_document_options& options = {}) const
    -> std::future<std::pair<manager_error_context, std::string>> {
        auto barrier = std::make_shared<std::promise<std::pair<manager_error_context, std::string>>>();
        analyze_document(std::move(index_name), document, options, [barrier](auto ctx, auto result) mutable {
            barrier->set_value(std::make_pair(std::move(ctx), std::move(result)));
        });
        return barrier->get_future();
      };


  private:
    friend class cluster;

    explicit search_index_manager(std::shared_ptr<couchbase::core::cluster> core)
    : core_(std::move(core))
    {
    }

    std::shared_ptr<couchbase::core::cluster> core_;
};
} // namespace couchbase

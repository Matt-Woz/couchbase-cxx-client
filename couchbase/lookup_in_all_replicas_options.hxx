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

#include <couchbase/subdoc/fwd/command_bundle.hxx>

#include <core/impl/subdoc/command.hxx>
#include <couchbase/codec/encoded_value.hxx>
#include <couchbase/common_options.hxx>
#include <couchbase/expiry.hxx>
#include <couchbase/lookup_in_replica_result.hxx>
#include <couchbase/lookup_in_result.hxx>
#include <couchbase/store_semantics.hxx>
#include <couchbase/subdocument_error_context.hxx>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace couchbase
{

struct lookup_in_all_replicas_options : common_options<lookup_in_all_replicas_options> {

    struct built : public common_options<lookup_in_all_replicas_options>::built {
    };

    [[nodiscard]] auto build() const -> built
    {
        return { build_common_options() };
    }
};

using lookup_in_all_replicas_result = std::vector<lookup_in_replica_result>;

using lookup_in_all_replicas_handler = std::function<void(couchbase::subdocument_error_context, lookup_in_all_replicas_result)>;

#ifndef COUCHBASE_CXX_CLIENT_DOXYGEN
namespace core
{
class cluster;
namespace impl
{

void
initiate_lookup_in_all_replicas_operation(std::shared_ptr<couchbase::core::cluster> core,
                                          std::string bucket_name,
                                          std::string scope_name,
                                          std::string collection_name,
                                          std::string document_key,
                                          const std::vector<couchbase::core::impl::subdoc::command>& specs,
                                          lookup_in_all_replicas_options::built options,
                                          lookup_in_all_replicas_handler&& handler);
#endif
} // namespace impl
} // namespace core
} // namespace couchbase
/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *   Copyright 2023-Present Couchbase, Inc.
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

#include <string>

using core_json_context = tao::json::value;

namespace couchbase
{
class operation_error_context
{
  public:
    explicit operation_error_context(core_json_context ctx);

    [[nodiscard]] auto to_string() const -> std::string;

    [[nodiscard]] auto to_json() const -> std::string;

    [[nodiscard]] auto retry_attempts() const -> std::uint64_t;

  private:
    core_json_context ctx_;
};
} // namespace couchbase

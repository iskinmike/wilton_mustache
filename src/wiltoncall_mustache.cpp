/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   wiltoncall_mustache.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 5:53 PM
 */

#include "staticlib/support.hpp"
#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton_mustache.h"

#include "wilton/support/buffer.hpp"
#include "wilton/support/exception.hpp"
#include "wilton/support/registrar.hpp"

namespace wilton {
namespace mustache {

support::buffer mustache_render(sl::io::span<const char> data) {
    // parse json
    auto json = sl::json::load(data);
    auto rtemplate = std::ref(sl::utils::empty_string());
    std::string values = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("template" == name) {
            rtemplate = fi.as_string_nonempty_or_throw(name);
        } else if ("values" == name) {
            values = fi.val().dumps();
        } else {
            throw support::exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rtemplate.get().empty()) throw support::exception(TRACEMSG(
            "Required parameter 'template' not specified"));
    const std::string& templade = rtemplate.get();
    if (values.empty()) {
        values = "{}";
    }
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_mustache_render(templade.c_str(), static_cast<int>(templade.length()),
            values.c_str(), static_cast<int>(values.length()), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) support::throw_wilton_error(err, TRACEMSG(err));
    return support::wrap_wilton_buffer(out, out_len);
}

support::buffer mustache_render_file(sl::io::span<const char> data) {
    // parse json
    auto json = sl::json::load(data);
    auto rfile = std::ref(sl::utils::empty_string());
    std::string values = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("file" == name) {
            rfile = fi.as_string_nonempty_or_throw(name);
        } else if ("values" == name) {
            values = fi.val().dumps();
        } else {
            throw support::exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rfile.get().empty()) throw support::exception(TRACEMSG(
            "Required parameter 'file' not specified"));
    const std::string& file = rfile.get();
    if (values.empty()) {
        values = "{}";
    }
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_mustache_render_file(file.c_str(), static_cast<int>(file.length()),
            values.c_str(), static_cast<int>(values.length()), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) support::throw_wilton_error(err, TRACEMSG(err));
    return support::wrap_wilton_buffer(out, out_len);
}

} // namespace
}

extern "C" char* wilton_module_init() {
    try {
        wilton::support::register_wiltoncall("mustache_render", wilton::mustache::mustache_render);
        wilton::support::register_wiltoncall("mustache_render_file", wilton::mustache::mustache_render_file);
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

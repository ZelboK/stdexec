/*
 * Copyright (c) 2023 NVIDIA Corporation
 * Copyright (c) 2023 Maikel Nadolski
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "exec/sequence/ignore_all_values.hpp"
#include "exec/sequence/empty_sequence.hpp"

#include <catch2/catch.hpp>

namespace {

  TEST_CASE("ignore_all_values - ignore empty sequence", "[sequence_senders][ignore_all_values]") {
    auto sndr = exec::ignore_all_values(exec::empty_sequence());
    using Sender = decltype(sndr);
    STATIC_REQUIRE(stdexec::sender_in<Sender, stdexec::empty_env>);
    STATIC_REQUIRE(stdexec::same_as<
                   stdexec::completion_signatures<stdexec::set_value_t()>,
                   stdexec::completion_signatures_of_t<Sender, stdexec::empty_env>>);
    STATIC_REQUIRE(stdexec::sender_expr_for<Sender, exec::ignore_all_values_t>);
    CHECK(stdexec::sync_wait(sndr));
  }

  TEST_CASE("ignore_all_values - ignore just(42)", "[sequence_senders][ignore_all_values]") {
    auto sndr = exec::ignore_all_values(stdexec::just(42));
    using Sender = decltype(sndr);
    STATIC_REQUIRE(stdexec::sender_in<Sender, stdexec::empty_env>);
    STATIC_REQUIRE(stdexec::same_as<
                   stdexec::completion_signatures<stdexec::set_value_t()>,
                   stdexec::completion_signatures_of_t<Sender, stdexec::empty_env>>);
    CHECK(stdexec::sync_wait(sndr));
  }

  TEST_CASE("ignore_all_values - ignore just()", "[sequence_senders][ignore_all_values]") {
    auto sndr = exec::ignore_all_values(stdexec::just());
    using Sender = decltype(sndr);
    STATIC_REQUIRE(stdexec::sender_in<Sender, stdexec::empty_env>);
    STATIC_REQUIRE(stdexec::same_as<
                   stdexec::completion_signatures<stdexec::set_value_t()>,
                   stdexec::completion_signatures_of_t<Sender, stdexec::empty_env>>);
    CHECK(stdexec::sync_wait(sndr));
  }

  TEST_CASE("ignore_all_values - ignore just_stopped()", "[sequence_senders][ignore_all_values]") {
    auto sndr = exec::ignore_all_values(stdexec::just_stopped());
    using Sender = decltype(sndr);
    STATIC_REQUIRE(stdexec::sender_in<Sender, stdexec::empty_env>);
    STATIC_REQUIRE(stdexec::same_as<
                   stdexec::completion_signatures<stdexec::set_value_t(), stdexec::set_stopped_t()>,
                   stdexec::completion_signatures_of_t<Sender, stdexec::empty_env>>);
    CHECK_FALSE(stdexec::sync_wait(sndr));
  }

  TEST_CASE("ignore_all_values - ignore just_error()", "[sequence_senders][ignore_all_values]") {
    auto sndr = exec::ignore_all_values(
      stdexec::just_error(std::make_exception_ptr(std::runtime_error("test"))));
    using Sender = decltype(sndr);
    STATIC_REQUIRE(stdexec::sender_in<Sender, stdexec::empty_env>);
    STATIC_REQUIRE(
      stdexec::same_as<
        stdexec::
          completion_signatures<stdexec::set_value_t(), stdexec::set_error_t(std::exception_ptr)>,
        stdexec::completion_signatures_of_t<Sender, stdexec::empty_env>>);
    CHECK_THROWS(stdexec::sync_wait(sndr));
  }

  struct sequence_op {
    friend void tag_invoke(stdexec::start_t, sequence_op&) noexcept {
    }
  };

  template <class Item>
  struct sequence {
    using is_sender = exec::sequence_tag;

    using completion_signatures =
      stdexec::completion_signatures<stdexec::set_value_t(), stdexec::set_error_t(int)>;

    using item_types = exec::item_types<Item>;

    friend sequence_op tag_invoke(exec::subscribe_t, sequence, stdexec::__ignore) noexcept {
      return sequence_op{};
    }
  };

  TEST_CASE("ignore_all_values - Merge error and stop signatures from sequence and items") {
    using just_t =
      decltype(stdexec::just_error(std::make_exception_ptr(std::runtime_error("test"))));
    sequence<just_t> seq;
    auto ignore = exec::ignore_all_values(seq);
    using Sigs = stdexec::completion_signatures_of_t<decltype(ignore), stdexec::empty_env>;
    using ExpectedSigs = stdexec::completion_signatures<
      stdexec::set_value_t(),
      stdexec::set_error_t(int),
      stdexec::set_error_t(std::exception_ptr)>;
    STATIC_REQUIRE(std::same_as<Sigs, ExpectedSigs>);
  }
}

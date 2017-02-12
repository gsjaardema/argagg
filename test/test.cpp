#include "../include/argagg/argagg.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstring>
#include <iostream>
#include <vector>


TEST_CASE("intro example")
{
  argagg::parser argparser {{
      { "help", {"-h", "--help"},
        "shows this help message", 0},
      { "delim", {"-d", "--delim"},
        "delimiter (default: ,)", 1},
      { "num", {"-n", "--num"},
        "number", 1},
    }};
  std::vector<const char*> argv {
    "test", "3.141", "foo", "-h", "bar", "300", "-n", "100", "-d", "--", "-"};
  argagg::parser_results args;
  try {
    args = argparser.parse(argv.size(), &(argv.front()));
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  CHECK(args.has_option("help") == true);
  CHECK(static_cast<bool>(args["help"]) == true);
  CHECK(args.has_option("delim") == true);
  CHECK(static_cast<bool>(args["delim"]) == true);
  auto delim = args["delim"].as<std::string>(",");
  CHECK(delim == "-");
  CHECK(args.has_option("num") == true);
  CHECK(static_cast<bool>(args["num"]) == true);
  int x = 0;
  if (args["num"]) {
    x = args["num"];
  }
  CHECK(x == 100);
  auto y = 0.0;
  if (args.pos.size() > 0) {
    y = args.as<double>(0);
  }
  CHECK(y == doctest::Approx(3.141));
  CHECK(args.as<std::string>(1) == "foo");
  CHECK(args.as<std::string>(2) == "bar");
  CHECK(args.as<int>(3) == 300);
}


TEST_CASE("no definitions")
{
  argagg::parser parser {{
    }};
  SUBCASE("no arguments") {
    std::vector<const char*> argv {
      "test"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == false);
    CHECK(args.has_option("output") == false);
    CHECK(args.count() == 0);
  }
  SUBCASE("with arguments") {
    std::vector<const char*> argv {
      "test", "foo", "bar", "baz"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == false);
    CHECK(args.has_option("output") == false);
    CHECK(args.count() == 3);
    CHECK(::std::string(args.pos[0]) == "foo");
    CHECK(::std::string(args.pos[1]) == "bar");
    CHECK(::std::string(args.pos[2]) == "baz");
  }
}


TEST_CASE("general")
{
  argagg::parser parser {{
      {"verbose", {"-v", "--verbose"}, "be verbose", 0},
      {"output", {"-o", "--output"}, "output filename", 1},
    }};
  SUBCASE("no arguments") {
    std::vector<const char*> argv {
      "test"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == false);
    CHECK(args.has_option("output") == false);
    CHECK(args.count() == 0);
  }
  SUBCASE("no flags") {
    std::vector<const char*> argv {
      "test", "foo", "bar", "baz"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == false);
    CHECK(args.has_option("output") == false);
    CHECK(args.count() == 3);
    CHECK(::std::string(args.pos[0]) == "foo");
    CHECK(::std::string(args.pos[1]) == "bar");
    CHECK(::std::string(args.pos[2]) == "baz");
  }
  SUBCASE("only flags") {
    std::vector<const char*> argv {
      "test", "--verbose", "--output", "foo", "-v", "-o", "bar"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == true);
    CHECK(args["verbose"].count() == 2);
    CHECK(args["verbose"][0].arg == nullptr);
    CHECK(args["verbose"][1].arg == nullptr);
    CHECK(args.has_option("output") == true);
    CHECK(args["output"].count() == 2);
    CHECK(args["output"][0].as<std::string>() == "foo");
    CHECK(args["output"][1].as<std::string>() == "bar");
    CHECK(args.count() == 0);
  }
  SUBCASE("simple mixed") {
    std::vector<const char*> argv {
      "test", "-v", "--output", "foo", "bar"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == true);
    CHECK(args["verbose"].count() == 1);
    CHECK(args["verbose"][0].arg == nullptr);
    CHECK(args.has_option("output") == true);
    CHECK(args["output"].count() == 1);
    CHECK(args["output"].as<std::string>() == "foo");
    CHECK(args.count() == 1);
    CHECK(args.as<std::string>(0) == "bar");
  }
  SUBCASE("trailing flags") {
    std::vector<const char*> argv {
      "test", "foo", "bar", "-v", "--output", "baz"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == true);
    CHECK(args["verbose"].count() == 1);
    CHECK(args["verbose"][0].arg == nullptr);
    CHECK(args.has_option("output") == true);
    CHECK(args["output"].count() == 1);
    CHECK(args["output"].as<std::string>() == "baz");
    CHECK(args.count() == 2);
    CHECK(args.as<std::string>(0) == "foo");
    CHECK(args.as<std::string>(1) == "bar");
  }
  SUBCASE("interleaved positional arguments") {
    std::vector<const char*> argv {
      "test", "foo", "-v", "bar", "--verbose", "baz", "--output", "dog", "cat"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == true);
    CHECK(args["verbose"].count() == 2);
    CHECK(args["verbose"][0].arg == nullptr);
    CHECK(args.has_option("output") == true);
    CHECK(args["output"].count() == 1);
    CHECK(args["output"].as<std::string>() == "dog");
    CHECK(args.count() == 4);
    CHECK(args.as<std::string>(0) == "foo");
    CHECK(args.as<std::string>(1) == "bar");
    CHECK(args.as<std::string>(2) == "baz");
    CHECK(args.as<std::string>(3) == "cat");
  }
  SUBCASE("unused short flag") {
    std::vector<const char*> argv {
      "test", "--output", "foo", "-h", "bar", "-v"};
    CHECK_THROWS_AS({
      argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    }, argagg::unexpected_option_error);
  }
  SUBCASE("unused long flag") {
    std::vector<const char*> argv {
      "test", "--output", "foo", "--help", "bar", "-v"};
    CHECK_THROWS_AS({
      argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    }, argagg::unexpected_option_error);
  }
}


TEST_CASE("flag stop")
{
  argagg::parser parser {{
      {"verbose", {"-v", "--verbose"}, "be verbose", 0},
      {"delim", {"-d", "--delim"}, "delimiter", argagg::optional},
    }};
  SUBCASE("ignore flags after stop") {
    std::vector<const char*> argv {
      "test", "-v", "--", "bar", "--verbose", "baz", "--delim", "dog", "-d", "cat"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == true);
    CHECK(args["verbose"].count() == 1);
    CHECK(args["verbose"][0].arg == nullptr);
    CHECK(args.has_option("output") == false);
    CHECK(args.count() == 7);
    CHECK(args.as<std::string>(0) == "bar");
    CHECK(args.as<std::string>(1) == "--verbose");
    CHECK(args.as<std::string>(2) == "baz");
    CHECK(args.as<std::string>(3) == "--delim");
    CHECK(args.as<std::string>(4) == "dog");
    CHECK(args.as<std::string>(5) == "-d");
    CHECK(args.as<std::string>(6) == "cat");
  }
  SUBCASE("use flag stop for dash argument") {
    std::vector<const char*> argv {
      "test", "-d", "--", "-", "boo"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("help") == false);
    CHECK(args.has_option("verbose") == false);
    CHECK(args.has_option("delim") == true);
    CHECK(args["delim"].as<std::string>() == "-");
    CHECK(args.count() == 1);
    CHECK(args.as<std::string>(0) == "boo");
  }
}


TEST_CASE("need one flag argument")
{
  argagg::parser parser {{
      {"verbose", {"-v", "--verbose"}, "be verbose", 0},
      {"number", {"-n", "--number"}, "number", 1},
    }};
  SUBCASE("arguments provided") {
    std::vector<const char*> argv {
      "test", "-n", "1", "2", "-n", "4"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("verbose") == false);
    CHECK(args.has_option("number") == true);
    CHECK(args["number"].count() == 2);
    CHECK(args["number"][0].as<int>() == 1);
    CHECK(args["number"][1].as<int>() == 4);
    CHECK(args.count() == 1);
    CHECK(args.as<int>(0) == 2);
  }
  SUBCASE("given zero, interrupted by flag")
  {
    std::vector<const char*> argv {
      "test", "-n", "-v"};
    CHECK_THROWS_AS({
      argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    }, argagg::option_lacks_argument_error);
  }
  SUBCASE("interrupted by unused flag")
  {
    std::vector<const char*> argv {
      "test", "-n", "1", "2", "-c"};
    CHECK_THROWS_AS({
      argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    }, argagg::unexpected_option_error);
  }
  SUBCASE("given zero, end of args") {
    std::vector<const char*> argv {
      "test", "-n"};
    CHECK_THROWS_AS({
      argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    }, argagg::option_lacks_argument_error);
  }
}


TEST_CASE("optional flag arguments")
{
  argagg::parser parser {{
      {"verbose", {"-v", "--verbose"}, "be verbose", 0},
      {"number", {"-n", "--number"}, "number", argagg::optional},
    }};
  std::vector<const char*> argv {
    "test", "-n", "1", "-n", "-n", "3", "-v", "4"};
  argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
  CHECK(args.has_option("verbose") == true);
  CHECK(args["verbose"].count() == 1);
  CHECK(args.has_option("number") == true);
  CHECK(args["number"].count() == 3);
  CHECK(args["number"][0].as<int>() == 1);
  CHECK(args["number"][1].as<int>(0) == 0);
  CHECK(args["number"][2].as<int>() == 3);
  CHECK(args.count() == 1);
  CHECK(args.as<int>(0) == 4);
}


TEST_CASE("argument conversions")
{
  argagg::parser parser {{
      {"number", {"-n", "--num", "--number"}, "number", 1},
    }};
  SUBCASE("positional integer") {
    std::vector<const char*> argv {
      "test", "1", "2"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.pos.size() == 2);
    CHECK(args.as<int>() == 1);
    CHECK(args.as<long>() == 1);
    CHECK(args.as<long long>() == 1);
    CHECK(args.as<std::string>() == "1");
    CHECK(args.as<int>(1) == 2);
    CHECK(args.as<long>(1) == 2);
    CHECK(args.as<long long>(1) == 2);
    CHECK(args.as<std::string>(1) == "2");
  }
  SUBCASE("positional floating point") {
    std::vector<const char*> argv {
      "test", "3.141592653", "2.71828182846"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.pos.size() == 2);
    CHECK(args.as<float>() == doctest::Approx(3.141592653f));
    CHECK(args.as<double>() == doctest::Approx(3.141592653));
    CHECK(args.as<std::string>() == "3.141592653");
    CHECK(args.as<float>(1) == doctest::Approx(2.71828182846f));
    CHECK(args.as<double>(1) == doctest::Approx(2.71828182846));
    CHECK(args.as<std::string>(1) == "2.71828182846");
  }
  SUBCASE("positional vector") {
    std::vector<const char*> argv {
      "test", "0", "1", "2"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.pos.size() == 3);
    auto v = args.all_as<int>();
    CHECK(v[0] == 0);
    CHECK(v[1] == 1);
    CHECK(v[2] == 2);
  }
  SUBCASE("option integer") {
    std::vector<const char*> argv {
      "test", "-n", "1"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("number") == true);
    CHECK(args["number"].count() == 1);
    CHECK(args["number"].as<int>() == 1);
    CHECK(args["number"].as<long>() == 1);
    CHECK(args["number"].as<long long>() == 1);
    CHECK(args["number"].as<std::string>() == "1");
  }
  SUBCASE("option floating point") {
    std::vector<const char*> argv {
      "test", "-n", "3.141592653"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("number") == true);
    CHECK(args["number"].count() == 1);
    CHECK(args["number"].as<float>() == doctest::Approx(3.141592653f));
    CHECK(args["number"].as<double>() == doctest::Approx(3.141592653));
    CHECK(args["number"].as<std::string>() == "3.141592653");
  }
  SUBCASE("option implicit conversions") {
    std::vector<const char*> argv {
      "test", "-n", "3.141592653", "-n", "2"};
    argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
    CHECK(args.has_option("number") == true);
    CHECK(args["number"].count() == 2);
    float x = args["number"][0];
    int y = args["number"][1];
    CHECK(x == doctest::Approx(3.141592653f));
    CHECK(y == 2);
  }
}


namespace argagg {
namespace convert {
  template <>
  std::vector<std::string> arg(const char* s)
  {
    std::vector<std::string> ret {};
    if (std::strlen(s) == 0) {
      return ret;
    }
    while (true) {
      const char* token = std::strchr(s, ',');
      if (token == nullptr) {
        ret.emplace_back(s, std::strlen(s));
        break;
      }
      std::size_t len = token - s;
      ret.emplace_back(s, len);
      s += len + 1;
    }
    return ret;
  }
} // namespace convert
} // namespace argagg


TEST_CASE("custom conversion function")
{
  argagg::parser parser {{
      {"words", {"-w", "--words"}, "words", 1},
    }};
  std::vector<const char*> argv {
    "test", "-w", "hello,world,foo,bar,baz"};
  argagg::parser_results args = parser.parse(argv.size(), &(argv.front()));
  CHECK(args.has_option("words") == true);
  auto v = args["words"].as<std::vector<std::string>>();
  CHECK(v.size() == 5);
  CHECK(v[0] == "hello");
  CHECK(v[1] == "world");
  CHECK(v[2] == "foo");
  CHECK(v[3] == "bar");
  CHECK(v[4] == "baz");
}


TEST_CASE("write options help")
{
  argagg::parser parser {{
      {"help", {"-h", "--help"}, "print help", 0},
      {"verbose", {"-v", "--verbose"}, "be verbose", 0},
      {"output", {"-o", "--output"}, "output filename", 1},
    }};
  // Just checking for no exceptions for now.
  std::cout << parser;
}
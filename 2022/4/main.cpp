#include <ranges>
#include <filesystem>
#include <streambuf>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <concepts>
#include <charconv>

struct ElfPair
{
    std::pair<uint32_t, uint32_t> first;
    std::pair<uint32_t, uint32_t> second;
};

template<std::integral T>
auto svTo(std::string_view str) -> T
{
    T result;
    auto const[ptr, code] {std::from_chars(str.data(), str.data() + str.length(), result)};

    if (code == std::errc::invalid_argument || code == std::errc::result_out_of_range)
    {
        return {};
    }

    return result;
}

auto toSv(auto&& range) -> std::string_view
{
    return std::string_view(&*range.begin(), std::ranges::distance(range));
}

auto isFullyContained(const ElfPair& pair) -> bool
{
    return (pair.first.first >= pair.second.first && pair.first.second <= pair.second.second) ||
           (pair.second.first >= pair.first.first && pair.second.second <= pair.first.second);
}

auto isContained(const ElfPair& pair) -> bool
{
    return (pair.first.second >= pair.second.first && pair.first.first <= pair.second.second) ||
           (pair.second.second >= pair.first.first && pair.second.first <= pair.first.second);
}


auto getFileContent(const std::filesystem::path& path) -> std::string
{
    auto input = std::ifstream{ path };
    return { std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{} };
}

auto splitContent(std::string_view content)
{
    using namespace std::literals::string_view_literals;
    return content | std::ranges::views::split("\n"sv) | std::ranges::views::transform([](auto&& splitView) {
        return toSv(splitView);
    });
}

auto toElfPair(auto&& range) -> ElfPair
{
    const auto elf1 = *std::ranges::begin(range);
    const auto elf2 = *std::next(std::ranges::begin(range));

    return {
        {svTo<uint32_t>(*std::ranges::begin(elf1)), svTo<uint32_t>(*std::next(std::ranges::begin(elf1)))},
        {svTo<uint32_t>(*std::ranges::begin(elf2)), svTo<uint32_t>(*std::next(std::ranges::begin(elf2)))}
    };
    return {};
}

auto getElfPairs(auto&& content)
{
    using namespace std::literals::string_view_literals;
    return content | std::ranges::views::transform([](auto&& line) {
        return line | std::ranges::views::split(","sv) | std::ranges::views::transform([](auto&& elf) {
            return elf | std::ranges::views::split("-"sv) | std::ranges::views::transform([](auto&& number) {
                return toSv(number);
            });
        });
    }) | std::ranges::views::transform([](auto&& line) {
        return toElfPair(line);
    });
}

auto getNumberOfFullyContained(auto&& range) -> size_t
{
    return std::ranges::count_if(range, isFullyContained);
}

auto getNumberOfContained(auto&& range) -> size_t
{
    return std::ranges::count_if(range, isContained);
}

auto main() -> int
{
    std::cout << "1: " << getNumberOfFullyContained(getElfPairs(splitContent(getFileContent("data.txt")))) << "\n";
    std::cout << "2: " << getNumberOfContained(getElfPairs(splitContent(getFileContent("data.txt")))) << "\n";
}

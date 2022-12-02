#include <ranges>
#include <filesystem>
#include <streambuf>
#include <fstream>
#include <charconv>
#include <iostream>
#include <numeric>

template <typename C>
struct to_helper {};

// This actually does the work
template <typename Container, std::ranges::range R>
requires std::convertible_to<std::ranges::range_value_t<R>, typename Container::value_type>
Container operator|(R&& r, to_helper<Container>) {
    return Container{r.begin(), r.end()};
}

template <std::ranges::range Container>
requires (!std::ranges::view<Container>)
auto to() {
    return to_helper<Container>{};
}

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

auto getFileContent(const std::filesystem::path& path) -> std::string
{
    auto input = std::ifstream{path};
    return {std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
}

auto splitContent(std::string_view content)
{
    using namespace std::literals::string_view_literals;
    return content | std::ranges::views::split("\n\n"sv) | std::ranges::views::transform([](auto&& splitView) {
        return splitView | std::ranges::views::split("\n"sv) | std::ranges::views::transform([](auto&& view) {
            return svTo<uint32_t>(std::string_view(&*view.begin(), std::ranges::distance(view)));
        });
    });
}

auto sumContent(auto&& range)
{
    return range | std::ranges::views::transform([](auto&& elem) {
        auto commonRange = elem | std::ranges::views::common;
        return std::accumulate(commonRange.begin(), commonRange.end(), uint32_t{});
    });
}

auto findMaxN(auto&& range, size_t n) -> size_t
{
    auto vec = range | to<std::vector<uint32_t>>();
    std::ranges::sort(vec, std::ranges::greater{});
    auto commonRange = vec | std::ranges::views::take(n) | std::ranges::views::common;
    return std::accumulate(commonRange.begin(), commonRange.end(), uint32_t{});
}

auto main() -> int
{
    const auto content = sumContent(splitContent(getFileContent("data.txt")));
    std::cout << "1: " << findMaxN(content, 1) << "\n";
    std::cout << "2: " << findMaxN(content, 3) << "\n";
}

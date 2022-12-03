#include <ranges>
#include <filesystem>
#include <streambuf>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_set>
#include <set>
#include <algorithm>


auto getFileContent(const std::filesystem::path& path) -> std::string
{
    auto input = std::ifstream{ path };
    return { std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{} };
}

auto splitContent(std::string_view content)
{
    using namespace std::literals::string_view_literals;
    return content | std::ranges::views::split("\n"sv) | std::ranges::views::transform([](auto&& splitView) {
        return std::string_view(&*splitView.begin(), std::ranges::distance(splitView));
    });
}

// Linear in avarage case
auto findCommonItem(std::string_view rucksack) -> char
{
    const auto first = std::unordered_set<char>{ rucksack.begin(), rucksack.begin() + rucksack.size() / 2 };
    const auto second = std::string_view{ rucksack.begin() + rucksack.size() / 2, rucksack.end() };

    for (auto item : second)
    {
        if (first.contains(item))
        {
            return item;
        }
    }

    return 0;
}

auto findCommonItems(auto&& rucksacks)
{
    return rucksacks | std::ranges::views::transform([](auto rucksack) {
        return findCommonItem(rucksack);
    });
}

// O(group.size() * n log(n)) in average case
auto findCommonBadge(auto&& group) -> char
{
    auto setIntersection = std::set<char>((*group.begin()).begin(), (*group.begin()).end());
    for (const auto rucksack : group)
    {
        const auto set = std::set<char>(rucksack.begin(), rucksack.end());
        auto tmpResult = std::set<char>{};
        std::ranges::set_intersection(setIntersection, set, std::inserter(tmpResult, tmpResult.begin()));
        setIntersection = std::move(tmpResult);
    }
    return *setIntersection.begin();
}

auto findCommonBadges(auto&& rucksacks)
{
    return rucksacks | std::ranges::views::chunk(3) | std::ranges::views::transform([](auto&& group) {
        return findCommonBadge(group);
    });
}

auto getItemPriority(char item) -> char
{
    if (item >= 'a')
    {
        return item - 'a' + 1;
    }
    else
    {
        return item - 'A' + 27;
    }
}

auto getPriorities(auto&& items)
{
    return items | std::ranges::views::transform([](auto item) {
        return getItemPriority(item);
    });
}

auto sumPriorities(auto&& priorities)
{
    return std::accumulate(priorities.begin(), priorities.end(), uint32_t{});
}

auto main() -> int
{
    std::cout << "1: " << sumPriorities(getPriorities(findCommonItems(splitContent(getFileContent("data.txt"))))) << "\n";
    std::cout << "2: " << sumPriorities(getPriorities(findCommonBadges(splitContent(getFileContent("data.txt"))))) << "\n";
}

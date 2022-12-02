#include <unordered_map>
#include <ranges>
#include <filesystem>
#include <streambuf>
#include <fstream>
#include <charconv>
#include <iostream>
#include <numeric>

using namespace std::literals::string_view_literals;

enum class Result
{
    LOSE = 0,
    DRAW = 3,
    WIN = 6
};

enum class Shape
{
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3
};

struct PairHash
{
    auto operator()(const std::pair<std::string_view, Shape> &x) const noexcept -> size_t
    {
        return std::hash<std::string_view>()(x.first) ^
               std::hash<std::underlying_type_t<Shape>>()(static_cast<std::underlying_type_t<Shape>>(x.second));
    }
    auto operator()(const std::pair<Shape, Shape> &x) const noexcept -> size_t
    {
        return std::hash<std::underlying_type_t<Shape>>()(static_cast<std::underlying_type_t<Shape>>(x.first)) ^
               std::hash<std::underlying_type_t<Shape>>()(static_cast<std::underlying_type_t<Shape>>(x.second));
    }
};

const auto youCode = std::unordered_map<std::string_view, Shape>{
    {"X"sv, Shape::ROCK},
    {"Y"sv, Shape::PAPER},
    {"Z"sv, Shape::SCISSORS}
};

const auto opponentCode = std::unordered_map<std::string_view, Shape>{
    {"A"sv, Shape::ROCK},
    {"B"sv, Shape::PAPER},
    {"C"sv, Shape::SCISSORS}
};

const auto rules = std::unordered_map<std::pair<Shape, Shape>, Result, PairHash>{
    {{Shape::ROCK, Shape::ROCK}, Result::DRAW},
    {{Shape::ROCK, Shape::PAPER}, Result::LOSE},
    {{Shape::ROCK, Shape::SCISSORS}, Result::WIN},
    {{Shape::PAPER, Shape::ROCK}, Result::WIN},
    {{Shape::PAPER, Shape::PAPER}, Result::DRAW},
    {{Shape::PAPER, Shape::SCISSORS}, Result::LOSE},
    {{Shape::SCISSORS, Shape::ROCK}, Result::LOSE},
    {{Shape::SCISSORS, Shape::PAPER}, Result::DRAW},
    {{Shape::SCISSORS, Shape::SCISSORS}, Result::WIN},
};

const auto extendedYouCode = std::unordered_map<std::pair<std::string_view, Shape>, Shape, PairHash>{
    {{"X"sv, Shape::ROCK}, Shape::SCISSORS},
    {{"Y"sv, Shape::ROCK}, Shape::ROCK},
    {{"Z"sv, Shape::ROCK}, Shape::PAPER},
    {{"X"sv, Shape::PAPER}, Shape::ROCK},
    {{"Y"sv, Shape::PAPER}, Shape::PAPER},
    {{"Z"sv, Shape::PAPER}, Shape::SCISSORS},
    {{"X"sv, Shape::SCISSORS}, Shape::PAPER},
    {{"Y"sv, Shape::SCISSORS}, Shape::SCISSORS},
    {{"Z"sv, Shape::SCISSORS}, Shape::ROCK}
};

class Round
{
public:
    const Shape you;
    const Shape opponent;
    const Result result;
    const uint32_t points;

private:
    auto getRoundResult() const -> Result
    {
        return rules.at({you, opponent});
    }

    auto getRoundPoints() const -> uint32_t
    {
        return static_cast<uint32_t>(result) + static_cast<uint32_t>(you);
    }

public:
    Round(Shape you, Shape opponent)
        : you{you}, opponent{opponent}, result{getRoundResult()}, points{getRoundPoints()}
    {
    }
};

template <typename C>
struct to_helper
{
};

// This actually does the work
template <typename Container, std::ranges::range R>
requires std::convertible_to < std::ranges::range_value_t<R>,
typename Container::value_type >
    Container operator|(R &&r, to_helper<Container>)
{
    return Container{r.begin(), r.end()};
}

template <std::ranges::range Container>
requires(!std::ranges::view<Container>) auto to()
{
    return to_helper<Container>{};
}

auto toRound(std::string_view opponent, std::string_view you) -> Round
{
    return Round{youCode.at(you), opponentCode.at(opponent)};
}

auto toRoundExtended(std::string_view opponent, std::string_view you) -> Round
{
    const auto opponentShape = opponentCode.at(opponent);
    return Round{extendedYouCode.at({you, opponentShape}), opponentShape};
}

auto toRounds(auto &&range) -> std::vector<Round>
{
    return range | std::ranges::views::transform([](auto &&view) {
        return toRound(*view.begin(), *(std::next(view.begin())));
    }) | to<std::vector<Round>>();
}

auto toRoundsExtended(auto &&range) -> std::vector<Round>
{
    return range | std::ranges::views::transform([](auto &&view) {
        return toRoundExtended(*view.begin(), *(std::next(view.begin())));
    }) | to<std::vector<Round>>();
}

auto getPoints(const std::vector<Round> &rounds) -> uint32_t
{
    return std::accumulate(rounds.begin(), rounds.end(), uint32_t{}, [](uint32_t points, const Round &round) {
        return points + round.points;
    });
}

auto getFileContent(const std::filesystem::path &path) -> std::string
{
    auto input = std::ifstream{path};
    return {std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
}

auto splitContent(std::string_view content)
{
    return content | std::ranges::views::split("\n"sv) | std::ranges::views::transform([](auto &&splitView) {
        return splitView | std::ranges::views::split(" "sv) | std::ranges::views::transform([](auto &&view) {
            return std::string_view{&*view.begin(), static_cast<size_t>(std::ranges::distance(view))};
        });
    });
}

auto main() -> int
{
    std::cout << "1: " << getPoints(toRounds(splitContent(getFileContent("data.txt")))) << "\n";
    std::cout << "2: " << getPoints(toRoundsExtended(splitContent(getFileContent("data.txt")))) << "\n";
}

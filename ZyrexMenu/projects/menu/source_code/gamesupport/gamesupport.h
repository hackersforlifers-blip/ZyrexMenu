#pragma once
#include <cstdint>
#include <string_view>

namespace gamesupport
{
	enum class GameKey : std::uint8_t
	{
		Unknown = 0,
		PhantomForces,
		MurderMystery2,
		LumberTycoon2,
		BadBusiness,
		Fallen,
		Locked,
	};

	namespace game_ids
	{
		// Universe IDs
		constexpr std::uint64_t MurderMystery2   = 66654135ULL;
		constexpr std::uint64_t LumberTycoon2   = 2471084ULL;
		constexpr std::uint64_t LockedUniverse  = 4945312977ULL;
		// Place IDs (used for games where Universe ID differs)
		constexpr std::uint64_t PhantomForces    = 292439477ULL;
		constexpr std::uint64_t BadBusiness     = 3233893879ULL;
		constexpr std::uint64_t Fallen          = 10228136016ULL;
		constexpr std::uint64_t Fallen2         = 13800717766ULL;
		constexpr std::uint64_t Locked          = 109883052223750ULL;
	}

	struct Detection final
	{
		GameKey key{ GameKey::Unknown };
		std::uint64_t game_id{ 0 };
		std::uint64_t place_id{ 0 };
		bool supported{ false };
		std::string_view name{ "Unknown" };
	};

	Detection detect(std::uint64_t game_id, std::uint64_t place_id);
	void log_support_status(const Detection& d);
}

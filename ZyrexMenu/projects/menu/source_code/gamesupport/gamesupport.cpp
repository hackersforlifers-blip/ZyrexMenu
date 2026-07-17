#include "gamesupport.h"
#include <Console.h>

gamesupport::Detection gamesupport::detect(std::uint64_t game_id, std::uint64_t place_id)
{
	Detection d{};
	d.game_id = game_id;
	d.place_id = place_id;
	d.supported = true;

	auto check_by_id = [&](std::uint64_t id, GameKey key, const char* name) -> bool {
		if (place_id == id || game_id == id) {
			d.key = key;
			d.name = name;
			return true;
		}
		return false;
	};

	if (check_by_id(game_ids::PhantomForces, GameKey::PhantomForces, "phantom forces")) return d;
	if (check_by_id(game_ids::MurderMystery2, GameKey::MurderMystery2, "murder mystery 2")) return d;
	if (check_by_id(game_ids::LumberTycoon2, GameKey::LumberTycoon2, "lumber tycoon 2")) return d;
	if (check_by_id(game_ids::BadBusiness, GameKey::BadBusiness, "bad business")) return d;
	if (check_by_id(game_ids::Fallen, GameKey::Fallen, "fallen")) return d;
	if (check_by_id(game_ids::Locked, GameKey::Locked, "LOCKED")) {
		return d;
	}

	// Also check Fallen2 place ID
	if (place_id == game_ids::Fallen2) {
		d.key = GameKey::Fallen;
		d.name = "fallen";
		return d;
	}

	d.key = GameKey::Unknown;
	d.name = "Unknown";
	return d;
}

void gamesupport::log_support_status(const Detection& d)
{
	if (d.key == GameKey::Unknown) return;
	Console::Info("%.*s -> game %s",
		(int)d.name.size(), d.name.data(),
		d.supported ? "supported" : "unsupported");
}

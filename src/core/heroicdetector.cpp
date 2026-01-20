/*!
 * \file heroicdetector.cpp
 * \brief Implementation of Heroic Games Launcher detection.
 */

#include "heroicdetector.h"
#include "log.h"
#include <fstream>
#include <json/json.h>
#include <format>

bool HeroicDetector::isHeroicInstalled()
{
  sfs::path config_dir = getHeroicConfigDir();
  return !config_dir.empty() && sfs::exists(config_dir);
}

std::vector<HeroicGameInfo> HeroicDetector::detectGames()
{
  std::vector<HeroicGameInfo> games;

  sfs::path config_dir = getHeroicConfigDir();
  if(config_dir.empty() || !sfs::exists(config_dir))
  {
    Log::debug("Heroic config directory not found");
    return games;
  }

  // Check Epic store
  sfs::path epic_installed = config_dir / "store" / "installed.json";
  if(sfs::exists(epic_installed))
  {
    auto epic_games = detectGamesFromStore("Epic", epic_installed, config_dir);
    games.insert(games.end(), epic_games.begin(), epic_games.end());
  }

  // Check GOG store
  sfs::path gog_installed = config_dir / "gog_store" / "installed.json";
  if(sfs::exists(gog_installed))
  {
    auto gog_games = detectGamesFromStore("GOG", gog_installed, config_dir);
    games.insert(games.end(), gog_games.begin(), gog_games.end());
  }

  // Check Amazon store
  sfs::path amazon_installed = config_dir / "amazon_store" / "installed.json";
  if(sfs::exists(amazon_installed))
  {
    auto amazon_games = detectGamesFromStore("Amazon", amazon_installed, config_dir);
    games.insert(games.end(), amazon_games.begin(), amazon_games.end());
  }

  Log::debug(std::format("Detected {} Heroic games", games.size()));
  return games;
}

std::optional<HeroicGameInfo> HeroicDetector::getGameConfig(const std::string& app_name)
{
  sfs::path config_dir = getHeroicConfigDir();
  if(config_dir.empty() || !sfs::exists(config_dir))
  {
    return std::nullopt;
  }

  return parseGameConfig(app_name, config_dir);
}

sfs::path HeroicDetector::getHeroicConfigDir()
{
  // Check for Flatpak installation first
  sfs::path flatpak_config =
    sfs::path(std::getenv("HOME")) / ".var/app/com.heroicgameslauncher.hgl/config/heroic";
  if(sfs::exists(flatpak_config))
  {
    return flatpak_config;
  }

  // Check native installation
  sfs::path native_config = sfs::path(std::getenv("HOME")) / ".config/heroic";
  if(sfs::exists(native_config))
  {
    return native_config;
  }

  return sfs::path();
}

sfs::path HeroicDetector::getHeroicToolsDir()
{
  sfs::path config_dir = getHeroicConfigDir();
  if(config_dir.empty())
  {
    return sfs::path();
  }

  // For Flatpak, tools are in a different location
  if(config_dir.string().find(".var/app") != std::string::npos)
  {
    sfs::path flatpak_tools = sfs::path(std::getenv("HOME")) /
                             ".var/app/com.heroicgameslauncher.hgl/config/heroic/tools/proton";
    if(sfs::exists(flatpak_tools))
    {
      return flatpak_tools;
    }
  }

  // Native Heroic tools
  sfs::path tools_dir = config_dir / "tools/proton";
  if(sfs::exists(tools_dir))
  {
    return tools_dir;
  }

  return sfs::path();
}

std::optional<HeroicGameInfo> HeroicDetector::parseGameConfig(const std::string& app_name,
                                                              const sfs::path& config_dir)
{
  sfs::path game_config_path = config_dir / "GamesConfig" / (app_name + ".json");

  if(!sfs::exists(game_config_path))
  {
    Log::debug(std::format("Heroic game config not found: {}", game_config_path.string()));
    return std::nullopt;
  }

  try
  {
    Json::Value json;
    std::ifstream file(game_config_path, std::fstream::binary);
    if(!file.is_open())
    {
      Log::debug(std::format("Failed to open Heroic game config: {}", game_config_path.string()));
      return std::nullopt;
    }
    file >> json;
    file.close();

    HeroicGameInfo info;
    info.app_name = app_name;

    // Parse required fields
    if(!json.isMember("install_path"))
    {
      Log::debug(std::format("Heroic config missing install_path: {}", app_name));
      return std::nullopt;
    }
    info.install_path = json["install_path"].asString();

    if(!json.isMember("winePrefix"))
    {
      Log::debug(std::format("Heroic config missing winePrefix: {}", app_name));
      return std::nullopt;
    }
    info.wine_prefix = json["winePrefix"].asString();

    // Parse wine version info
    if(json.isMember("wineVersion") && json["wineVersion"].isMember("name"))
    {
      info.wine_version = json["wineVersion"]["name"].asString();
    }

    // Find Proton path
    if(!info.wine_version.empty())
    {
      sfs::path tools_dir = getHeroicToolsDir();
      info.proton_path = findProtonPath(info.wine_version, tools_dir);
    }

    return info;
  }
  catch(const Json::Exception& e)
  {
    Log::debug(
      std::format("Failed to parse Heroic game config {}: {}", app_name, e.what()));
    return std::nullopt;
  }
}

std::vector<HeroicGameInfo> HeroicDetector::detectGamesFromStore(const std::string& store_type,
                                                                 const sfs::path& installed_json_path,
                                                                 const sfs::path& config_dir)
{
  std::vector<HeroicGameInfo> games;

  try
  {
    Json::Value json;
    std::ifstream file(installed_json_path, std::fstream::binary);
    if(!file.is_open())
    {
      Log::debug(std::format("Failed to open {} installed games list", store_type));
      return games;
    }
    file >> json;
    file.close();

    if(!json.isArray())
    {
      Log::debug(std::format("{} installed games list is not an array", store_type));
      return games;
    }

    for(const auto& entry : json)
    {
      if(!entry.isMember("appName"))
      {
        continue;
      }

      std::string app_name = entry["appName"].asString();
      auto game_info = parseGameConfig(app_name, config_dir);

      if(game_info)
      {
        // Add title if available
        if(entry.isMember("title"))
        {
          game_info->title = entry["title"].asString();
        }
        games.push_back(*game_info);
      }
    }

    Log::debug(std::format("Detected {} games from {} store", games.size(), store_type));
  }
  catch(const Json::Exception& e)
  {
    Log::debug(std::format("Failed to parse {} installed games list: {}", store_type, e.what()));
  }

  return games;
}

sfs::path HeroicDetector::findProtonPath(const std::string& wine_version,
                                         const sfs::path& tools_dir)
{
  if(tools_dir.empty() || !sfs::exists(tools_dir))
  {
    return sfs::path();
  }

  // Try exact match first
  sfs::path exact_path = tools_dir / wine_version;
  if(sfs::exists(exact_path) && sfs::is_directory(exact_path))
  {
    return exact_path;
  }

  // Try to find a directory that contains the version name
  try
  {
    for(const auto& entry : sfs::directory_iterator(tools_dir))
    {
      if(entry.is_directory() && entry.path().filename().string().find(wine_version) !=
                                    std::string::npos)
      {
        return entry.path();
      }
    }
  }
  catch(const sfs::filesystem_error& e)
  {
    Log::debug(std::format("Error scanning Proton directory: {}", e.what()));
  }

  return sfs::path();
}

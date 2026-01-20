/*!
 * \file heroicdetector.h
 * \brief Header for Heroic Games Launcher detection and configuration parsing.
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace sfs = std::filesystem;

/*!
 * \brief Information about a game installed via Heroic.
 */
struct HeroicGameInfo
{
  /*! \brief Unique app name (e.g., "Croc" for Epic games). */
  std::string app_name;
  /*! \brief Human-readable game title. */
  std::string title;
  /*! \brief Path to the game installation directory. */
  sfs::path install_path;
  /*! \brief Path to the Wine prefix (contains pfx subdirectory). */
  sfs::path wine_prefix;
  /*! \brief Wine/Proton version string (e.g., "GE-Proton9-2"). */
  std::string wine_version;
  /*! \brief Path to the Proton installation directory. */
  sfs::path proton_path;
};

/*!
 * \brief Detects and retrieves information about games installed via Heroic.
 * Parses Heroic's JSON configuration files from ~/.config/heroic/ and
 * ~/.config/heroic/GamesConfig/.
 */
class HeroicDetector
{
public:
  /*!
   * \brief Checks if Heroic is installed on this system.
   * \return True if Heroic configuration files are found.
   */
  static bool isHeroicInstalled();

  /*!
   * \brief Detects all games installed via Heroic.
   * Scans Epic, GOG, and Amazon stores.
   * \return Vector of detected Heroic games, or empty vector if none found.
   */
  static std::vector<HeroicGameInfo> detectGames();

  /*!
   * \brief Retrieves configuration for a specific Heroic game.
   * \param app_name The Heroic app name.
   * \return Game configuration if found, std::nullopt otherwise.
   */
  static std::optional<HeroicGameInfo> getGameConfig(const std::string& app_name);

private:
  /*!
   * \brief Gets the Heroic config directory path.
   * Handles both native (~/.config/heroic) and Flatpak installations.
   * \return Path to Heroic config directory, or empty path if not found.
   */
  static sfs::path getHeroicConfigDir();

  /*!
   * \brief Gets the Heroic tools directory for Proton/Wine executables.
   * \return Path to tools directory.
   */
  static sfs::path getHeroicToolsDir();

  /*!
   * \brief Parses the GamesConfig JSON file for a specific game.
   * \param app_name The app name to parse config for.
   * \param config_dir The Heroic config directory.
   * \return Game info if successfully parsed, std::nullopt otherwise.
   */
  static std::optional<HeroicGameInfo> parseGameConfig(const std::string& app_name,
                                                       const sfs::path& config_dir);

  /*!
   * \brief Detects games from a specific store's installed.json.
   * \param store_type The store type ("Epic", "GOG", "Amazon").
   * \param installed_json_path Path to the installed.json file.
   * \param config_dir The Heroic config directory.
   * \return Vector of detected games from this store.
   */
  static std::vector<HeroicGameInfo> detectGamesFromStore(const std::string& store_type,
                                                          const sfs::path& installed_json_path,
                                                          const sfs::path& config_dir);

  /*!
   * \brief Finds the Proton installation path for the given version.
   * \param wine_version The version string (e.g., "GE-Proton9-2").
   * \param tools_dir The tools directory to search in.
   * \return Path to Proton directory if found, empty path otherwise.
   */
  static sfs::path findProtonPath(const std::string& wine_version, const sfs::path& tools_dir);
};

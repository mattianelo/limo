/*!
 * \file launcher.h
 * \brief Header for launcher abstraction classes.
 */

#pragma once

#include <filesystem>
#include <string>

namespace sfs = std::filesystem;

/*!
 * \brief Enum describing the launcher type.
 */
enum class LauncherType
{
  /*! \brief Game installed via Steam. */
  steam,
  /*! \brief Game installed via Heroic Games Launcher. */
  heroic
};

/*!
 * \brief Abstract base class for launcher-specific configurations.
 * Provides an interface for both Steam and Heroic launchers.
 */
class LauncherConfig
{
public:
  virtual ~LauncherConfig() = default;

  /*!
   * \brief Returns the launcher type.
   * \return The launcher type.
   */
  virtual LauncherType getType() const = 0;

  /*!
   * \brief Returns the unique identifier for the game.
   * For Steam: numeric app ID as string
   * For Heroic: app name
   * \return The identifier.
   */
  virtual std::string getIdentifier() const = 0;

  /*!
   * \brief Returns the installation path of the game.
   * \return The installation path.
   */
  virtual sfs::path getInstallPath() const = 0;

  /*!
   * \brief Returns the Wine prefix path.
   * For Steam: Proton prefix path
   * For Heroic: winePrefix path (containing pfx subdirectory)
   * \return The wine prefix path.
   */
  virtual sfs::path getWinePrefix() const = 0;

  /*!
   * \brief Returns the Wine/Proton version string.
   * For Steam: e.g., "Proton-9.0"
   * For Heroic: e.g., "GE-Proton9-2"
   * \return The version string.
   */
  virtual std::string getWineVersion() const = 0;

  /*!
   * \brief Returns the path to the Proton/Wine executable folder.
   * For Steam: path to the Proton installation directory
   * For Heroic: path to the Proton installation directory
   * \return The proton path.
   */
  virtual sfs::path getProtonPath() const = 0;

  /*!
   * \brief Returns whether this launcher requires special Flatpak handling.
   * \return True if running under Flatpak.
   */
  virtual bool isFlatpak() const = 0;
};

/*!
 * \brief Steam launcher configuration.
 */
class SteamLauncherConfig : public LauncherConfig
{
public:
  /*!
   * \brief Constructs a Steam launcher configuration.
   * \param steam_app_id The Steam app ID.
   * \param install_path Path to the game installation.
   * \param proton_prefix Path to the Proton prefix directory.
   */
  SteamLauncherConfig(int steam_app_id,
                      const sfs::path& install_path,
                      const sfs::path& proton_prefix);

  LauncherType getType() const override;
  std::string getIdentifier() const override;
  sfs::path getInstallPath() const override;
  sfs::path getWinePrefix() const override;
  std::string getWineVersion() const override;
  sfs::path getProtonPath() const override;
  bool isFlatpak() const override;

private:
  int steam_app_id_;
  sfs::path install_path_;
  sfs::path proton_prefix_;
};

/*!
 * \brief Heroic launcher configuration.
 */
class HeroicLauncherConfig : public LauncherConfig
{
public:
  /*!
   * \brief Constructs a Heroic launcher configuration.
   * \param app_name The Heroic app name (unique identifier).
   * \param install_path Path to the game installation.
   * \param wine_prefix Path to the Wine prefix (containing pfx subdirectory).
   * \param wine_version The Wine/Proton version string.
   * \param proton_path Path to the Proton installation.
   * \param is_flatpak Whether Heroic is running under Flatpak.
   */
  HeroicLauncherConfig(const std::string& app_name,
                       const sfs::path& install_path,
                       const sfs::path& wine_prefix,
                       const std::string& wine_version,
                       const sfs::path& proton_path,
                       bool is_flatpak = false);

  LauncherType getType() const override;
  std::string getIdentifier() const override;
  sfs::path getInstallPath() const override;
  sfs::path getWinePrefix() const override;
  std::string getWineVersion() const override;
  sfs::path getProtonPath() const override;
  bool isFlatpak() const override;

private:
  std::string app_name_;
  sfs::path install_path_;
  sfs::path wine_prefix_;
  std::string wine_version_;
  sfs::path proton_path_;
  bool is_flatpak_;
};

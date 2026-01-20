/*!
 * \file launcher.cpp
 * \brief Implementation of launcher abstraction classes.
 */

#include "launcher.h"

SteamLauncherConfig::SteamLauncherConfig(int steam_app_id,
                                         const sfs::path& install_path,
                                         const sfs::path& proton_prefix) :
  steam_app_id_(steam_app_id), install_path_(install_path), proton_prefix_(proton_prefix)
{
}

LauncherType SteamLauncherConfig::getType() const
{
  return LauncherType::steam;
}

std::string SteamLauncherConfig::getIdentifier() const
{
  return std::to_string(steam_app_id_);
}

sfs::path SteamLauncherConfig::getInstallPath() const
{
  return install_path_;
}

sfs::path SteamLauncherConfig::getWinePrefix() const
{
  return proton_prefix_;
}

std::string SteamLauncherConfig::getWineVersion() const
{
  return "steam";
}

sfs::path SteamLauncherConfig::getProtonPath() const
{
  return sfs::path();
}

bool SteamLauncherConfig::isFlatpak() const
{
  return false;
}

HeroicLauncherConfig::HeroicLauncherConfig(const std::string& app_name,
                                           const sfs::path& install_path,
                                           const sfs::path& wine_prefix,
                                           const std::string& wine_version,
                                           const sfs::path& proton_path,
                                           bool is_flatpak) :
  app_name_(app_name), install_path_(install_path), wine_prefix_(wine_prefix),
  wine_version_(wine_version), proton_path_(proton_path), is_flatpak_(is_flatpak)
{
}

LauncherType HeroicLauncherConfig::getType() const
{
  return LauncherType::heroic;
}

std::string HeroicLauncherConfig::getIdentifier() const
{
  return app_name_;
}

sfs::path HeroicLauncherConfig::getInstallPath() const
{
  return install_path_;
}

sfs::path HeroicLauncherConfig::getWinePrefix() const
{
  return wine_prefix_;
}

std::string HeroicLauncherConfig::getWineVersion() const
{
  return wine_version_;
}

sfs::path HeroicLauncherConfig::getProtonPath() const
{
  return proton_path_;
}

bool HeroicLauncherConfig::isFlatpak() const
{
  return is_flatpak_;
}

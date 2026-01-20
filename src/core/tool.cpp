#include "tool.h"
#include <format>
#include <ranges>

namespace sfs = std::filesystem;


Tool::Tool(const std::string& name, const sfs::path& icon_path, const std::string& command) :
  name_(name), icon_path_(icon_path), runtime_(native), command_overwrite_(command)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           const sfs::path& executable_path,
           const sfs::path& working_directory,
           const std::map<std::string, std::string>& environment_variables,
           const std::string& arguments) :
  name_(name), icon_path_(icon_path), executable_path_(executable_path), runtime_(native),
  working_directory_(working_directory), environment_variables_(environment_variables),
  arguments_(arguments)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           const sfs::path& executable_path,
           const sfs::path& prefix_path,
           const sfs::path& working_directory,
           const std::map<std::string, std::string>& environment_variables,
           const std::string& arguments) :
  name_(name), icon_path_(icon_path), executable_path_(executable_path), runtime_(wine),
  prefix_path_(prefix_path), working_directory_(working_directory),
  environment_variables_(environment_variables), arguments_(arguments)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           const sfs::path& executable_path,
           bool use_flatpak_protontricks,
           int steam_app_id,
           const sfs::path& working_directory,
           const std::map<std::string, std::string>& environment_variables,
           const std::string& arguments,
           const std::string& protontricks_arguments) :
  name_(name), icon_path_(icon_path), executable_path_(executable_path), runtime_(protontricks),
  use_flatpak_runtime_(use_flatpak_protontricks), steam_app_id_(steam_app_id),
  working_directory_(working_directory), environment_variables_(environment_variables),
  arguments_(arguments), protontricks_arguments_(protontricks_arguments)
{}

Tool::Tool(const std::string& name,
           const sfs::path& icon_path,
           int steam_app_id,
           bool use_flatpak_steam) :
  name_(name), icon_path_(icon_path), runtime_(steam), steam_app_id_(steam_app_id),
  use_flatpak_runtime_(use_flatpak_steam)
{}

Tool::Tool(const Json::Value& json_object)
{
  if(!json_object.isMember("use_flatpak_runtime")) // Initialize from old format
  {
    name_ = json_object["name"].asString();
    icon_path_ = json_object["icon_path"].asString();
    command_overwrite_ = json_object["command"].asString();
    
    // Parse additional fields that might be in old format
    if(json_object.isMember("executable_path"))
      executable_path_ = json_object["executable_path"].asString();
    if(json_object.isMember("working_directory"))
      working_directory_ = json_object["working_directory"].asString();
    if(json_object.isMember("arguments"))
      arguments_ = json_object["arguments"].asString();
    if(json_object.isMember("protontricks_arguments"))
      protontricks_arguments_ = json_object["protontricks_arguments"].asString();
    
    // Parse runtime - handle both string and int formats
    if(json_object.isMember("runtime"))
    {
      if(json_object["runtime"].isString())
      {
        std::string runtime_str = json_object["runtime"].asString();
        if(runtime_str == "protontricks") runtime_ = Runtime::protontricks;
        else if(runtime_str == "wine") runtime_ = Runtime::wine;
        else if(runtime_str == "steam") runtime_ = Runtime::steam;
        else runtime_ = Runtime::native;
      }
      else
      {
        runtime_ = static_cast<Runtime>(json_object["runtime"].asInt());
      }
    }
    
    // Parse launcher-specific fields for Heroic support (even in old format)
    if(json_object.isMember("launcher"))
    {
      // Handle string-based launcher field
      std::string launcher_str = json_object["launcher"].asString();
      if(launcher_str == "heroic")
      {
        launcher_type_ = LauncherType::heroic;
      }
      else
      {
        launcher_type_ = LauncherType::steam;
      }
    }
    if(json_object.isMember("launcher_type"))
    {
      launcher_type_ = static_cast<LauncherType>(json_object["launcher_type"].asInt());
    }
    if(json_object.isMember("appName"))
    {
      launcher_identifier_ = json_object["appName"].asString();
    }
    if(json_object.isMember("launcher_identifier"))
    {
      launcher_identifier_ = json_object["launcher_identifier"].asString();
    }
    if(json_object.isMember("proton_path"))
    {
      proton_path_ = json_object["proton_path"].asString();
    }
  }
  else
  {
    name_ = json_object["name"].asString();
    icon_path_ = json_object["icon_path"].asString();
    executable_path_ = json_object["executable_path"].asString();
    
    // Parse runtime - handle both string and int formats
    if(json_object["runtime"].isString())
    {
      std::string runtime_str = json_object["runtime"].asString();
      if(runtime_str == "protontricks") runtime_ = Runtime::protontricks;
      else if(runtime_str == "wine") runtime_ = Runtime::wine;
      else if(runtime_str == "steam") runtime_ = Runtime::steam;
      else runtime_ = Runtime::native;
    }
    else
    {
      runtime_ = static_cast<Runtime>(json_object["runtime"].asInt());
    }
    
    use_flatpak_runtime_ = json_object["use_flatpak_runtime"].asBool();
    prefix_path_ = json_object["prefix_path"].asString();
    steam_app_id_ = json_object["steam_app_id"].asInt();
    working_directory_ = json_object["working_directory"].asString();
    for(int i = 0; i < json_object["environment_variables"].size(); i++)
    {
      environment_variables_[json_object["environment_variables"][i]["variable"].asString()] =
        json_object["environment_variables"][i]["value"].asString();
    }
    arguments_ = json_object["arguments"].asString();
    protontricks_arguments_ = json_object["protontricks_arguments"].asString();
    command_overwrite_ = json_object["command"].asString();

    // Parse launcher-specific fields for Heroic support
    if(json_object.isMember("launcher"))
    {
      // Handle string-based launcher field
      std::string launcher_str = json_object["launcher"].asString();
      if(launcher_str == "heroic")
      {
        launcher_type_ = LauncherType::heroic;
      }
      else
      {
        launcher_type_ = LauncherType::steam;
      }
    }
    if(json_object.isMember("launcher_type"))
    {
      launcher_type_ = static_cast<LauncherType>(json_object["launcher_type"].asInt());
    }
    if(json_object.isMember("appName"))
    {
      launcher_identifier_ = json_object["appName"].asString();
    }
    if(json_object.isMember("launcher_identifier"))
    {
      launcher_identifier_ = json_object["launcher_identifier"].asString();
    }
    if(json_object.isMember("proton_path"))
    {
      proton_path_ = json_object["proton_path"].asString();
    }
  }
}

std::string Tool::getCommand(bool is_flatpak) const
{
  if(!command_overwrite_.empty())
    return is_flatpak ? "flatpak-spawn --host " + command_overwrite_
                      : command_overwrite_;

  std::string command;
  if(is_flatpak)
    command = "flatpak-spawn --host ";

  if(runtime_ == steam)
  {
    if(!use_flatpak_runtime_)
      command += "steam ";
    else
      command += "flatpak run com.valvesoftware.Steam ";
    return command + std::format("-applaunch {}", steam_app_id_);
  }

  // Heroic launcher support
  if(launcher_type_ == LauncherType::heroic && runtime_ == protontricks)
  {
    if(!working_directory_.empty())
    {
      if(is_flatpak)
        command += "--directory=" + encloseInQuotes(working_directory_.string());
      else
        command += "cd " + encloseInQuotes(working_directory_.string()) + "; ";
    }

    // Set Proton environment variables
    appendEnvironmentVariables(command,
                               { { "STEAM_COMPAT_DATA_PATH", prefix_path_.string() },
                                 { "STEAM_COMPAT_CLIENT_INSTALL_PATH", "/usr" } },
                               is_flatpak);
    appendEnvironmentVariables(command, environment_variables_, is_flatpak);

    if(!command.empty())
      command += " ";

    // Run through Proton
    command += encloseInQuotes(proton_path_.string() + "/proton") + " run ";
    command += encloseInQuotes(executable_path_);

    if(!arguments_.empty())
      command += " " + arguments_;

    return command;
  }

  if(!working_directory_.empty())
  {
    if(is_flatpak)
      command += "--directory=" + encloseInQuotes(working_directory_.string());
    else
      command += "cd " + encloseInQuotes(working_directory_.string()) + ";";
  }

  appendEnvironmentVariables(command, environment_variables_, is_flatpak);
  if(runtime_ == wine && !prefix_path_.empty())
    appendEnvironmentVariables(command, { { "WINEPREFIX", prefix_path_.string() } }, is_flatpak);

  if(!command.empty() && runtime_ != native)
    command += " ";
  if(runtime_ == wine)
    command += "wine";
  else if(runtime_ == protontricks)
  {
    if(use_flatpak_runtime_)
      command += "flatpak run --command=protontricks-launch com.github.Matoking.protontricks ";
    else
      command += "protontricks-launch ";
    command += "--appid " + std::to_string(steam_app_id_);
    if(!protontricks_arguments_.empty())
      command += " " + protontricks_arguments_;
  }

  if(!command.empty())
    command += " ";
  command += encloseInQuotes(executable_path_);

  if(!arguments_.empty())
    command += " " + arguments_;

  return command;
}

Json::Value Tool::toJson() const
{
  Json::Value json_object;
  json_object["name"] = name_;
  json_object["icon_path"] = icon_path_.string();
  json_object["executable_path"] = executable_path_.string();
  json_object["runtime"] = static_cast<int>(runtime_);
  json_object["use_flatpak_runtime"] = use_flatpak_runtime_;
  json_object["prefix_path"] = prefix_path_.string();
  json_object["steam_app_id"] = steam_app_id_;
  json_object["working_directory"] = working_directory_.string();
  for(const auto& [i, pair] : std::views::enumerate(environment_variables_))
  {
    const auto& [variable, value] = pair;
    json_object["environment_variables"][(int)i]["variable"] = variable;
    json_object["environment_variables"][(int)i]["value"] = value;
  }
  json_object["arguments"] = arguments_;
  json_object["protontricks_arguments"] = protontricks_arguments_;
  json_object["command"] = command_overwrite_;

  // Serialize launcher-specific fields
  json_object["launcher_type"] = static_cast<int>(launcher_type_);
  json_object["launcher_identifier"] = launcher_identifier_;
  json_object["proton_path"] = proton_path_.string();

  return json_object;
}

std::string Tool::getName() const
{
  return name_;
}

sfs::path Tool::getIconPath() const
{
  return icon_path_;
}

sfs::path Tool::getExecutablePath() const
{
  return executable_path_;
}

Tool::Runtime Tool::getRuntime() const
{
  return runtime_;
}

bool Tool::usesFlatpakRuntime() const
{
  return use_flatpak_runtime_;
}

sfs::path Tool::getPrefixPath() const
{
  return prefix_path_;
}

int Tool::getSteamAppId() const
{
  return steam_app_id_;
}

sfs::path Tool::getWorkingDirectory() const
{
  return working_directory_;
}

const std::map<std::string, std::string> Tool::getEnvironmentVariables() const
{
  return environment_variables_;
}

std::string Tool::getArguments() const
{
  return arguments_;
}

std::string Tool::getProtontricksArguments() const
{
  return protontricks_arguments_;
}

std::string Tool::getCommandOverwrite() const
{
  return command_overwrite_;
}

LauncherType Tool::getLauncherType() const
{
  return launcher_type_;
}

std::string Tool::getLauncherIdentifier() const
{
  return launcher_identifier_;
}

std::filesystem::path Tool::getProtonPath() const
{
  return proton_path_;
}

void Tool::appendEnvironmentVariables(
  std::string& command,
  const std::map<std::string, std::string>& environment_variables,
  bool is_flatpak) const
{
  for(const auto& [variable, value] : environment_variables)
  {
    if(!command.empty())
      command += " ";
    if(is_flatpak)
      command += "--env=";
    command += variable + "=" + encloseInQuotes(value);
  }
}

std::string Tool::encloseInQuotes(const std::string& string) const
{
  if(string.starts_with('"') && string.ends_with('"'))
    return string;

  return '"' + string + '"';
}

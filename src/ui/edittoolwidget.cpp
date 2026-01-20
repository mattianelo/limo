#include "edittoolwidget.h"
#include "importfromsteamdialog.h"
#include "tablepushbutton.h"
#include "core/launcher.h"
#include "core/heroicdetector.h"
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStandardPaths>
#include <filesystem>
#include <ranges>
#include <json/json.h>

namespace sfs = std::filesystem;


EditToolWidget::EditToolWidget(QWidget* parent) : QWidget{ parent }
{
  mode_label_ = new QLabel("Mode:", this);
  mode_box_ = new QComboBox(this);
  mode_box_->addItems({ "Guided", "Manual" });
  connect(mode_box_,
          qOverload<int>(&QComboBox::currentIndexChanged),
          this,
          &EditToolWidget::modeBoxIndexChanged);

  launcher_label_ = new QLabel("Launcher:", this);
  launcher_label_->setToolTip("Which launcher hosts the game or application");
  launcher_box_ = new QComboBox(this);
  launcher_box_->addItems({ "Steam", "Heroic" });
  connect(launcher_box_,
          qOverload<int>(&QComboBox::currentIndexChanged),
          this,
          &EditToolWidget::launcherBoxIndexChanged);

  name_label_ = new QLabel("Name:", this);
  name_label_->setToolTip("The name of the tool");
  name_field_ = new ValidatingLineEdit(this);
  connect(name_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);

  icon_label_ = new QLabel("Icon:", this);
  icon_label_->setToolTip("The icon used to represent the tool");
  icon_field_ = new ValidatingLineEdit(this, ValidatingLineEdit::VALID_IS_EXISTING_FILE);
  icon_field_->setAcceptsEmptyPaths(true);
  connect(icon_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);
  icon_picker_ = new QPushButton(this);
  icon_picker_->setIcon(QIcon::fromTheme("folder-open"));
  connect(icon_picker_, &QPushButton::clicked, this, &EditToolWidget::iconPickerClicked);

  executable_label_ = new QLabel("Tool executable:", this);
  executable_label_->setToolTip("Path to the executable");
  executable_field_ = new ValidatingLineEdit(this, ValidatingLineEdit::VALID_NOT_EMPTY);
  connect(executable_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);
  executable_picker_ = new QPushButton(this);
  executable_picker_->setIcon(QIcon::fromTheme("folder-open"));
  connect(
    executable_picker_, &QPushButton::clicked, this, &EditToolWidget::executablePickerClicked);

  runtime_label_ = new QLabel("Runtime:", this);
  runtime_label_->setToolTip("How to run the executable");
  runtime_box_ = new QComboBox(this);
  runtime_box_->addItems({ "Native", "Wine", "Protontricks", "Steam" });
  runtime_box_->setItemData(0, "Run the tool as a native command", Qt::ToolTipRole);
  runtime_box_->setItemData(1, "Run the tool through wine", Qt::ToolTipRole);
  runtime_box_->setItemData(2, "Run the tool through Protontricks", Qt::ToolTipRole);
  runtime_box_->setItemData(3, "Run a Steam app", Qt::ToolTipRole);
  connect(runtime_box_,
          qOverload<int>(&QComboBox::currentIndexChanged),
          this,
          &EditToolWidget::runtimeBoxIndexChanged);

  runtime_version_label_ = new QLabel("Version:");
  runtime_version_label_->setToolTip("Which runtime version of to use");
  runtime_version_box_ = new QComboBox(this);
  runtime_version_box_->addItems({ "Native", "Flatpak" });

  prefix_label_ = new QLabel("Wine prefix:", this);
  prefix_label_->setToolTip("Path to the wine prefix to use. Leave empty to use the system prefix");
  prefix_field_ = new ValidatingLineEdit(this, ValidatingLineEdit::VALID_IS_EXISTING_DIRECTORY);
  prefix_field_->setAcceptsEmptyPaths(true);
  connect(prefix_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);
  prefix_picker_ = new QPushButton(this);
  prefix_picker_->setIcon(QIcon::fromTheme("folder-open"));
  connect(prefix_picker_, &QPushButton::clicked, this, &EditToolWidget::prefixPickerClicked);

  app_id_label_ = new QLabel("Steam App ID:", this);
  app_id_label_->setToolTip("Steam app ID for the Proton prefix");
  app_id_field_ = new ValidatingLineEdit(this);
  app_id_field_->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]*"), this));
  connect(app_id_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);
  app_id_import_button_ = new QPushButton("Import", this);
  connect(app_id_import_button_, &QPushButton::clicked, this, &EditToolWidget::importButtonClicked);
  import_dialog_ = new ImportFromSteamDialog(this);
  connect(import_dialog_,
          &ImportFromSteamDialog::applicationImported,
          this,
          &EditToolWidget::steamAppImported);

  app_name_label_ = new QLabel("Heroic App Name:", this);
  app_name_label_->setToolTip("Name of the game in Heroic Games Launcher");
  app_name_field_ = new QComboBox(this);
  app_name_import_button_ = new QPushButton("Detect Games", this);
  connect(app_name_import_button_, &QPushButton::clicked, this, &EditToolWidget::detectHeroicGamesClicked);

  working_directory_label_ = new QLabel("Working directory:", this);
  working_directory_label_->setToolTip("Working directory in which to run the executable");
  working_directory_field_ =
    new ValidatingLineEdit(this, ValidatingLineEdit::VALID_IS_EXISTING_DIRECTORY);
  working_directory_field_->setAcceptsEmptyPaths(true);
  connect(
    working_directory_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);
  working_directory_picker_ = new QPushButton(this);
  working_directory_picker_->setIcon(QIcon::fromTheme("folder-open"));
  connect(working_directory_picker_,
          &QPushButton::clicked,
          this,
          &EditToolWidget::workingDirPickerClicked);

  environment_label_ = new QLabel("Environment variables:", this);
  environment_label_->setToolTip("These environment variables will be set for the tool");
  environment_table_ = new QTableWidget(this);
  environment_table_->setColumnCount(3);
  environment_table_->setHorizontalHeaderLabels({ "Action", "Variable", "Value" });
  environment_table_->horizontalHeader()->setStretchLastSection(true);
  environment_table_->verticalHeader()->setVisible(false);
  connect(environment_table_,
          &QTableWidget::cellChanged,
          this,
          &EditToolWidget::environmentTableCellChanged);
  environment_table_->setColumnWidth(ENVIRONMENT_ACTION_COL, 55);
  environment_table_->setColumnWidth(ENVIRONMENT_VARIABLE_COL, 170);
  environment_table_->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

  arguments_label_ = new QLabel("Arguments:", this);
  arguments_label_->setToolTip("Arguments to pass to the executable");
  arguments_field_ = new QLineEdit(this);

  protontricks_arguments_label_ = new QLabel("Protontricks arguments:", this);
  protontricks_arguments_label_->setToolTip("Arguments to pass to protontricks-launch");
  protontricks_arguments_field_ = new QLineEdit(this);

  command_label_ = new QLabel("Command:", this);
  command_label_->setToolTip("Command to run");
  command_field_ = new ValidatingLineEdit(this);
  connect(command_field_, &QLineEdit::textChanged, this, &EditToolWidget::textFieldEdited);


  auto layout = new QGridLayout(this);
  layout->addWidget(mode_label_, 0, 0);
  layout->addWidget(mode_box_, 0, 1, 1, 3);

  layout->addWidget(launcher_label_, 1, 0);
  layout->addWidget(launcher_box_, 1, 1, 1, 3);

  layout->addWidget(name_label_, 2, 0);
  layout->addWidget(name_field_, 2, 1, 1, 3);

  layout->addWidget(icon_label_, 3, 0);
  layout->addWidget(icon_field_, 3, 1, 1, 2);
  layout->addWidget(icon_picker_, 3, 3);

  layout->addWidget(runtime_label_, 4, 0);
  layout->addWidget(runtime_box_, 4, 1, 1, 3);

  layout->addWidget(runtime_version_label_, 5, 0);
  layout->addWidget(runtime_version_box_, 5, 1, 1, 3);

  layout->addWidget(executable_label_, 6, 0);
  layout->addWidget(executable_field_, 6, 1, 1, 2);
  layout->addWidget(executable_picker_, 6, 3);

  layout->addWidget(prefix_label_, 7, 0);
  layout->addWidget(prefix_field_, 7, 1, 1, 2);
  layout->addWidget(prefix_picker_, 7, 3);

  layout->addWidget(app_id_label_, 8, 0);
  layout->addWidget(app_id_field_, 8, 1);
  layout->addWidget(app_id_import_button_, 8, 2, 1, 2);

  layout->addWidget(app_name_label_, 9, 0);
  layout->addWidget(app_name_field_, 9, 1);
  layout->addWidget(app_name_import_button_, 9, 2, 1, 2);

  layout->addWidget(working_directory_label_, 10, 0);
  layout->addWidget(working_directory_field_, 10, 1, 1, 2);
  layout->addWidget(working_directory_picker_, 10, 3);

  layout->addWidget(environment_label_, 11, 0);

  layout->addWidget(environment_table_, 12, 0, 1, 4);

  layout->addWidget(arguments_label_, 13, 0);
  layout->addWidget(arguments_field_, 13, 1, 1, 3);

  layout->addWidget(protontricks_arguments_label_, 14, 0);
  layout->addWidget(protontricks_arguments_field_, 14, 1, 1, 3);

  layout->addWidget(command_label_, 15, 0);
  layout->addWidget(command_field_, 15, 1, 1, 3);

  layout->addItem(
    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 16, 0, 3, 1);
  layout->setColumnStretch(1, 1);

  setLayout(layout);
  updateEnvironmentTable();
  updateChildrenVisibility();
}

Tool EditToolWidget::getTool()
{
  if(mode_box_->currentIndex() == MODE_MANUAL_INDEX)
    return { name_field_->text().toStdString(),
             icon_field_->text().toStdString(),
             command_field_->text().toStdString() };

  const int runtime = runtime_box_->currentIndex();
  const bool is_heroic = launcher_box_->currentIndex() == LAUNCHER_HEROIC_INDEX;
  
  if(runtime == RUNTIME_STEAM_INDEX)
    return { name_field_->text().toStdString(),
             icon_field_->text().toStdString(),
             app_id_field_->text().toInt(),
             runtime_version_box_->currentIndex() == VERSION_FLATPAK_INDEX };

  std::map<std::string, std::string> variable_map;
  for(const auto& [variable, value] : environment_variables)
  {
    if(!variable.isEmpty())
      variable_map[variable.toStdString()] = value.toStdString();
  }

  if(runtime == RUNTIME_NATIVE_INDEX)
    return { name_field_->text().toStdString(),
             icon_field_->text().toStdString(),
             executable_field_->text().toStdString(),
             working_directory_field_->text().toStdString(),
             variable_map,
             arguments_field_->text().toStdString() };
  if(runtime == RUNTIME_WINE_INDEX)
    return { name_field_->text().toStdString(),
             icon_field_->text().toStdString(),
             executable_field_->text().toStdString(),
             prefix_field_->text().toStdString(),
             working_directory_field_->text().toStdString(),
             variable_map,
             arguments_field_->text().toStdString() };
  
  // runtime == RUNTIME_PROTONTRICKS_INDEX
  if(is_heroic)
  {
    // For Heroic launcher with Protontricks
    auto app_name = app_name_field_->currentData(Qt::UserRole).toString().toStdString();
    auto heroic_game = HeroicDetector::getGameConfig(app_name);
    
    if(heroic_game)
    {
      // Build a JSON representation to construct the tool properly
      Json::Value json;
      json["name"] = name_field_->text().toStdString();
      json["icon_path"] = icon_field_->text().toStdString();
      json["executable_path"] = executable_field_->text().toStdString();
      json["runtime"] = "protontricks";
      json["launcher"] = "heroic";
      json["appName"] = app_name;
      json["working_directory"] = working_directory_field_->text().toStdString();
      json["arguments"] = arguments_field_->text().toStdString();
      json["protontricks_arguments"] = protontricks_arguments_field_->text().toStdString();
      
      // Add environment variables
      Json::Value env_json(Json::arrayValue);
      for(const auto& [variable, value] : environment_variables)
      {
        if(!variable.isEmpty())
        {
          Json::Value var_obj;
          var_obj["variable"] = variable.toStdString();
          var_obj["value"] = value.toStdString();
          env_json.append(var_obj);
        }
      }
      json["environment_variables"] = env_json;
      
      return Tool(json);
    }
  }
  
  // Fall back to Steam protontricks
  return { name_field_->text().toStdString(),
           icon_field_->text().toStdString(),
           executable_field_->text().toStdString(),
           runtime_version_box_->currentIndex() == VERSION_FLATPAK_INDEX,
           app_id_field_->text().toInt(),
           working_directory_field_->text().toStdString(),
           variable_map,
           arguments_field_->text().toStdString(),
           protontricks_arguments_field_->text().toStdString() };
}

bool EditToolWidget::hasValidInput() const
{
  return name_field_->hasValidText() && icon_field_->hasValidText() &&
         executable_field_->hasValidText() && prefix_field_->hasValidText() &&
         app_id_field_->hasValidText() && working_directory_field_->hasValidText() &&
         command_field_->hasValidText();
}

void EditToolWidget::init()
{
  mode_box_->setCurrentIndex(MODE_GUIDED_INDEX);
  name_field_->clear();
  icon_field_->clear();
  icon_picker_->setIcon(QIcon::fromTheme("folder-open"));
  executable_field_->clear();
  runtime_box_->setCurrentIndex(RUNTIME_NATIVE_INDEX);
  runtime_version_box_->setCurrentIndex(VERSION_NATIVE_INDEX);
  prefix_field_->clear();
  app_id_field_->clear();
  working_directory_field_->clear();
  environment_variables.clear();
  updateEnvironmentTable();
  arguments_field_->clear();
  protontricks_arguments_field_->clear();
  arguments_field_->clear();
  command_field_->clear();
  emit inputValidityChanged(has_valid_input_);
}

void EditToolWidget::init(const Tool& tool)
{
  init();
  name_field_->setText(tool.getName().c_str());
  icon_field_->setText(tool.getIconPath().c_str());
  if(!icon_field_->text().isEmpty())
  {
    const QIcon icon(icon_field_->text());
    if(icon.availableSizes().size() > 0)
      icon_picker_->setIcon(icon);
  }
  
  // Set launcher type
  int launcher_idx = (tool.getLauncherType() == LauncherType::heroic) ?
                     LAUNCHER_HEROIC_INDEX : LAUNCHER_STEAM_INDEX;
  launcher_box_->setCurrentIndex(launcher_idx);
  
  if(tool.getCommandOverwrite().empty())
  {
    mode_box_->setCurrentIndex(MODE_GUIDED_INDEX);
    if(tool.getRuntime() == Tool::native)
      runtime_box_->setCurrentIndex(RUNTIME_NATIVE_INDEX);
    else if(tool.getRuntime() == Tool::wine)
      runtime_box_->setCurrentIndex(RUNTIME_WINE_INDEX);
    else if(tool.getRuntime() == Tool::protontricks)
      runtime_box_->setCurrentIndex(RUNTIME_PROTONTRICKS_INDEX);
    else if(tool.getRuntime() == Tool::steam)
      runtime_box_->setCurrentIndex(RUNTIME_STEAM_INDEX);

    if(!executable_field_->isHidden())
      executable_field_->setText(tool.getExecutablePath().c_str());
    if(!runtime_version_box_->isHidden())
      runtime_version_box_->setCurrentIndex(tool.usesFlatpakRuntime() ? VERSION_FLATPAK_INDEX
                                                                      : VERSION_NATIVE_INDEX);
    if(!prefix_field_->isHidden())
      prefix_field_->setText(tool.getPrefixPath().c_str());
    if(!app_id_field_->isHidden() && launcher_idx == LAUNCHER_STEAM_INDEX)
      app_id_field_->setText(QString::number(tool.getSteamAppId()));
    
    // Set Heroic app name if applicable
    if(!app_name_field_->isHidden() && launcher_idx == LAUNCHER_HEROIC_INDEX)
    {
      auto app_name = tool.getLauncherIdentifier();
      int idx = app_name_field_->findData(QString::fromStdString(app_name));
      if(idx >= 0)
      {
        app_name_field_->setCurrentIndex(idx);
      }
    }
    
    if(!working_directory_field_->isHidden())
      working_directory_field_->setText(tool.getWorkingDirectory().c_str());
    if(!environment_table_->isHidden())
    {
      for(const auto& [variable, value] : tool.getEnvironmentVariables())
        environment_variables.emplace_back(variable.c_str(), value.c_str());
      updateEnvironmentTable();
    }
    if(!arguments_field_->isHidden())
      arguments_field_->setText(tool.getArguments().c_str());
    if(!protontricks_arguments_field_->isHidden())
      protontricks_arguments_field_->setText(tool.getProtontricksArguments().c_str());
  }
  else
  {
    mode_box_->setCurrentIndex(MODE_MANUAL_INDEX);
    command_field_->setText(tool.getCommandOverwrite().c_str());
  }
  emit inputValidityChanged(has_valid_input_);
}

void EditToolWidget::updateChildrenVisibility()
{
  const bool command_only = mode_box_->currentIndex() == MODE_MANUAL_INDEX;
  const int runtime = runtime_box_->currentIndex();

  runtime_label_->setVisible(!command_only);
  runtime_box_->setVisible(!command_only);

  const bool executable_visible = runtime != RUNTIME_STEAM_INDEX && !command_only;
  executable_label_->setVisible(executable_visible);
  executable_field_->setVisible(executable_visible);
  executable_picker_->setVisible(executable_visible);

  const bool flatpak_runtime_visible =
    (runtime == RUNTIME_STEAM_INDEX || runtime == RUNTIME_PROTONTRICKS_INDEX) && !command_only;
  runtime_version_label_->setVisible(flatpak_runtime_visible);
  if(runtime == RUNTIME_STEAM_INDEX)
  {
    runtime_version_label_->setText("Steam version:");
    runtime_version_label_->setToolTip("Which Steam version to use");
  }
  else if(runtime == RUNTIME_PROTONTRICKS_INDEX)
  {
    runtime_version_label_->setText("Protontricks version:");
    runtime_version_label_->setToolTip("Which Protontricks version to use");
  }
  runtime_version_box_->setVisible(flatpak_runtime_visible);

  const bool prefix_visible = runtime == RUNTIME_WINE_INDEX && !command_only;
  prefix_label_->setVisible(prefix_visible);
  prefix_field_->setVisible(prefix_visible);
  prefix_picker_->setVisible(prefix_visible);

  const bool app_id_visible =
    (runtime == RUNTIME_PROTONTRICKS_INDEX || runtime == RUNTIME_STEAM_INDEX) && !command_only;
  
  const bool is_heroic = launcher_box_->currentIndex() == LAUNCHER_HEROIC_INDEX;
  const bool steam_app_id_visible = app_id_visible && !is_heroic;
  app_id_label_->setVisible(steam_app_id_visible);
  app_id_field_->setVisible(steam_app_id_visible);
  app_id_import_button_->setVisible(steam_app_id_visible);
  
  const bool app_name_visible = app_id_visible && is_heroic && runtime == RUNTIME_PROTONTRICKS_INDEX;
  app_name_label_->setVisible(app_name_visible);
  app_name_field_->setVisible(app_name_visible);
  app_name_import_button_->setVisible(app_name_visible);
  
  if(steam_app_id_visible)
  {
    app_id_label_->setText("Steam App ID:");
    app_id_field_->setToolTip(runtime == RUNTIME_PROTONTRICKS_INDEX
                                ? "Steam app ID for the proton prefix"
                                : "Steam app ID to be run");
  }

  const bool working_dir_visible = runtime != RUNTIME_STEAM_INDEX && !command_only;
  working_directory_label_->setVisible(working_dir_visible);
  working_directory_field_->setVisible(working_dir_visible);
  working_directory_picker_->setVisible(working_dir_visible);

  const bool environment_visible = runtime != RUNTIME_STEAM_INDEX && !command_only;
  environment_label_->setVisible(environment_visible);
  environment_table_->setVisible(environment_visible);

  const bool arguments_visible = runtime != RUNTIME_STEAM_INDEX && !command_only;
  arguments_label_->setVisible(arguments_visible);
  arguments_field_->setVisible(arguments_visible);

  const bool protontricks_args_visible = runtime == RUNTIME_PROTONTRICKS_INDEX && !command_only;
  protontricks_arguments_label_->setVisible(protontricks_args_visible);
  protontricks_arguments_field_->setVisible(protontricks_args_visible);

  command_label_->setVisible(command_only);
  command_field_->setVisible(command_only);
}

void EditToolWidget::runFileDialog(QLineEdit* target_field,
                                   const QString& title,
                                   bool directories_only)
{
  QString starting_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString path = target_field->text();
  if(!path.isEmpty() && sfs::exists(path.toStdString()))
    starting_dir = sfs::path(path.toStdString()).parent_path().string().c_str();
  auto dialog = new QFileDialog;
  dialog->setWindowTitle(title);
  if(directories_only)
  {
    dialog->setOption(QFileDialog::ShowDirsOnly, true);
    dialog->setFileMode(QFileDialog::Directory);
  }
  else
    dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setDirectory(starting_dir);
  dialog->exec();
  QStringList file_names = dialog->selectedFiles();
  if(!file_names.isEmpty() && sfs::exists(file_names.front().toStdString()))
    target_field->setText(file_names.front());
}

void EditToolWidget::updateEnvironmentTable()
{
  environment_table_->blockSignals(true);
  environment_table_->setRowCount(0);
  environment_table_->setRowCount(environment_variables.size() + 1);
  for(const auto& [row, pair] : std::views::enumerate(environment_variables))
  {
    const auto& [variable, value] = pair;
    auto remove_button = new TablePushButton(row, ENVIRONMENT_ACTION_COL);
    remove_button->setIcon(QIcon::fromTheme("user-trash"));
    remove_button->setToolTip("Remove condition");
    remove_button->adjustSize();
    connect(remove_button,
            &TablePushButton::clickedAt,
            this,
            &EditToolWidget::environmentVariableRemoved);
    environment_table_->setCellWidget(row, ENVIRONMENT_ACTION_COL, remove_button);

    auto variable_item = new QTableWidgetItem(variable);
    variable_item->setFlags(variable_item->flags() | Qt::ItemIsEditable);
    environment_table_->setItem(row, ENVIRONMENT_VARIABLE_COL, variable_item);

    auto value_item = new QTableWidgetItem(value);
    value_item->setFlags(value_item->flags() | Qt::ItemIsEditable);
    environment_table_->setItem(row, ENVIRONMENT_VALUE_COL, value_item);
  }

  auto add_variable_button = new QPushButton();
  add_variable_button->setIcon(QIcon::fromTheme("list-add"));
  add_variable_button->setToolTip("Add variable");
  add_variable_button->adjustSize();
  connect(
    add_variable_button, &QPushButton::clicked, this, &EditToolWidget::environmentVariableAdded);
  environment_table_->setCellWidget(
    environment_table_->rowCount() - 1, ENVIRONMENT_ACTION_COL, add_variable_button);
  for(int col = 1; col < environment_table_->columnCount(); col++)
  {
    auto dummy_item = new QTableWidgetItem();
    dummy_item->setFlags(dummy_item->flags() & ~Qt::ItemIsEditable);
    environment_table_->setItem(environment_table_->rowCount() - 1, col, dummy_item);
  }

  environment_table_->blockSignals(false);
}

void EditToolWidget::modeBoxIndexChanged(int index)
{
  updateChildrenVisibility();
  textFieldEdited("");
}

void EditToolWidget::runtimeBoxIndexChanged(int index)
{
  updateChildrenVisibility();
  textFieldEdited("");
}

void EditToolWidget::executablePickerClicked()
{
  runFileDialog(executable_field_, "Select Executable", false);
}

void EditToolWidget::prefixPickerClicked()
{
  runFileDialog(prefix_field_, "Select Wine Prefix", true);
}

void EditToolWidget::workingDirPickerClicked()
{
  runFileDialog(working_directory_field_, "Select Working Directory", true);
}

void EditToolWidget::iconPickerClicked()
{
  runFileDialog(icon_field_, "Select Icon", false);
  if(!icon_field_->text().isEmpty())
  {
    const QIcon icon(icon_field_->text());
    icon_picker_->setIcon(icon.availableSizes().size() > 0 ? icon
                                                           : QIcon::fromTheme("folder-open"));
  }
}

void EditToolWidget::environmentVariableRemoved(int row, int col)
{
  if(row >= 0 && row < environment_variables.size())
    environment_variables.erase(environment_variables.begin() + row);
  updateEnvironmentTable();
}

void EditToolWidget::environmentVariableAdded()
{
  environment_variables.push_back({});
  updateEnvironmentTable();
}

void EditToolWidget::environmentTableCellChanged(int row, int col)
{
  if(row < 0 || row >= environment_variables.size() || col == ENVIRONMENT_ACTION_COL)
    return;

  auto& [variable, value] = environment_variables[row];
  if(col == ENVIRONMENT_VARIABLE_COL)
    variable = environment_table_->item(row, col)->text();
  else
    value = environment_table_->item(row, col)->text();
}

void EditToolWidget::textFieldEdited(QString new_text)
{
  const bool current_input_is_valid = hasValidInput();
  if(current_input_is_valid != has_valid_input_)
    emit inputValidityChanged(current_input_is_valid);
  has_valid_input_ = current_input_is_valid;
}

void EditToolWidget::importButtonClicked()
{
  import_dialog_->init();
  import_dialog_->show();
}

void EditToolWidget::steamAppImported(QString name,
                                      QString app_id,
                                      QString install_dir,
                                      QString prefix_path,
                                      QString icon_path)
{
  bool is_int;
  app_id.toInt(&is_int);
  if(is_int)
  {
    app_id_field_->setText(app_id);
    if(runtime_box_->currentIndex() == RUNTIME_STEAM_INDEX && icon_field_->text().isEmpty() &&
       sfs::exists(icon_path.toStdString()))
    {
      icon_field_->setText(icon_path);
      const QIcon icon(icon_path);
      icon_picker_->setIcon(icon.availableSizes().size() > 0 ? icon
                                                             : QIcon::fromTheme("folder-open"));
    }
  }
}

void EditToolWidget::launcherBoxIndexChanged(int index)
{
  if(index == LAUNCHER_HEROIC_INDEX && HeroicDetector::isHeroicInstalled())
  {
    detectHeroicGamesClicked();
  }
  updateChildrenVisibility();
}

void EditToolWidget::detectHeroicGamesClicked()
{
  if(!HeroicDetector::isHeroicInstalled())
  {
    QMessageBox::warning(this, "Not Found", "Heroic Games Launcher not found on this system");
    return;
  }

  auto games = HeroicDetector::detectGames();
  app_name_field_->blockSignals(true);
  app_name_field_->clear();

  for(const auto& game : games)
  {
    app_name_field_->addItem(QString::fromStdString(game.title),
                             QString::fromStdString(game.app_name));
  }

  app_name_field_->blockSignals(false);

  if(games.empty())
  {
    QMessageBox::information(this, "No Games", "No games found in Heroic Games Launcher");
  }
}

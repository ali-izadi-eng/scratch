#include "Serialization.h"
#include "Project.h"
#include "core/Logger.h"
#include "core/FileUtil.h"

#include <nlohmann/json.hpp>
#include <algorithm>
using nlohmann::json;

// -------------------- Costume / Sound --------------------

static json costumeToJson(const Costume& c) {
  return json{
    {"name", c.name},
    {"imagePath", c.imagePath},
    {"width", c.width},
    {"height", c.height}
  };
}

static Costume costumeFromJson(const json& j) {
  Costume c;
  c.name = j.value("name", "");
  c.imagePath = j.value("imagePath", "");
  c.width = j.value("width", 0);
  c.height = j.value("height", 0);
  return c;
}

static json soundToJson(const Sound& s) {
  return json{
    {"name", s.name},
    {"filePath", s.filePath},
    {"volume", s.volume},
    {"muted", s.muted}
  };
}

static Sound soundFromJson(const json& j) {
  Sound s;
  s.name = j.value("name", "");
  s.filePath = j.value("filePath", "");
  s.volume = j.value("volume", 1.0f);
  s.muted = j.value("muted", false);
  return s;
}

// -------------------- Stage --------------------

static json stageToJson(const Stage& st) {
  json jb = json::array();
  for (const auto& b : st.backdrops) jb.push_back(costumeToJson(b));
  return json{
    {"currentBackdrop", st.currentBackdrop},
    {"backdrops", jb}
  };
}

static Stage stageFromJson(const json& j) {
  Stage st;
  st.currentBackdrop = j.value("currentBackdrop", 0);
  if (j.contains("backdrops") && j["backdrops"].is_array()) {
    for (auto& it : j["backdrops"]) st.backdrops.push_back(costumeFromJson(it));
  }
  return st;
}

// -------------------- Blocks / Scripts --------------------

static json blockToJson(const Block& b) {
  json jArgs = json::array();
  for (const auto& a : b.args) jArgs.push_back(a);

  return json{
    {"id", b.id},
    {"type", (int)b.type},
    {"x", b.x},
    {"y", b.y},
    {"args", jArgs},
    {"nextId", b.nextId},
    {"childHeadId", b.childHeadId}
  };
}

static Block blockFromJson(const json& j) {
  Block b;
  b.id = j.value("id", 0);
  b.type = (BlockType)j.value("type", 0);
  b.x = j.value("x", 0.0f);
  b.y = j.value("y", 0.0f);
  b.nextId = j.value("nextId", -1);
  b.childHeadId = j.value("childHeadId", -1);

  if (j.contains("args") && j["args"].is_array()) {
    for (auto& a : j["args"]) {
      b.args.push_back(a.get<std::string>());
    }
  }
  return b;
}

static json scriptToJson(const Script& sc) {
  return json{
    {"id", sc.id},
    {"headBlockId", sc.headBlockId}
  };
}

static Script scriptFromJson(const json& j) {
  Script sc;
  sc.id = j.value("id", 0);
  sc.headBlockId = j.value("headBlockId", -1);
  return sc;
}

// -------------------- Sprite --------------------

static json spriteToJson(const Sprite& sp) {
  json jc = json::array();
  for (const auto& c : sp.costumes) jc.push_back(costumeToJson(c));

  json js = json::array();
  for (const auto& snd : sp.sounds) js.push_back(soundToJson(snd));

  // scripts
  json jScripts = json::array();
  for (const auto& sc : sp.scripts) jScripts.push_back(scriptToJson(sc));

  // blocks (unordered_map -> array)
  json jBlocks = json::array();
  for (const auto& [id, b] : sp.blocks) {
    (void)id;
    jBlocks.push_back(blockToJson(b));
  }

  return json{
    {"id", sp.id},
    {"name", sp.name},
    {"x", sp.x},
    {"y", sp.y},
    {"directionDeg", sp.directionDeg},
    {"sizePercent", sp.sizePercent},
    {"visible", sp.visible},
    {"currentCostume", sp.currentCostume},
    {"costumes", jc},
    {"sounds", js},
    {"soundVolume", sp.soundVolume},
    {"soundPitch", sp.soundPitch},
    {"colorEffect", sp.colorEffect},

    {"selectedScriptId", sp.selectedScriptId},
    {"scripts", jScripts},
    {"blocks", jBlocks}
  };
}

static Sprite spriteFromJson(const json& j) {
  Sprite sp;
  sp.id = j.value("id", 0);
  sp.name = j.value("name", "Sprite");
  sp.x = j.value("x", 0.0f);
  sp.y = j.value("y", 0.0f);
  sp.directionDeg = j.value("directionDeg", 90.0f);
  sp.sizePercent = j.value("sizePercent", 100.0f);
  sp.visible = j.value("visible", true);
  sp.currentCostume = j.value("currentCostume", 0);
  sp.colorEffect = j.value("colorEffect", 0.0f);
  sp.soundVolume = j.value("soundVolume", 100.0f);
  sp.soundPitch  = j.value("soundPitch", 0.0f);

  if (j.contains("costumes") && j["costumes"].is_array()) {
    for (auto& it : j["costumes"]) sp.costumes.push_back(costumeFromJson(it));
  }

  if (j.contains("sounds") && j["sounds"].is_array()) {
    for (auto& it : j["sounds"]) sp.sounds.push_back(soundFromJson(it));
  }

  sp.selectedScriptId = j.value("selectedScriptId", 0);

  if (j.contains("scripts") && j["scripts"].is_array()) {
    for (auto& it : j["scripts"]) sp.scripts.push_back(scriptFromJson(it));
  }

  if (j.contains("blocks") && j["blocks"].is_array()) {
    for (auto& it : j["blocks"]) {
      Block b = blockFromJson(it);
      if (b.id != 0) sp.blocks[b.id] = std::move(b);
    }
  }

  // اگر selectedScriptId معتبر نبود، درستش کن
  if (sp.selectedScriptId != 0) {
    bool ok = false;
    for (auto& sc : sp.scripts) if (sc.id == sp.selectedScriptId) ok = true;
    if (!ok) sp.selectedScriptId = sp.scripts.empty() ? 0 : sp.scripts.front().id;
  } else {
    sp.selectedScriptId = sp.scripts.empty() ? 0 : sp.scripts.front().id;
  }

  return sp;
}

// -------------------- API --------------------

namespace Serialization {

bool saveToFile(const Project& project, const std::string& path, std::string* err) {
  try {
    json root;
    root["version"] = 3;
    root["stage"] = stageToJson(project.stage());

    json arr = json::array();
    for (const auto& sp : project.sprites()) arr.push_back(spriteToJson(sp));
    root["sprites"] = arr;

    std::string text = root.dump(2);
    if (!FileUtil::writeAllText(path, text)) {
      if (err) *err = "Failed to write file";
      return false;
    }

    Logger::info("Save", "Saved project to: " + path);
    return true;
  } catch (const std::exception& e) {
    if (err) *err = e.what();
    Logger::error("Save", std::string("Exception: ") + e.what());
    return false;
  }
}

bool loadFromFile(Project& project, const std::string& path, std::string* err) {
  try {
    std::string text;
    if (!FileUtil::readAllText(path, text)) {
      if (err) *err = "Failed to read file";
      return false;
    }

    json root = json::parse(text);
    int version = root.value("version", 1);

    project = Project{};
    project.resetToDefault();

    if (root.contains("stage")) {
      project.stage() = stageFromJson(root["stage"]);
    }

    project.sprites().clear();

    int maxSpriteId = 0;
    int maxScriptId = 0;
    int maxBlockId  = 0;

    if (root.contains("sprites") && root["sprites"].is_array()) {
      for (auto& it : root["sprites"]) {
        Sprite sp = spriteFromJson(it);

        maxSpriteId = std::max(maxSpriteId, sp.id);
        for (auto& sc : sp.scripts) maxScriptId = std::max(maxScriptId, sc.id);
        for (auto& kv : sp.blocks)   maxBlockId  = std::max(maxBlockId, kv.first);

        project.sprites().push_back(std::move(sp));
      }
    }

    // sync id generators (نیازمند ensureNextIds در Project)
    project.ensureNextIds(maxSpriteId + 1, maxScriptId + 1, maxBlockId + 1);

    project.setFilePath(path);
    project.clearDirty();

    Logger::info("Load", "Loaded project v" + std::to_string(version) + " from: " + path);
    return true;

  } catch (const std::exception& e) {
    if (err) *err = e.what();
    Logger::error("Load", std::string("Exception: ") + e.what());
    return false;
  }
}

} // namespace Serialization
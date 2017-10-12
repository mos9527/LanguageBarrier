#define DEFINE_JSON_CONSTANTS
#include "LanguageBarrier.h"
#include <ctime>
#include <fstream>
#include "Config.h"
#include "Game.h"
#include "GameText.h"
#include "MinHook.h"
#include "SigScan.h"

static bool isInitialised = false;

namespace lb {
void *memset_perms(void *dst, int val, size_t size) {
  DWORD oldProtect;
  VirtualProtect(dst, size, PAGE_READWRITE, &oldProtect);
  void *retval = memset(dst, val, size);
  VirtualProtect(dst, size, oldProtect, &oldProtect);
  return retval;
}
void loadJsonConstants() {
  LanguageBarrierLog("loading constants from gamedef.json/patchdef.json...");

  // Game.h
  BGM_CLEAR = config["gamedef"]["bgmClear"].get<uint32_t>();
  MPK_ID_SCRIPT_MPK = config["gamedef"]["mpkIdScriptMpk"].get<uint8_t>();
  MPK_ID_BGM_MPK = config["gamedef"]["mpkIdBgmMpk"].get<uint8_t>();
  AUDIO_PLAYER_ID_BGM1 = config["gamedef"]["audioPlayerIdBgm1"].get<uint8_t>();

  // GameText.h
  FIRST_FONT_ID = config["gamedef"]["firstFontId"].get<uint8_t>();
  COORDS_MULTIPLIER = config["gamedef"]["coordsMultiplier"].get<float>();
  FONT_CELL_WIDTH = config["gamedef"]["fontCellWidth"].get<uint8_t>();
  FONT_CELL_HEIGHT = config["gamedef"]["fontCellHeight"].get<uint8_t>();
  FONT_ROW_LENGTH = config["gamedef"]["fontRowLength"].get<uint8_t>();
  GLYPH_RANGE_FULLWIDTH_START =
      config["gamedef"]["glyphRangeFullwidthStart"].get<uint16_t>();
  DEFAULT_LINE_LENGTH = config["gamedef"]["defaultLineLength"].get<uint16_t>();
  DEFAULT_MAX_CHARACTERS =
      config["gamedef"]["defaultMaxCharacters"].get<uint16_t>();
  HAS_SGHD_PHONE = config["gamedef"]["hasSghdPhone"].get<bool>();
  if (HAS_SGHD_PHONE) {
    SGHD_LINK_UNDERLINE_GLYPH_X =
        config["gamedef"]["sghdLinkUnderlineGlyphX"].get<float>();
    SGHD_LINK_UNDERLINE_GLYPH_Y =
        config["gamedef"]["sghdLinkUnderlineGlyphY"].get<float>();
    SGHD_PHONE_X_PADDING = config["patch"]["sghdPhoneXPadding"].get<int>();
  }
  // these are default true for backwards compatibility with S;G0 patch config
  HAS_DOUBLE_GET_SC3_STRING_DISPLAY_WIDTH = true;
  if (config["gamedef"].count("hasDoubleGetSc3StringDisplayWidth") == 1) {
    HAS_DOUBLE_GET_SC3_STRING_DISPLAY_WIDTH =
        config["gamedef"]["hasDoubleGetSc3StringDisplayWidth"].get<bool>();
  }
  HAS_DRAW_PHONE_TEXT = true;
  if (config["gamedef"].count("hasDrawPhoneText") == 1) {
    HAS_DRAW_PHONE_TEXT = config["gamedef"]["hasDrawPhoneText"].get<bool>();
  }
  HAS_GET_SC3_STRING_LINE_COUNT = true;
  if (config["gamedef"].count("hasGetSc3StringLineCount") == 1) {
    HAS_GET_SC3_STRING_LINE_COUNT =
        config["gamedef"]["hasGetSc3StringLineCount"].get<bool>();
  }
  HAS_RINE = config["gamedef"]["hasRine"].get<bool>();
  if (HAS_RINE) {
    RINE_BLACK_NAMES = config["patch"]["rineBlackNames"].get<bool>();
  }
  DIALOGUE_REDESIGN_YOFFSET_SHIFT =
      config["patch"]["dialogueRedesignYOffsetShift"].get<int>();
  DIALOGUE_REDESIGN_LINEHEIGHT_SHIFT =
      config["patch"]["dialogueRedesignLineHeightShift"].get<int>();
  HAS_BACKLOG_UNDERLINE = config["gamedef"]["hasBacklogUnderline"].get<bool>();
  if (HAS_BACKLOG_UNDERLINE) {
    BACKLOG_HIGHLIGHT_DEFAULT_HEIGHT =
        config["gamedef"]["backlogHighlightDefaultHeight"].get<int8_t>();
    BACKLOG_HIGHLIGHT_HEIGHT_SHIFT =
        config["patch"]["backlogHighlightHeightShift"].get<int8_t>();
  }
  IMPROVE_DIALOGUE_OUTLINES =
      config["patch"]["improveDialogueOutlines"].get<bool>();
  if (IMPROVE_DIALOGUE_OUTLINES) {
    OUTLINE_PADDING = config["patch"]["outlinePadding"].get<float>();
    OUTLINE_CELL_WIDTH = config["patch"]["outlineCellWidth"].get<uint8_t>();
    OUTLINE_CELL_HEIGHT = config["patch"]["outlineCellHeight"].get<uint8_t>();
    OUTLINE_TEXTURE_ID = config["patch"]["outlineTextureId"].get<uint16_t>();
  }
  GLYPH_ID_FULLWIDTH_SPACE =
      config["gamedef"]["glyphIdFullwidthSpace"].get<uint16_t>();
  GLYPH_ID_HALFWIDTH_SPACE =
      config["gamedef"]["glyphIdHalfwidthSpace"].get<uint16_t>();
  NEEDS_CLEARLIST_TEXT_POSITION_ADJUST =
      config["gamedef"]["needsClearlistTextPositionAdjust"].get<bool>();
  HAS_SPLIT_FONT = config["gamedef"]["hasSplitFont"].get<bool>();
  if (config["patch"].count("tipReimplementation") == 1) {
    TIP_REIMPL = config["patch"]["tipReimplementation"].get<bool>();
  }
  if (TIP_REIMPL) {
    TIP_REIMPL_GLYPH_SIZE =
        config["patch"]["tipReimplementationGlyphSize"].get<int>();
    TIP_REIMPL_LINE_LENGTH =
        config["patch"]["tipReimplementationLineLength"].get<int>();
  }
  if (config["patch"].count("ccBacklogHighlight") == 1) {
    CC_BACKLOG_HIGHLIGHT = config["patch"]["ccBacklogHighlight"].get<bool>();
  }
  if (CC_BACKLOG_HIGHLIGHT) {
    CC_BACKLOG_HIGHLIGHT_HEIGHT_SHIFT =
        config["patch"]["ccBacklogHighlightHeightShift"].get<float>();
    CC_BACKLOG_HIGHLIGHT_SPRITE_HEIGHT =
        config["patch"]["ccBacklogHighlightSpriteHeight"].get<float>();
    CC_BACKLOG_HIGHLIGHT_SPRITE_Y =
        config["patch"]["ccBacklogHighlightSpriteY"].get<float>();
    CC_BACKLOG_HIGHLIGHT_YOFFSET_SHIFT =
        config["patch"]["ccBacklogHighlightYOffsetShift"].get<float>();
  }
}
void LanguageBarrierInit() {
  if (isInitialised) {
    LanguageBarrierLog("LanguageBarrierInit() called twice...");
    return;
  }
  isInitialised = true;

  configInit();

  std::remove("languagebarrier\\log.txt");
  // TODO: proper versioning
  LanguageBarrierLog("LanguageBarrier v1.10");
  {
    std::stringstream logstr;
    logstr << "Game: " << configGetGameName();
    LanguageBarrierLog(logstr.str());
  }
  {
    std::stringstream logstr;
    logstr << "Patch: " << configGetPatchName();
    LanguageBarrierLog(logstr.str());
  }
  LanguageBarrierLog("**** Start apprication ****");

  MH_STATUS mhStatus = MH_Initialize();
  if (mhStatus != MH_OK) {
    std::stringstream logstr;
    logstr << "MinHook failed to initialize!" << MH_StatusToString(mhStatus);
    LanguageBarrierLog(logstr.str());
    return;
  }

  WCHAR path[MAX_PATH], exeName[_MAX_FNAME];
  GetModuleFileNameW(NULL, path, MAX_PATH);
  _wsplitpath_s(path, NULL, 0, NULL, 0, exeName, _MAX_FNAME, NULL, 0);
  if (wcslen(exeName) < wcslen(L"Launcher") ||
      _wcsnicmp(exeName, L"Launcher", wcslen(L"Launcher")) != 0) {
    loadJsonConstants();
    gameInit();
  }
}
// TODO: make this better
void LanguageBarrierLog(const std::string &text) {
  std::ofstream logFile("languagebarrier\\log.txt",
                        std::ios_base::out | std::ios_base::app);
  std::time_t t = std::time(NULL);
  logFile << std::put_time(std::gmtime(&t), "[%D %r] ");
  logFile << text << std::endl;
}
bool scanCreateEnableHook(char *category, char *name, uintptr_t *ppTarget,
                          LPVOID pDetour, LPVOID *ppOriginal) {
  *ppTarget = sigScan(category, name);
  if (*ppTarget == NULL) return false;

  MH_STATUS mhStatus;
  mhStatus = MH_CreateHook((LPVOID)*ppTarget, pDetour, ppOriginal);
  if (mhStatus != MH_OK) {
    std::stringstream logstr;
    logstr << "Failed to create hook " << name << ": "
           << MH_StatusToString(mhStatus);
    LanguageBarrierLog(logstr.str());
    return false;
  }
  mhStatus = MH_EnableHook((LPVOID)*ppTarget);
  if (mhStatus != MH_OK) {
    std::stringstream logstr;
    logstr << "Failed to enable hook " << name << ": "
           << MH_StatusToString(mhStatus);
    LanguageBarrierLog(logstr.str());
    return false;
  }

  std::stringstream logstr;
  logstr << "Successfully hooked " << name;
  LanguageBarrierLog(logstr.str());

  return true;
}
bool createEnableApiHook(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour,
                         LPVOID *ppOriginal) {
  MH_STATUS mhStatus;
  LPVOID pTarget;
  mhStatus =
      MH_CreateHookApiEx(pszModule, pszProcName, pDetour, ppOriginal, &pTarget);
  if (mhStatus != MH_OK) {
    std::stringstream logstr;
    logstr << "Failed to create API hook " << pszModule << "." << pszProcName
           << ": " << MH_StatusToString(mhStatus);
    LanguageBarrierLog(logstr.str());
    return false;
  }
  mhStatus = MH_EnableHook(pTarget);
  if (mhStatus != MH_OK) {
    std::stringstream logstr;
    logstr << "Failed to enable API hook " << pszModule << "." << pszProcName
           << ": " << MH_StatusToString(mhStatus);
    LanguageBarrierLog(logstr.str());
    return false;
  }

  std::stringstream logstr;
  logstr << "Successfully hooked " << pszModule << "." << pszProcName;
  LanguageBarrierLog(logstr.str());

  return true;
}
void slurpFile(const std::string &fileName, std::string **ppBuffer) {
  std::ifstream in(fileName, std::ios::in | std::ios::binary);
  in.seekg(0, std::ios::end);
  *ppBuffer = new std::string(in.tellg(), 0);
  in.seekg(0, std::ios::beg);
  in.read(&((**ppBuffer)[0]), (*ppBuffer)->size());
  in.close();
}
}  // namespace lb

#include "MarineNaviPi.h"

#include "wx/wx.h"
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "ocpn_plugin.h"

class MarineNaviPi;
class Dlg;

using namespace std;

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr) {
  return new MarineNaviPi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) { delete p; }

//---------------------------------------------------------------------------------------------------------
//
//    Marine  NaviPi PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

#include "icons.h"

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

/**
 * Load a icon, possibly using SVG
 * Parameters
 *  - api_name: Argument to GetPluginDataDir()
 *  - icon_name: Base name of icon living in data/ directory. When using
 *    SVG icon_name.svg is used, otherwise icon_name.png
 */

static wxBitmap load_plugin(const char* icon_name, const char* api_name) {
  wxBitmap bitmap;
  wxFileName fn;
  auto path = GetPluginDataDir(api_name);
  fn.SetPath(path);
  fn.AppendDir("data");
  fn.SetName(icon_name);
#ifdef ocpnUSE_SVG
  wxLogDebug("Loading SVG icon");
  fn.SetExt("svg");
  const static int ICON_SIZE = 48;  // FIXME: Needs size from GUI
  bitmap = GetBitmapFromSVGFile(fn.GetFullPath(), ICON_SIZE, ICON_SIZE);
#else
  wxLogDebug("Loading png icon");
  fn.SetExt("png");
  path = fn.GetFullPath();
  if (!wxImage::CanRead(path)) {
    wxLogDebug("Initiating image handlers.");
    wxInitAllImageHandlers();
  }
  wxImage panelIcon(path);
  bitmap = wxBitmap(panelIcon);
#endif
  wxLogDebug("Icon loaded, result: %s", bitmap.IsOk() ? "ok" : "fail");
  return bitmap;
}

MarineNaviPi::MarineNaviPi(void* ppimgr)
    : opencpn_plugin_118(ppimgr),
      deps_(marine_navi::CreateDependencies(nullptr)),
      parentWindow_(nullptr),
      dlg_(nullptr),
      renderOverlay_(deps_.RenderOverlay),
      toolId_(-1),
      showDlg_(false) {
  // Create the PlugIn icons
  initialize_images();
  panelBitmap_ = load_plugin("shipdriver_panel_icon", "MarineNaviPi");
}

MarineNaviPi::~MarineNaviPi(void) { delete _img_ShipDriverIcon; }

int MarineNaviPi::Init(void) {
  AddLocaleCatalog("opencpn-MarineNaviPi");

  parentWindow_ = GetOCPNCanvasWindow();
#ifdef ocpnUSE_SVG
  toolId_ = InsertPlugInToolSVG(
      "Navigation", _svg_shipdriver, _svg_shipdriver, _svg_shipdriver_toggled,
      wxITEM_CHECK, "Navigation", "", NULL, ShipDriver_TOOL_POSITION, 0, this);
#else
  toolId_ = InsertPlugInTool("", _img_ShipDriverIcon, _img_ShipDriverIcon,
                             wxITEM_CHECK, _("ShipDriver"), "", NULL,
                             ShipDriver_TOOL_POSITION, 0, this);
#endif

  return (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK |
          WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL |
          WANTS_PLUGIN_MESSAGING | WANTS_VECTOR_CHART_OBJECT_INFO);

  // return (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK |
  //         WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL |
  //         WANTS_CURSOR_LATLON | WANTS_NMEA_SENTENCES | WANTS_AIS_SENTENCES |
  //         WANTS_PREFERENCES | WANTS_PLUGIN_MESSAGING | WANTS_CONFIG);
}

bool MarineNaviPi::DeInit(void) {
  if (dlg_) {
    dlg_->Close();
    dlg_ = nullptr;
    showDlg_ = false;
    SetToolbarItemState(toolId_, showDlg_);
  }

  RequestRefresh(parentWindow_);
  return true;
}

int MarineNaviPi::GetAPIVersionMajor() { return atoi(API_VERSION); }

int MarineNaviPi::GetAPIVersionMinor() {
  std::string v(API_VERSION);
  size_t dotpos = v.find('.');
  return atoi(v.substr(dotpos + 1).c_str());
}

int MarineNaviPi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int MarineNaviPi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

int GetPlugInVersionPatch() { return PLUGIN_VERSION_PATCH; }

int GetPlugInVersionPost() { return PLUGIN_VERSION_TWEAK; };

const char* GetPlugInVersionPre() { return PKG_PRERELEASE; }

const char* GetPlugInVersionBuild() { return PKG_BUILD_INFO; }

wxBitmap* MarineNaviPi::GetPlugInBitmap() { return &panelBitmap_; }

wxString MarineNaviPi::GetCommonName() { return PLUGIN_API_NAME; }

wxString MarineNaviPi::GetShortDescription() { return PKG_SUMMARY; }

wxString MarineNaviPi::GetLongDescription() { return PKG_DESCRIPTION; }

void MarineNaviPi::OnToolbarToolCallback(int id) {
  if (!dlg_) {
    dlg_ = std::make_shared<MarineNavi::dialogs::MarineNaviMainDlg>(
        parentWindow_, -1, "Main dialog", wxPoint(100, 100), wxSize(800, 800),
        deps_);
    dlg_->Register(std::bind(&MarineNaviPi::OnMainDlgClose, this),
                   MarineNavi::dialogs::MarineNaviDlgBase::EventType::kClose);
  }

  // Toggle
  showDlg_ = !showDlg_;

  // Toggle dialog?
  if (showDlg_) {
    dlg_->Show();
  } else {
    dlg_->Hide();
  }

  // Toggle is handled by the toolbar but we must keep plugin manager b_toggle
  // updated to actual status to ensure correct status upon toolbar rebuild
  SetToolbarItemState(toolId_, showDlg_);

  // Capture dialog position
  RequestRefresh(parentWindow_);  // refresh main window
}

void MarineNaviPi::OnMainDlgClose() {
  // assert(showDlg_ == false);

  showDlg_ = false;
  SetToolbarItemState(toolId_, showDlg_);
  dlg_->Hide();

  RequestRefresh(parentWindow_);  // refresh main window
}

wxString MarineNaviPi::StandardPath() {
  wxString s = wxFileName::GetPathSeparator();
  wxString stdPath = *GetpPrivateApplicationDataLocation();

  stdPath += s + _T("plugins");
  if (!wxDirExists(stdPath)) {
    wxMkdir(stdPath);
  }

  stdPath += s + _T("marine_navi");

#ifdef __WXOSX__
  // Compatibility with pre-OCPN-4.2; move config dir to
  // ~/Library/Preferences/opencpn if it exists
  {
    wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
    wxString s = wxFileName::GetPathSeparator();
    // should be ~/Library/Preferences/opencpn
    wxString oldPath = (std_path.GetUserConfigDir() + s + _T("plugins") + s +
                        _T("marine_navi"));
    if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
      wxLogMessage("marine_navi: moving config dir %s to %s", oldPath, stdPath);
      wxRenameFile(oldPath, stdPath);
    }
  }
#endif

  if (!wxDirExists(stdPath)) {
    wxMkdir(stdPath);
  }

  stdPath += s;
  return stdPath;
}

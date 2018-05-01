/**********************************************************************

  Audacity: A Digital Audio Editor

  LadspaEffect.h

  Dominic Mazzoni

**********************************************************************/

class wxSlider;
class wxStaticText;
class wxTextCtrl;
class wxCheckBox;

#include "../../MemoryX.h"
#include <wx/dialog.h>

#include "audacity/EffectInterface.h"
#include "audacity/ModuleInterface.h"
#include "audacity/PluginInterface.h"

#include "../../widgets/NumericTextCtrl.h"

#include "ladspa.h"
#include "../../SampleFormat.h"

#define LADSPAEFFECTS_VERSION wxT("1.0.0.0")
/* i8n-hint: abbreviates "Linux Audio Developer's Simple Plugin API"
   (Application programming interface)
 */
#define LADSPAEFFECTS_FAMILY XO("LADSPA")

///////////////////////////////////////////////////////////////////////////////
//
// LadspaEffect
//
///////////////////////////////////////////////////////////////////////////////

class LadspaEffectMeter;

class LadspaEffect final : public wxEvtHandler,
                     public EffectClientInterface,
                     public EffectUIClientInterface
{
public:
   LadspaEffect(const wxString & path, int index);
   virtual ~LadspaEffect();

   // IdentInterface implementation

   wxString GetPath() override;
   IdentInterfaceSymbol GetSymbol() override;
   IdentInterfaceSymbol GetVendor() override;
   wxString GetVersion() override;
   wxString GetDescription() override;

   // EffectDefinitionInterface implementation

   EffectType GetType() override;
   IdentInterfaceSymbol GetFamilyId() override;
   bool IsInteractive() override;
   bool IsDefault() override;
   bool IsLegacy() override;
   bool SupportsRealtime() override;
   bool SupportsAutomation() override;

   // EffectClientInterface implementation

   bool SetHost(EffectHostInterface *host) override;

   unsigned GetAudioInCount() override;
   unsigned GetAudioOutCount() override;

   int GetMidiInCount() override;
   int GetMidiOutCount() override;

   void SetSampleRate(double rate) override;
   size_t SetBlockSize(size_t maxBlockSize) override;

   sampleCount GetLatency() override;
   size_t GetTailSize() override;

   bool IsReady() override;
   bool ProcessInitialize(sampleCount totalLen, ChannelNames chanMap = NULL) override;
   bool ProcessFinalize() override;
   size_t ProcessBlock(float **inBlock, float **outBlock, size_t blockLen) override;

   bool RealtimeInitialize() override;
   bool RealtimeAddProcessor(unsigned numChannels, float sampleRate) override;
   bool RealtimeFinalize() override;
   bool RealtimeSuspend() override;
   bool RealtimeResume() override;
   bool RealtimeProcessStart() override;
   size_t RealtimeProcess(int group,
                                       float **inbuf,
                                       float **outbuf,
                                       size_t numSamples) override;
   bool RealtimeProcessEnd() override;

   bool ShowInterface(wxWindow *parent, bool forceModal = false) override;

   bool GetAutomationParameters(CommandParameters & parms) override;
   bool SetAutomationParameters(CommandParameters & parms) override;

   bool LoadUserPreset(const wxString & name) override;
   bool SaveUserPreset(const wxString & name) override;

   wxArrayString GetFactoryPresets() override;
   bool LoadFactoryPreset(int id) override;
   bool LoadFactoryDefaults() override;

   // EffectUIClientInterface implementation

   void SetHostUI(EffectUIHostInterface *host) override;
   bool PopulateUI(wxWindow *parent) override;
   bool IsGraphicalUI() override;
   bool ValidateUI() override;
   bool HideUI() override;
   bool CloseUI() override;

   bool CanExportPresets() override;
   void ExportPresets() override;
   void ImportPresets() override;

   bool HasOptions() override;
   void ShowOptions() override;

   // LadspaEffect implementation

private:
   bool Load();
   void Unload();

   bool LoadParameters(const wxString & group);
   bool SaveParameters(const wxString & group);

   LADSPA_Handle InitInstance(float sampleRate);
   void FreeInstance(LADSPA_Handle handle);

   void OnCheckBox(wxCommandEvent & evt);
   void OnSlider(wxCommandEvent & evt);
   void OnTextCtrl(wxCommandEvent & evt);
   void RefreshControls(bool outputOnly = false);

private:

   wxString mPath;
   int mIndex;
   EffectHostInterface *mHost;

   wxDynamicLibrary mLib;
   const LADSPA_Descriptor *mData;

   wxString pluginName;

   bool mReady;

   LADSPA_Handle mMaster;

   double mSampleRate;
   size_t mBlockSize;

   bool mInteractive;

   unsigned mAudioIns;
   ArrayOf<unsigned long> mInputPorts;

   unsigned mAudioOuts;
   ArrayOf<unsigned long> mOutputPorts;

   int mNumInputControls;
   Floats mInputControls;
   int mNumOutputControls;
   Floats mOutputControls;

   bool mUseLatency;
   int mLatencyPort;
   bool mLatencyDone;

   // Realtime processing
   std::vector<LADSPA_Handle> mSlaves;

   EffectUIHostInterface *mUIHost;

   NumericTextCtrl *mDuration;
   wxDialog *mDialog;
   wxWindow *mParent;
   ArrayOf<wxSlider*> mSliders;
   ArrayOf<wxTextCtrl*> mFields;
   ArrayOf<wxStaticText*> mLabels;
   ArrayOf<wxCheckBox*> mToggles;
   ArrayOf<LadspaEffectMeter *> mMeters;

   DECLARE_EVENT_TABLE()

   friend class LadspaEffectsModule;
};

///////////////////////////////////////////////////////////////////////////////
//
// LadspaEffectsModule
//
///////////////////////////////////////////////////////////////////////////////

class LadspaEffectsModule final : public ModuleInterface
{
public:
   LadspaEffectsModule(ModuleManagerInterface *moduleManager, const wxString *path);
   virtual ~LadspaEffectsModule();

   // IdentInterface implementation

   wxString GetPath() override;
   IdentInterfaceSymbol GetSymbol() override;
   IdentInterfaceSymbol GetVendor() override;
   wxString GetVersion() override;
   wxString GetDescription() override;

   // ModuleInterface implementation

   bool Initialize() override;
   void Terminate() override;

   wxArrayString FileExtensions() override;
   wxString InstallPath() override;

   bool AutoRegisterPlugins(PluginManagerInterface & pm) override;
   wxArrayString FindPluginPaths(PluginManagerInterface & pm) override;
   unsigned DiscoverPluginsAtPath(
      const wxString & path, wxString &errMsg,
      const RegistrationCallback &callback)
         override;

   bool IsPluginValid(const wxString & path, bool bFast) override;

   IdentInterface *CreateInstance(const wxString & path) override;
   void DeleteInstance(IdentInterface *instance) override;

   // LadspaEffectModule implementation

   wxArrayString GetSearchPaths();

private:
   ModuleManagerInterface *mModMan;
   wxString mPath;
};


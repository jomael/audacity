/**********************************************************************

  Audacity: A Digital Audio Editor

  PrefsDialog.h

  Joshua Haberman
  James Crook

**********************************************************************/

#ifndef __AUDACITY_PREFS_DIALOG__
#define __AUDACITY_PREFS_DIALOG__

#include <functional>
#include <vector>
#include "../widgets/wxPanelWrapper.h" // to inherit

class AudacityProject;
class wxTreebook;
class wxTreeEvent;
class PrefsPanel;
class ShuttleGui;

#ifdef __GNUC__
#define CONST
#else
#define CONST const
#endif

class AudacityProject;

class PrefsDialog /* not final */ : public wxDialogWrapper
{
 public:
    // An array of PrefsNode specifies the tree of pages in pre-order traversal.
    struct PrefsNode {
       using Factory =
         std::function< PrefsPanel * (
            wxWindow *parent, wxWindowID winid, AudacityProject *) >;
       Factory factory;
       size_t nChildren{ 0 };
       bool expanded{ false };

       PrefsNode(const Factory &factory_,
          unsigned nChildren_ = 0,
          bool expanded_ = true)
          : factory(factory_), nChildren(nChildren_), expanded(expanded_)
       {}
    };
   typedef std::vector<PrefsNode> Factories;
   static Factories &DefaultFactories();

   PrefsDialog(wxWindow * parent,
      AudacityProject *pProject, // may be null
      const TranslatableString &titlePrefix = XO("Preferences:"),
      Factories &factories = DefaultFactories());
   virtual ~PrefsDialog();

   // Defined this so a protected virtual can be invoked after the constructor
   int ShowModal() override;
   void ShuttleAll( ShuttleGui & S);

   void OnCategoryChange(wxCommandEvent & e);
   void OnOK(wxCommandEvent & e);
   void OnCancel(wxCommandEvent & e);
   void OnPreview(wxCommandEvent & e);
   void OnHelp(wxCommandEvent & e);
   void OnTreeKeyDown(wxTreeEvent & e); // Used to dismiss the dialog when enter is pressed with focus on tree

   void SelectPageByName(const wxString &pageName);

   // Accessor to help implementations of SavePreferredPage(),
   // such as by saving a preference after DoModal() returns
   int GetSelectedPage() const;

 protected:
    // Decide which page to open first; return -1 for undecided
    virtual long GetPreferredPage() = 0;

    // Called after OK is clicked and all pages validate
    virtual void SavePreferredPage() = 0;

private:
   void RecordExpansionState();
   PrefsPanel * GetCurrentPanel();
   wxTreebook *mCategories{};
   PrefsPanel *mUniquePage{};
   Factories &mFactories;
   const TranslatableString mTitlePrefix;

   DECLARE_EVENT_TABLE()
};

// This adds code appropriate only to the original use of PrefsDialog for
// global settings -- not its reuses elsewhere as in View Settings
class GlobalPrefsDialog final : public PrefsDialog
{
public:
   GlobalPrefsDialog(
      wxWindow * parent, AudacityProject *pProject,
      Factories &factories = DefaultFactories());
   virtual ~GlobalPrefsDialog();
   long GetPreferredPage() override;
   void SavePreferredPage() override;
};

class AudacityProject;
void DoReloadPreferences( AudacityProject &project );

#endif

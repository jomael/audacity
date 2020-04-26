/**********************************************************************

  Audacity: A Digital Audio Editor

  HistoryWindow.h

  Joshua Haberman

**********************************************************************/

#ifndef __AUDACITY_HISTORY_WINDOW__
#define __AUDACITY_HISTORY_WINDOW__

#include "widgets/wxPanelWrapper.h" // to inherit

class wxButton;
class wxListCtrl;
class wxListEvent;
class wxSpinCtrl;
class wxTextCtrl;
class AudacityProject;
class ShuttleGui;
class UndoManager;

class HistoryDialog final : public wxDialogWrapper {

 public:
   HistoryDialog(AudacityProject * parent, UndoManager *manager);

   void UpdateDisplay(wxEvent &e);
   
   bool Show( bool show = true ) override;

 private:
   void OnAudioIO(wxCommandEvent & evt);
   void DoUpdate();
   void UpdateLevels();

   void OnChar(wxEvent &event);
   void OnSize(wxSizeEvent & event);
   void OnCloseWindow(wxCloseEvent & WXUNUSED(event));
   void OnItemSelected(wxListEvent & event);
   void OnDiscard(wxCommandEvent & event);
   void OnDiscardClipboard(wxCommandEvent & event);

   AudacityProject   *mProject;
   UndoManager       *mManager;
   wxListCtrl        *mList;
   wxTextCtrl        *mTotal;
   wxTextCtrl        *mClipboard;
   wxTextCtrl        *mAvail;
   wxSpinCtrl        *mLevels;
   wxButton          *mDiscard;

   int               mSelected;
   bool              mAudioIOBusy;

 public:
   DECLARE_EVENT_TABLE()
};

#endif

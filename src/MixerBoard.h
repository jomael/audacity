/**********************************************************************

  Audacity: A Digital Audio Editor

  MixerBoard.h

  Vaughan Johnson, January 2007

**********************************************************************/

#include "Experimental.h"

#ifndef __AUDACITY_MIXER_BOARD__
#define __AUDACITY_MIXER_BOARD__

#include <wx/frame.h>
#include <wx/bmpbuttn.h>
#include <wx/hashmap.h>
#include <wx/image.h>
#include <wx/scrolwin.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>

#include "widgets/AButton.h"
#include "widgets/ASlider.h"
#include "widgets/wxPanelWrapper.h"

// containment hierarchy:
//    MixerBoardFrame -> MixerBoard -> MixerBoardScrolledWindow -> MixerTrackCluster(s)


// MixerTrackSlider is a subclass just to override OnMouseEvent,
// so we can know when adjustment ends, so we can PushState only then.
class MixerTrackSlider final : public ASlider
{
public:
   MixerTrackSlider(wxWindow * parent,
                     wxWindowID id,
                     const wxString &name,
                     const wxPoint & pos,
                     const wxSize & size,
                     const ASlider::Options &options = ASlider::Options{});
   virtual ~MixerTrackSlider() {}

   void OnMouseEvent(wxMouseEvent & event);

   void OnFocus(wxFocusEvent &event);
   void OnCaptureKey(wxCommandEvent& event);

protected:
   bool mIsPan;

public:
    DECLARE_EVENT_TABLE()
};


class AudacityProject;
class MeterPanel;
class MixerBoard;

class Track;
#ifdef USE_MIDI
class NoteTrack;
#endif
class PlayableTrack;

class WaveTrack;
class auStaticText;

class MixerTrackCluster final : public wxPanelWrapper
{
public:
   MixerTrackCluster(wxWindow* parent,
                     MixerBoard* grandParent, AudacityProject* project,
                     const std::shared_ptr<PlayableTrack> &pTrack,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize);
   virtual ~MixerTrackCluster() {}

   WaveTrack *GetWave() const;
   WaveTrack *GetRight() const;
#ifdef EXPERIMENTAL_MIDI_OUT
   NoteTrack *GetNote() const;
#endif

   void UpdatePrefs();

   void HandleResize(); // For wxSizeEvents, update gain slider and meter.

   void HandleSliderGain(const bool bWantPushState = false);
#ifdef EXPERIMENTAL_MIDI_OUT
   void HandleSliderVelocity(const bool bWantPushState = false);
#endif
   void HandleSliderPan(const bool bWantPushState = false);

   void ResetMeter(const bool bResetClipping);

   // These are used by TrackPanel for synchronizing control states.
   void UpdateForStateChange(); // Update the controls that can be affected by state change.
   void UpdateName();
   void UpdateMute();
   void UpdateSolo();
   void UpdatePan();
   void UpdateGain();
#ifdef EXPERIMENTAL_MIDI_OUT
   void UpdateVelocity();
#endif
   void UpdateMeter(const double t0, const double t1);

private:
   wxColour GetTrackColor();

   // event handlers
   void HandleSelect(bool bShiftDown, bool bControlDown);

   void OnKeyEvent(wxKeyEvent& event);
   void OnMouseEvent(wxMouseEvent& event);
   void OnPaint(wxPaintEvent& evt);

   void OnButton_MusicalInstrument(wxCommandEvent& event);
   void OnSlider_Gain(wxCommandEvent& event);
#ifdef EXPERIMENTAL_MIDI_OUT
   void OnSlider_Velocity(wxCommandEvent& event);
#endif
   void OnSlider_Pan(wxCommandEvent& event);
   void OnButton_Mute(wxCommandEvent& event);
   void OnButton_Solo(wxCommandEvent& event);
   //v void OnSliderScroll_Gain(wxScrollEvent& event);


public:
   std::shared_ptr<PlayableTrack>   mTrack;

private:
   MixerBoard* mMixerBoard;
   AudacityProject* mProject;

   // controls
   auStaticText* mStaticText_TrackName;
   wxBitmapButton* mBitmapButton_MusicalInstrument;
   AButton* mToggleButton_Mute;
   AButton* mToggleButton_Solo;
   MixerTrackSlider* mSlider_Pan;
   MixerTrackSlider* mSlider_Gain;
#ifdef EXPERIMENTAL_MIDI_OUT
   MixerTrackSlider* mSlider_Velocity;
#endif
   MeterPanel* mMeter;

public:
   DECLARE_EVENT_TABLE()
};


class MusicalInstrument
{
public:
   MusicalInstrument(std::unique_ptr<wxBitmap> &&pBitmap, const wxString & strXPMfilename);
   virtual ~MusicalInstrument();

   std::unique_ptr<wxBitmap> mBitmap;
   wxArrayString  mKeywords;
};

using MusicalInstrumentArray = std::vector<movable_ptr<MusicalInstrument>>;



// wxScrolledWindow ignores mouse clicks in client area,
// but they don't get passed to Mixerboard.
// We need to catch them to deselect all track clusters.
class MixerBoardScrolledWindow final : public wxScrolledWindow
{
public:
   MixerBoardScrolledWindow(AudacityProject* project,
                              MixerBoard* parent, wxWindowID id = -1,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = wxHSCROLL | wxVSCROLL);
   virtual ~MixerBoardScrolledWindow();

private:
   void OnMouseEvent(wxMouseEvent& event);

private:
   MixerBoard* mMixerBoard;
   AudacityProject* mProject;

public:
   DECLARE_EVENT_TABLE()
};


class MixerBoardFrame;
class TrackList;

class MixerBoard final : public wxWindow
{
   friend class MixerBoardFrame;

public:
   MixerBoard(AudacityProject* pProject,
               wxFrame* parent,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize);

   void UpdatePrefs();

   // Add clusters for any tracks we're not yet showing.
   // Update pointers for tracks we're aleady showing.
   void UpdateTrackClusters();

   int GetTrackClustersWidth();
   void MoveTrackCluster(const PlayableTrack* pTrack, bool bUp); // Up in TrackPanel is left in MixerBoard.
   void RemoveTrackCluster(const PlayableTrack* pTrack);
   void RemoveTrackCluster(size_t nIndex);


   wxBitmap* GetMusicalInstrumentBitmap(const Track *pTrack);

   bool HasSolo();

   void RefreshTrackCluster(const PlayableTrack* pTrack, bool bEraseBackground = true);
   void RefreshTrackClusters(bool bEraseBackground = true);
   void ResizeTrackClusters();

   void ResetMeters(const bool bResetClipping);

   void UpdateName(const PlayableTrack* pTrack);
   void UpdateMute(const PlayableTrack* pTrack = NULL); // NULL means update for all tracks.
   void UpdateSolo(const PlayableTrack* pTrack = NULL); // NULL means update for all tracks.
   void UpdatePan(const PlayableTrack* pTrack = NULL); // NULL means update for all tracks.
   void UpdateGain(const PlayableTrack* pTrack);
#ifdef EXPERIMENTAL_MIDI_OUT
   void UpdateVelocity(const PlayableTrack* pTrack);
#endif

   void UpdateMeters(const double t1, const bool bLoopedPlay);

   void UpdateWidth();

private:
   void MakeButtonBitmap( wxMemoryDC & dc, wxBitmap & bitmap, 
      wxRect & bev, const wxString & str, bool up );
   void CreateMuteSoloImages();
   int FindMixerTrackCluster(const PlayableTrack* pTrack,
                              MixerTrackCluster** hMixerTrackCluster) const;
   void LoadMusicalInstruments();

   // event handlers
   void OnSize(wxSizeEvent &evt);
   void OnTimer(wxCommandEvent &event);


public:
   // mute & solo button images: Create once and store on MixerBoard for use in all MixerTrackClusters.
   std::unique_ptr<wxImage> mImageMuteUp, mImageMuteOver, mImageMuteDown,
      mImageMuteDownWhileSolo, // the one actually alternate image
      mImageMuteDisabled, mImageSoloUp, mImageSoloOver, mImageSoloDown, mImageSoloDisabled;

   int mMuteSoloWidth;

private:
   // Track clusters are maintained in the same order as the WaveTracks.
   std::vector<MixerTrackCluster*> mMixerTrackClusters;

   MusicalInstrumentArray     mMusicalInstruments;
   AudacityProject*           mProject;
   MixerBoardScrolledWindow*  mScrolledWindow; // Holds the MixerTrackClusters and handles scrolling.
   double                     mPrevT1;
   TrackList*                 mTracks;

public:
   DECLARE_EVENT_TABLE()
};


class MixerBoardFrame final : public wxFrame
{
public:
   MixerBoardFrame(AudacityProject* parent);
   virtual ~MixerBoardFrame();

private:
   // event handlers
   void OnCloseWindow(wxCloseEvent &WXUNUSED(event));
   void OnMaximize(wxMaximizeEvent &event);
   void OnSize(wxSizeEvent &evt);
   void OnKeyEvent(wxKeyEvent &evt);

public:
   MixerBoard* mMixerBoard;

public:
   DECLARE_EVENT_TABLE()
};

#endif // __AUDACITY_MIXER_BOARD__



/**********************************************************************

  Audacity: A Digital Audio Editor

  Normalize.cpp

  Dominic Mazzoni
  Vaughan Johnson (Preview)

*******************************************************************//**

\class EffectNormalize
\brief An Effect to bring the peak level up to a chosen level.

*//*******************************************************************/


#include "../Audacity.h" // for rint from configwin.h
#include "Normalize.h"

#include <math.h>

#include <wx/intl.h>
#include <wx/valgen.h>

#include "../Internat.h"
#include "../Prefs.h"
#include "../ShuttleGui.h"
#include "../WaveTrack.h"
#include "../widgets/valnum.h"

// Define keys, defaults, minimums, and maximums for the effect parameters
//
//     Name       Type     Key                        Def      Min      Max   Scale
Param( Level,     double,  wxT("Level"),               -1.0,    -145.0,  0.0,  1  );
Param( RemoveDC,  bool,    wxT("RemoveDcOffset"),      true,    false,   true, 1  );
Param( ApplyGain, bool,    wxT("ApplyGain"),           true,    false,   true, 1  );
Param( StereoInd, bool,    wxT("StereoIndependent"),   false,   false,   true, 1  );

BEGIN_EVENT_TABLE(EffectNormalize, wxEvtHandler)
   EVT_CHECKBOX(wxID_ANY, EffectNormalize::OnUpdateUI)
   EVT_TEXT(wxID_ANY, EffectNormalize::OnUpdateUI)
END_EVENT_TABLE()

EffectNormalize::EffectNormalize()
{
   mLevel = DEF_Level;
   mDC = DEF_RemoveDC;
   mGain = DEF_ApplyGain;
   mStereoInd = DEF_StereoInd;

   SetLinearEffectFlag(false);
}

EffectNormalize::~EffectNormalize()
{
}

// IdentInterface implementation

IdentInterfaceSymbol EffectNormalize::GetSymbol()
{
   return NORMALIZE_PLUGIN_SYMBOL;
}

wxString EffectNormalize::GetDescription()
{
   return _("Sets the peak amplitude of one or more tracks");
}

wxString EffectNormalize::ManualPage()
{
   return wxT("Normalize");
}

// EffectDefinitionInterface implementation

EffectType EffectNormalize::GetType()
{
   return EffectTypeProcess;
}

// EffectClientInterface implementation
bool EffectNormalize::DefineParams( ShuttleParams & S ){
   S.SHUTTLE_PARAM( mLevel, Level );
   S.SHUTTLE_PARAM( mGain, ApplyGain );
   S.SHUTTLE_PARAM( mDC, RemoveDC );
   S.SHUTTLE_PARAM( mStereoInd, StereoInd );
   return true;
}

bool EffectNormalize::GetAutomationParameters(CommandParameters & parms)
{
   parms.Write(KEY_Level, mLevel);
   parms.Write(KEY_ApplyGain, mGain);
   parms.Write(KEY_RemoveDC, mDC);
   parms.Write(KEY_StereoInd, mStereoInd);

   return true;
}

bool EffectNormalize::SetAutomationParameters(CommandParameters & parms)
{
   ReadAndVerifyDouble(Level);
   ReadAndVerifyBool(ApplyGain);
   ReadAndVerifyBool(RemoveDC);
   ReadAndVerifyBool(StereoInd);

   mLevel = Level;
   mGain = ApplyGain;
   mDC = RemoveDC;
   mStereoInd = StereoInd;

   return true;
}

// Effect implementation

bool EffectNormalize::CheckWhetherSkipEffect()
{
   return ((mGain == false) && (mDC == false));
}

bool EffectNormalize::Startup()
{
   wxString base = wxT("/Effects/Normalize/");

   // Migrate settings from 2.1.0 or before

   // Already migrated, so bail
   if (gPrefs->Exists(base + wxT("Migrated")))
   {
      return true;
   }

   // Load the old "current" settings
   if (gPrefs->Exists(base))
   {
      int boolProxy = gPrefs->Read(base + wxT("RemoveDcOffset"), 1);
      mDC = (boolProxy == 1);
      boolProxy = gPrefs->Read(base + wxT("Normalize"), 1);
      mGain = (boolProxy == 1);
      gPrefs->Read(base + wxT("Level"), &mLevel, -1.0);
      if(mLevel > 0.0)  // this should never happen
         mLevel = -mLevel;
      boolProxy = gPrefs->Read(base + wxT("StereoIndependent"), 0L);
      mStereoInd = (boolProxy == 1);

      SaveUserPreset(GetCurrentSettingsGroup());

      // Do not migrate again
      gPrefs->Write(base + wxT("Migrated"), true);
      gPrefs->Flush();
   }

   return true;
}

bool EffectNormalize::Process()
{
   if (mGain == false && mDC == false)
      return true;

   float ratio;
   if( mGain )
      // same value used for all tracks
      ratio = DB_TO_LINEAR(TrapDouble(mLevel, MIN_Level, MAX_Level));
   else
      ratio = 1.0;

   //Iterate over each track
   this->CopyInputTracks(); // Set up mOutputTracks.
   bool bGoodResult = true;
   SelectedTrackListOfKindIterator iter(Track::Wave, mOutputTracks.get());
   WaveTrack *track = (WaveTrack *) iter.First();
   WaveTrack *prevTrack;
   prevTrack = track;
   int curTrackNum = 0;
   wxString topMsg;
   if(mDC && mGain)
      topMsg = _("Removing DC offset and Normalizing...\n");
   else if(mDC && !mGain)
      topMsg = _("Removing DC offset...\n");
   else if(!mDC && mGain)
      topMsg = _("Normalizing without removing DC offset...\n");
   else if(!mDC && !mGain)
      topMsg = _("Not doing anything...\n");   // shouldn't get here

   while (track) {
      //Get start and end times from track
      double trackStart = track->GetStartTime();
      double trackEnd = track->GetEndTime();

      //Set the current bounds to whichever left marker is
      //greater and whichever right marker is less:
      mCurT0 = mT0 < trackStart? trackStart: mT0;
      mCurT1 = mT1 > trackEnd? trackEnd: mT1;

      // Process only if the right marker is to the right of the left marker
      if (mCurT1 > mCurT0) {
         wxString msg;
         auto trackName = track->GetName();

         if(!track->GetLinked() || mStereoInd)
            msg =
               topMsg + wxString::Format( _("Analyzing: %s"), trackName );
         else
            msg =
               topMsg + wxString::Format( _("Analyzing first track of stereo pair: %s"), trackName );
         float offset, min, max;
         bGoodResult = AnalyseTrack(track, msg, curTrackNum, offset, min, max); 
         if (!bGoodResult )
             break;
         if(!track->GetLinked() || mStereoInd) {
            // mono or 'stereo tracks independently'
            float extent = wxMax(fabs(max), fabs(min));
            if( (extent > 0) && mGain )
               mMult = ratio / extent;
            else
               mMult = 1.0;
            msg =
               topMsg + wxString::Format( _("Processing: %s"), trackName );
            if(track->GetLinked() || prevTrack->GetLinked())  // only get here if there is a linked track but we are processing independently
               msg =
                  topMsg + wxString::Format( _("Processing stereo channels independently: %s"), trackName );

            if (!ProcessOne(track, msg, curTrackNum, offset))
            {
               bGoodResult = false;
               break;
            }
         }
         else
         {
            // we have a linked stereo track
            // so we need to find it's min, max and offset
            // as they are needed to calc the multiplier for both tracks
            track = (WaveTrack *) iter.Next();  // get the next one
            msg =
               topMsg + wxString::Format( _("Analyzing second track of stereo pair: %s"), trackName );
            float offset2, min2, max2;
            bGoodResult = AnalyseTrack(track, msg, curTrackNum + 1, offset2, min2, max2);
            if ( !bGoodResult )
                break;
            float extent = wxMax(fabs(min), fabs(max));
            extent = wxMax(extent, fabs(min2));
            extent = wxMax(extent, fabs(max2));
            if( (extent > 0) && mGain )
               mMult = ratio / extent; // we need to use this for both linked tracks
            else
               mMult = 1.0;
            track = (WaveTrack *) iter.Prev();  // go back to the first linked one
            msg =
               topMsg + wxString::Format( _("Processing first track of stereo pair: %s"), trackName );
            if (!ProcessOne(track, msg, curTrackNum, offset))
            {
               bGoodResult = false;
               break;
            }
            track = (WaveTrack *) iter.Next();  // go to the second linked one
            curTrackNum++;   // keeps progress bar correct
            msg =
               topMsg + wxString::Format( _("Processing second track of stereo pair: %s"), trackName );
            if (!ProcessOne(track, msg, curTrackNum, offset2))
            {
               bGoodResult = false;
               break;
            }
         }
      }

      //Iterate to the next track
      prevTrack = track;
      track = (WaveTrack *) iter.Next();
      curTrackNum++;
   }

   this->ReplaceProcessedTracks(bGoodResult);
   return bGoodResult;
}

void EffectNormalize::PopulateOrExchange(ShuttleGui & S)
{
   mCreating = true;

   S.StartVerticalLay(0);
   {
      S.StartMultiColumn(2, wxALIGN_CENTER);
      {
         S.StartVerticalLay(false);
         {
            mDCCheckBox = S.AddCheckBox(_("Remove DC offset (center on 0.0 vertically)"),
                                        mDC ? wxT("true") : wxT("false"));
            mDCCheckBox->SetValidator(wxGenericValidator(&mDC));

            S.StartHorizontalLay(wxALIGN_CENTER, false);
            {
               mGainCheckBox = S.AddCheckBox(_("Normalize maximum amplitude to"),
                                             mGain ? wxT("true") : wxT("false"));
               mGainCheckBox->SetValidator(wxGenericValidator(&mGain));

               FloatingPointValidator<double> vldLevel(2, &mLevel, NumValidatorStyle::ONE_TRAILING_ZERO);
               vldLevel.SetRange(MIN_Level, MAX_Level);
               mLevelTextCtrl = S.AddTextBox( {}, wxT(""), 10);
               mLevelTextCtrl->SetName(_("Maximum amplitude dB"));
               mLevelTextCtrl->SetValidator(vldLevel);
               mLeveldB = S.AddVariableText(_("dB"), false,
                                            wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
               mWarning = S.AddVariableText( {}, false,
                                            wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
            }
            S.EndHorizontalLay();

         
            mStereoIndCheckBox = S.AddCheckBox(_("Normalize stereo channels independently"),
                                               mStereoInd ? wxT("true") : wxT("false"));
            mStereoIndCheckBox->SetValidator(wxGenericValidator(&mStereoInd));
         }
         S.EndVerticalLay();
      }
      S.EndMultiColumn();
   }
   S.EndVerticalLay();

   mCreating = false;
}

bool EffectNormalize::TransferDataToWindow()
{
   if (!mUIParent->TransferDataToWindow())
   {
      return false;
   }

   UpdateUI();

   return true;
}

bool EffectNormalize::TransferDataFromWindow()
{
   if (!mUIParent->Validate() || !mUIParent->TransferDataFromWindow())
   {
      return false;
   }

   return true;
}

// EffectNormalize implementation

bool EffectNormalize::AnalyseTrack(const WaveTrack * track, const wxString &msg,
                                   int curTrackNum,
                                   float &offset, float &min, float &max)
{
   if(mGain) {
      // Since we need complete summary data, we need to block until the OD tasks are done for this track
      // TODO: should we restrict the flags to just the relevant block files (for selections)
      while (track->GetODFlags()) {
         // update the gui
         if (ProgressResult::Cancelled == mProgress->Update(
            0, _("Waiting for waveform to finish computing...")) )
            return false;
         wxMilliSleep(100);
      }

      // set mMin, mMax.  No progress bar here as it's fast.
      auto pair = track->GetMinMax(mCurT0, mCurT1); // may throw
      min = pair.first, max = pair.second;

   } else {
      min = -1.0, max = 1.0;   // sensible defaults?
   }

   if(mDC) {
      auto rc = AnalyseDC(track, msg, curTrackNum, offset);
      min += offset;
      max += offset;
      return rc;
   } else {
      offset = 0.0;
      return true;
   }
}

//AnalyseDC() takes a track, transforms it to bunch of buffer-blocks,
//and executes AnalyzeData on it...
bool EffectNormalize::AnalyseDC(const WaveTrack * track, const wxString &msg,
                                int curTrackNum,
                                float &offset)
{
   bool rc = true;

   offset = 0.0; // we might just return

   if(!mDC)  // don't do analysis if not doing dc removal
      return(rc);

   //Transform the marker timepoints to samples
   auto start = track->TimeToLongSamples(mCurT0);
   auto end = track->TimeToLongSamples(mCurT1);

   //Get the length of the buffer (as double). len is
   //used simply to calculate a progress meter, so it is easier
   //to make it a double now than it is to do it later
   auto len = (end - start).as_double();

   //Initiate a processing buffer.  This buffer will (most likely)
   //be shorter than the length of the track being processed.
   Floats buffer{ track->GetMaxBlockSize() };

   mSum = 0.0; // dc offset inits
   mCount = 0;

   sampleCount blockSamples;
   sampleCount totalSamples = 0;

   //Go through the track one buffer at a time. s counts which
   //sample the current buffer starts at.
   auto s = start;
   while (s < end) {
      //Get a block of samples (smaller than the size of the buffer)
      //Adjust the block size if it is the final block in the track
      const auto block = limitSampleBufferSize(
         track->GetBestBlockSize(s),
         end - s
      );

      //Get the samples from the track and put them in the buffer
      track->Get((samplePtr) buffer.get(), floatSample, s, block, fillZero, true, &blockSamples);
      totalSamples += blockSamples;

      //Process the buffer.
      AnalyzeData(buffer.get(), block);

      //Increment s one blockfull of samples
      s += block;

      //Update the Progress meter
      if (TrackProgress(curTrackNum,
                        ((s - start).as_double() / len)/2.0, msg)) {
         rc = false; //lda .. break, not return, so that buffer is deleted
         break;
      }
   }
   if( totalSamples > 0 )
      offset = -mSum / totalSamples.as_double();  // calculate actual offset (amount that needs to be added on)
   else
      offset = 0.0;

   //Return true because the effect processing succeeded ... unless cancelled
   return rc;
}

//ProcessOne() takes a track, transforms it to bunch of buffer-blocks,
//and executes ProcessData, on it...
// uses mMult and offset to normalize a track.
// mMult must be set before this is called
bool EffectNormalize::ProcessOne(
   WaveTrack * track, const wxString &msg, int curTrackNum, float offset)
{
   bool rc = true;

   //Transform the marker timepoints to samples
   auto start = track->TimeToLongSamples(mCurT0);
   auto end = track->TimeToLongSamples(mCurT1);

   //Get the length of the buffer (as double). len is
   //used simply to calculate a progress meter, so it is easier
   //to make it a double now than it is to do it later
   auto len = (end - start).as_double();

   //Initiate a processing buffer.  This buffer will (most likely)
   //be shorter than the length of the track being processed.
   Floats buffer{ track->GetMaxBlockSize() };

   //Go through the track one buffer at a time. s counts which
   //sample the current buffer starts at.
   auto s = start;
   while (s < end) {
      //Get a block of samples (smaller than the size of the buffer)
      //Adjust the block size if it is the final block in the track
      const auto block = limitSampleBufferSize(
         track->GetBestBlockSize(s),
         end - s
      );

      //Get the samples from the track and put them in the buffer
      track->Get((samplePtr) buffer.get(), floatSample, s, block);

      //Process the buffer.
      ProcessData(buffer.get(), block, offset);

      //Copy the newly-changed samples back onto the track.
      track->Set((samplePtr) buffer.get(), floatSample, s, block);

      //Increment s one blockfull of samples
      s += block;

      //Update the Progress meter
      if (TrackProgress(curTrackNum,
                        0.5+((s - start).as_double() / len)/2.0, msg)) {
         rc = false; //lda .. break, not return, so that buffer is deleted
         break;
      }
   }

   //Return true because the effect processing succeeded ... unless cancelled
   return rc;
}

void EffectNormalize::AnalyzeData(float *buffer, size_t len)
{
   for(decltype(len) i = 0; i < len; i++)
      mSum += (double)buffer[i];
   mCount += len;
}

void EffectNormalize::ProcessData(float *buffer, size_t len, float offset)
{
   for(decltype(len) i = 0; i < len; i++) {
      float adjFrame = (buffer[i] + offset) * mMult;
      buffer[i] = adjFrame;
   }
}

void EffectNormalize::OnUpdateUI(wxCommandEvent & WXUNUSED(evt))
{
   UpdateUI();
}

void EffectNormalize::UpdateUI()
{
   if (!mUIParent->TransferDataFromWindow())
   {
      mWarning->SetLabel(_(".  Maximum 0dB."));
      EnableApply(false);
      return;
   }
   mWarning->SetLabel(wxT(""));

   // Disallow level stuff if not normalizing
   mLevelTextCtrl->Enable(mGain);
   mLeveldB->Enable(mGain);
   mStereoIndCheckBox->Enable(mGain);

   // Disallow OK/Preview if doing nothing
   EnableApply(mGain || mDC);
}

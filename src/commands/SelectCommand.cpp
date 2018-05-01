/**********************************************************************

   Audacity - A Digital Audio Editor
   Copyright 1999-2009 Audacity Team
   License: wxwidgets

   Dan Horgan
   James Crook

******************************************************************//**

\file SelectCommand.cpp
\brief Definitions for SelectCommand classes

\class SelectTimeCommand
\brief Command for changing the time selection

\class SelectFrequenciesCommand
\brief Command for changing the frequency selection

\class SelectTracksCommand
\brief Command for changing the selection of tracks

\class SelectCommand
\brief Command for changing time, frequency and track selection. This
class is a little baroque, as it uses the SelectTimeCommand, 
SelectFrequenciesCommand and SelectTracksCommand, when it could just
explicitly code all three.

*//*******************************************************************/

#include "../Audacity.h"
#include <wx/string.h>
#include <float.h>

#include "SelectCommand.h"
#include "../Project.h"
#include "../Track.h"
#include "../TrackPanel.h"
#include "../ShuttleGui.h"
#include "CommandContext.h"


// Relative to project and relative to selection cover MOST options, since you can already
// set a selection to a clip.
const int nRelativeTos =6;
static const IdentInterfaceSymbol kRelativeTo[nRelativeTos] =
{
   { wxT("ProjectStart"), XO("Project Start") },
   { XO("Project") },
   { wxT("ProjectEnd"), XO("Project End") },
   { wxT("SelectionStart"), XO("Selection Start") },
   { XO("Selection") },
   { wxT("SelectionEnd"), XO("Selection End") }
};

bool SelectTimeCommand::DefineParams( ShuttleParams & S ){
   // Allow selection down to -ve 100seconds.
   // Typically used to expand/contract selections by a small amount.
   S.OptionalY( bHasT0           ).Define( mT0, wxT("Start"), 0.0, -100.0, (double)FLT_MAX);
   S.OptionalY( bHasT1           ).Define( mT1, wxT("End"), 0.0, -100.0, (double)FLT_MAX);
   S.OptionalN( bHasRelativeSpec ).DefineEnum( mRelativeTo,   wxT("RelativeTo"), 0, kRelativeTo, nRelativeTos );
   return true;
}

void SelectTimeCommand::PopulateOrExchange(ShuttleGui & S)
{
   auto relativeSpec = LocalizedStrings( kRelativeTo, nRelativeTos );
   S.AddSpace(0, 5);

   S.StartMultiColumn(3, wxEXPAND);
   {
      S.SetStretchyCol( 2 );
      S.Optional( bHasT0 ).TieTextBox(_("Start Time:"), mT0);
      S.Optional( bHasT1 ).TieTextBox(_("End Time:"),   mT1);
      // Chooses what time is relative to.
      S.Optional( bHasRelativeSpec ).TieChoice( 
         _("Relative To:"), mRelativeTo, &relativeSpec);
   }
   S.EndMultiColumn();
}

bool SelectTimeCommand::Apply(const CommandContext & context){
   // Many commands need focus on track panel.
   // No harm in setting it with a scripted select.
   context.GetProject()->GetTrackPanel()->SetFocus();
   if( !bHasT0 && !bHasT1 )
      return true;

   // Defaults if no value...
   if( !bHasT0 )
      mT0 = 0.0;
   if( !bHasT1 )
      mT1 = 0.0;
   if( !bHasRelativeSpec )
      mRelativeTo = 0;

   AudacityProject * p = context.GetProject();
   double end = p->GetTracks()->GetEndTime();
   double t0;
   double t1;

   switch( bHasRelativeSpec ? mRelativeTo : 0 ){
   default:
   case 0: //project start
      t0 = mT0;
      t1 = mT1;
      break;
   case 1: //project
      t0 = mT0;
      t1 = end + mT1;
      break;
   case 2: //project end;
      t0 = end - mT0;
      t1 = end - mT1;
      break;
   case 3: //selection start
      t0 = mT0 + p->GetSel0();
      t1 = mT1 + p->GetSel0();
      break;
   case 4: //selection
      t0 = mT0 + p->GetSel0();
      t1 = mT1 + p->GetSel1();
      break;
   case 5: //selection end
      t0 =  p->GetSel1() - mT0;
      t1 =  p->GetSel1() - mT1;
      break;
   }

   p->mViewInfo.selectedRegion.setTimes( t0, t1);
   return true;
}

bool SelectFrequenciesCommand::DefineParams( ShuttleParams & S ){
   S.OptionalN( bHasTop ).Define(    mTop,    wxT("High"), 0.0, 0.0, (double)FLT_MAX);
   S.OptionalN( bHasBottom ).Define( mBottom, wxT("Low"),  0.0, 0.0, (double)FLT_MAX);
   return true;
}

void SelectFrequenciesCommand::PopulateOrExchange(ShuttleGui & S)
{
   S.AddSpace(0, 5);

   S.StartMultiColumn(3, wxEXPAND);
   {
      S.SetStretchyCol( 2 );
      S.Optional( bHasTop    ).TieTextBox(_("High:"), mTop);
      S.Optional( bHasBottom ).TieTextBox(_("Low:"),  mBottom);
   }
   S.EndMultiColumn();
}

bool SelectFrequenciesCommand::Apply(const CommandContext & context){
   if( !bHasBottom && !bHasTop )
      return true;

   // Defaults if no value...
   if( !bHasTop )
      mTop = 0.0;
   if( !bHasBottom )
      mBottom = 0.0;

   context.GetProject()->SSBL_ModifySpectralSelection(
      mBottom, mTop, false);// false for not done.
   return true;
}

const int nModes =3;
static const IdentInterfaceSymbol kModes[nModes] =
{
   // These are acceptable dual purpose internal/visible names

   /* i18n-hint verb, imperative */
   { XO("Set") },
   { XO("Add") },
   { XO("Remove") },
};

bool SelectTracksCommand::DefineParams( ShuttleParams & S ){
   S.OptionalN( bHasFirstTrack).Define( mFirstTrack, wxT("Track"), 0.0, 0.0, 100.0);
   S.OptionalN( bHasNumTracks ).Define( mNumTracks,  wxT("TrackCount"),  1.0, 0.0, 100.0);
   S.OptionalY( bHasMode      ).DefineEnum( mMode,   wxT("Mode"), 0, kModes, nModes );
   
   return true;
}

void SelectTracksCommand::PopulateOrExchange(ShuttleGui & S)
{
   auto modes = LocalizedStrings( kModes, nModes );
   S.AddSpace(0, 5);

   S.StartMultiColumn(3, wxEXPAND);
   {
      S.SetStretchyCol( 2 );
      S.Optional( bHasFirstTrack).TieTextBox(_("First Track:"),mFirstTrack);
      S.Optional( bHasNumTracks).TieTextBox(_("Track Count:"),mNumTracks);
   }
   S.EndMultiColumn();
   S.StartMultiColumn(2, wxALIGN_CENTER);
   {
      // Always used, so no check box.
      S.TieChoice( _("Mode:"), mMode, &modes);
   }
   S.EndMultiColumn();
}

bool SelectTracksCommand::Apply(const CommandContext &context)
{
   int index = 0;
   TrackList *tracks = context.GetProject()->GetTracks();

   // Defaults if no value...
   if( !bHasNumTracks ) 
      mNumTracks = 1.0;
   if( !bHasFirstTrack ) 
      mFirstTrack = 0.0;

   // Stereo second tracks count as 0.5 of a track.
   double last = mFirstTrack+mNumTracks;
   double first = mFirstTrack;
   bool bIsSecondChannel = false;
   TrackListIterator iter(tracks);
   Track *t = iter.First();
   while (t) {
      // Add 0.01 so we are free of rounding errors in comparisons.
      // Optionally add 0.5 for second track which counts as is half a track
      double track = index + 0.01 + (bIsSecondChannel ? 0.5 : 0.0);
      bool sel = first <= track && track <= last;
      if( mMode == 0 ){ // Set
         t->SetSelected(sel);
      }
      else if( mMode == 1 && sel ){ // Add
         t->SetSelected(sel);
      }
      else if( mMode == 2 && sel ){ // Remove
         t->SetSelected(!sel);
      }
      // Do second channel in stereo track too.
      bIsSecondChannel = t->GetLinked();
      if( !bIsSecondChannel )
         ++index;
      t = iter.Next();
   }
   return true;
}


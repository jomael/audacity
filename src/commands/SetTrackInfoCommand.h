/**********************************************************************

   Audacity - A Digital Audio Editor
   Copyright 1999-2018 Audacity Team
   License: wxwidgets

   Dan Horgan
   James Crook

******************************************************************//**

\file SetTrackCommand.h
\brief Declarations of SetTrackCommand and SetTrackCommandType classes

*//*******************************************************************/

#ifndef __SET_TRACK_COMMAND__
#define __SET_TRACK_COMMAND__

#include "Command.h"
#include "CommandType.h"

#define SET_TRACK_PLUGIN_SYMBOL IdentInterfaceSymbol{ XO("Set Track") }
#define SET_TRACK_STATUS_PLUGIN_SYMBOL IdentInterfaceSymbol{ XO("Set Track Status") }
#define SET_TRACK_AUDIO_PLUGIN_SYMBOL IdentInterfaceSymbol{ XO("Set Track Audio") }
#define SET_TRACK_VISUALS_PLUGIN_SYMBOL IdentInterfaceSymbol{ XO("Set Track Visuals") }

class Track;

class SetTrackBase : public AudacityCommand
{
public:
   SetTrackBase();
   bool Apply(const CommandContext & context) override;
   virtual bool ApplyInner( const CommandContext &context, Track *t  );
   virtual bool DefineParams( ShuttleParams & S ) override;
   virtual void PopulateOrExchange(ShuttleGui & S) override;

   int mTrackIndex;
   int mChannelIndex;
   bool bHasTrackIndex;
   bool bHasChannelIndex;

   bool bIsSecondChannel;
   bool mbPromptForTracks;
};


class SetTrackStatusCommand : public SetTrackBase
{
public:
   //SetTrackStatusCommand();
   // CommandDefinitionInterface overrides
   IdentInterfaceSymbol GetSymbol() override {return SET_TRACK_STATUS_PLUGIN_SYMBOL;};
   wxString GetDescription() override {return _("Sets various values for a track.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;

   // AudacityCommand overrides
   wxString ManualPage() override {return wxT("Extra_Menu:_Tools#set_track_status");};
   bool ApplyInner( const CommandContext & context, Track * t ) override;

public:
   wxString mTrackName;
   bool bSelected;
   bool bFocused;

// For tracking optional parameters.
   bool bHasTrackName;
   bool bHasSelected;
   bool bHasFocused;
};

class SetTrackAudioCommand : public SetTrackBase
{
public:
   //SetTrackAudioCommand();
   // CommandDefinitionInterface overrides
   IdentInterfaceSymbol GetSymbol() override {return SET_TRACK_AUDIO_PLUGIN_SYMBOL;};
   wxString GetDescription() override {return _("Sets various values for a track.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;

   // AudacityCommand overrides
   wxString ManualPage() override {return wxT("Extra_Menu:_Tools#set_track");};
   bool ApplyInner( const CommandContext & context, Track * t ) override;

public:
   double mPan;
   double mGain;
   bool bSolo;
   bool bMute;

// For tracking optional parameters.
   bool bHasPan;
   bool bHasGain;
   bool bHasSolo;
   bool bHasMute;
};

class SetTrackVisualsCommand : public SetTrackBase
{
public:
   //SetTrackVisualsCommand();
   // CommandDefinitionInterface overrides
   IdentInterfaceSymbol GetSymbol() override {return SET_TRACK_VISUALS_PLUGIN_SYMBOL;};
   wxString GetDescription() override {return _("Sets various values for a track.");};
   bool DefineParams( ShuttleParams & S ) override;
   void PopulateOrExchange(ShuttleGui & S) override;

   // AudacityCommand overrides
   wxString ManualPage() override {return wxT("Extra_Menu:_Tools#set_track");};
   bool ApplyInner( const CommandContext & context, Track * t ) override;

public:
   int mColour;
   int mHeight;
   int mDisplayType;
   int mScaleType;
   int mVZoom;

   bool bUseSpecPrefs;
   bool bSpectralSelect;
   bool bGrayScale;

// For tracking optional parameters.
   bool bHasColour;
   bool bHasHeight;
   bool bHasDisplayType;
   bool bHasScaleType;
   bool bHasVZoom;

   bool bHasUseSpecPrefs;
   bool bHasSpectralSelect;
   bool bHasGrayScale;
};

class SetTrackCommand : public SetTrackBase
{
public:
   SetTrackCommand();
   // CommandDefinitionInterface overrides
   IdentInterfaceSymbol GetSymbol() override {return SET_TRACK_PLUGIN_SYMBOL;};
   wxString GetDescription() override {return _("Sets various values for a track.");};
   // AudacityCommand overrides
   wxString ManualPage() override {return wxT("Extra_Menu:_Tools#set_track");};

public:

   bool DefineParams( ShuttleParams & S ) override { 
      return 
         SetTrackBase::DefineParams(S) &&
         mSetStatus.DefineParams(S) &&  
         mSetAudio.DefineParams(S) &&
         mSetVisuals.DefineParams(S);
   };
   void PopulateOrExchange(ShuttleGui & S) override {
      SetTrackBase::PopulateOrExchange( S );
      mSetStatus.PopulateOrExchange(S);
      mSetAudio.PopulateOrExchange(S);
      mSetVisuals.PopulateOrExchange(S);
   };
   bool ApplyInner(const CommandContext & context, Track * t ) override {
      mSetStatus.bIsSecondChannel = bIsSecondChannel;
      mSetAudio.bIsSecondChannel = bIsSecondChannel;
      mSetVisuals.bIsSecondChannel = bIsSecondChannel;
      return 
         mSetStatus.ApplyInner( context, t ) &&  
         mSetAudio.ApplyInner( context, t )&&
         mSetVisuals.ApplyInner( context, t );
   }

private:
   SetTrackStatusCommand mSetStatus;
   SetTrackAudioCommand mSetAudio;
   SetTrackVisualsCommand mSetVisuals;
};



#endif /* End of include guard: __SETTRACKINFOCOMMAND__ */

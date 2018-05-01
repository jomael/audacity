/**********************************************************************

  Audacity: A Digital Audio Editor

  Silence.h

  Dominic Mazzoni

  An effect to add silence.

**********************************************************************/

#ifndef __AUDACITY_EFFECT_SILENCE__
#define __AUDACITY_EFFECT_SILENCE__

#include <wx/string.h>

#include "../widgets/NumericTextCtrl.h"

#include "Generator.h"

#define SILENCE_PLUGIN_SYMBOL IdentInterfaceSymbol{ XO("Silence") }

class EffectSilence final : public Generator
{
public:
   EffectSilence();
   virtual ~EffectSilence();

   // IdentInterface implementation

   IdentInterfaceSymbol GetSymbol() override;
   wxString GetDescription() override;
   wxString ManualPage() override;

   // EffectDefinitionInterface implementation

   EffectType GetType() override;

   // Effect implementation

   void PopulateOrExchange(ShuttleGui & S) override;
   bool TransferDataToWindow() override;
   bool TransferDataFromWindow() override;

protected:
   // Generator implementation

   bool GenerateTrack(WaveTrack *tmp, const WaveTrack &track, int ntrack) override;

private:
   NumericTextCtrl *mDurationT;
};

#endif

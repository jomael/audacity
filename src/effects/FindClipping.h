/**********************************************************************

  Audacity: A Digital Audio Editor

  FindClipping.h

  Dominic Mazzoni
  Vaughan Johnson (dialog)

**********************************************************************/

#ifndef __AUDACITY_EFFECT_FINDCLIPPING__
#define __AUDACITY_EFFECT_FINDCLIPPING__

class wxString;

class LabelTrack;

#include <wx/string.h>

#include "Effect.h"

#define FINDCLIPPING_PLUGIN_SYMBOL IdentInterfaceSymbol{ XO("Find Clipping") }

class EffectFindClipping final : public Effect
{
public:
   EffectFindClipping();
   virtual ~EffectFindClipping();

   // IdentInterface implementation

   IdentInterfaceSymbol GetSymbol() override;
   wxString GetDescription() override;
   wxString ManualPage() override;

   // EffectDefinitionInterface implementation

   EffectType GetType() override;

   // EffectClientInterface implementation

   bool DefineParams( ShuttleParams & S ) override;
   bool GetAutomationParameters(CommandParameters & parms) override;
   bool SetAutomationParameters(CommandParameters & parms) override;

   // Effect implementation

   bool Process() override;
   void PopulateOrExchange(ShuttleGui & S) override;
   bool TransferDataToWindow() override;
   bool TransferDataFromWindow() override;

private:
   // EffectFindCliping implementation

   bool ProcessOne(LabelTrack *lt, int count, const WaveTrack * wt,
                   sampleCount start, sampleCount len);

private:
   int mStart;   ///< Using int rather than sampleCount because values are only ever small numbers
   int mStop;    ///< Using int rather than sampleCount because values are only ever small numbers
};

#endif // __AUDACITY_EFFECT_FINDCLIPPING__

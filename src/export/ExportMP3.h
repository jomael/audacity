/**********************************************************************

  Audacity: A Digital Audio Editor

  ExportMP3.h

  Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_EXPORTMP3__
#define __AUDACITY_EXPORTMP3__

/* --------------------------------------------------------------------------*/

#include "../MemoryX.h"

enum MP3RateMode : unsigned {
   MODE_SET = 0,
   MODE_VBR,
   MODE_ABR,
   MODE_CBR,
};

template< typename Enum > class EnumSetting;
extern EnumSetting< MP3RateMode > MP3RateModeSetting;

#if defined(__WXMSW__) || defined(__WXMAC__)
#define MP3_EXPORT_BUILT_IN 1
#endif

class wxString;
class wxWindow;

//----------------------------------------------------------------------------
// Get MP3 library version
//----------------------------------------------------------------------------
wxString GetMP3Version(wxWindow *parent, bool prompt);

#endif


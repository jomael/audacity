/**********************************************************************

  Audacity: A Digital Audio Editor

  WindowAccessible

  David Bailes

**********************************************************************/

#ifndef __AUDACITY_WINDOW_ACCESSIBLE__
#define __AUDACITY_WINDOW_ACCESSIBLE__

#include <wx/setup.h>

#if wxUSE_ACCESSIBILITY

#include <wx/access.h>

class WindowAccessible: public wxAccessible
{
public:
   WindowAccessible(wxWindow* win);
   virtual ~WindowAccessible() {}

   wxAccStatus GetName(int childId, wxString* name) override;

};

#endif      // wxUSE_ACCESSIBILITY
#endif      // __AUDACITY_WINDOW_ACCESSIBLE__

/**********************************************************************

Audacity: A Digital Audio Editor

UIHandle.cpp

Paul Licameli

**********************************************************************/

#include "Audacity.h"
#include "UIHandle.h"

UIHandle::~UIHandle()
{
}

void UIHandle::Enter(bool)
{
}

bool UIHandle::HasRotation() const
{
   return false;
}

bool UIHandle::Rotate(bool)
{
   return false;
}

void UIHandle::DrawExtras
   (DrawingPass, wxDC *, const wxRegion &, const wxRect &)
{
}

bool UIHandle::StopsOnKeystroke()
{
   return false;
}

void UIHandle::OnProjectChange(AudacityProject *)
{
}

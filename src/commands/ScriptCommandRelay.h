/**********************************************************************

   Audacity - A Digital Audio Editor
   Copyright 1999-2018 Audacity Team
   File License: wxWidgets

   Dan Horgan

******************************************************************//**

\file ScriptCommandRelay.h
\brief Contains declarations for ScriptCommandRelay

*//*******************************************************************/

#ifndef __SCRIPT_COMMAND_RELAY__
#define __SCRIPT_COMMAND_RELAY__

#include "../Audacity.h"

#include "../MemoryX.h"

class wxString;

typedef int(*tpExecScriptServerFunc)(wxString * pIn, wxString * pOut);
typedef int(*tpRegScriptServerFunc)(tpExecScriptServerFunc pFn);

class ScriptCommandRelay
{
public:
   static void StartScriptServer(tpRegScriptServerFunc scriptFn);
};

#endif /* End of include guard: __SCRIPT_COMMAND_RELAY__ */

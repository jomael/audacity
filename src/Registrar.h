/**********************************************************************

  Audacity: A Digital Audio Editor

  Registrar.h

  James Crook

*******************************************************************//**

\class Registrar
\brief Base class for registration callback.
Audcaity will call providers RegisterNameOfThing() functions with 
an &Registrar as the argument.  RegisterNameOfThing() is then 
responsible for calling the appropriate callback functions.

**********************************************************************/


#ifndef __AUDACITY_REGISTRAR__
#define __AUDACITY_REGISTRAR__

#include "Audacity.h"
#include "MemoryX.h"

class Command;
class LoadableModule;
class CommandDefinitionInterface;
class Effect;

class AUDACITY_DLL_API Registrar 
{
public:
   Registrar(){
      bWantsModules = false;
      bWantsCommands= false;
      bWantsCommandTypes= false;
      bWantsEffects= false;
   }
   bool bWantsModules;
   bool bWantsCommands;
   bool bWantsCommandTypes;
   bool bWantsEffects;
   virtual void AddCommandType(movable_ptr<CommandDefinitionInterface> && WXUNUSED(comDef) ){;};
   virtual void AddCommand(movable_ptr<AudacityCommand> && WXUNUSED(command) ){;};
   virtual void AddModule(movable_ptr<LoadableModule> && WXUNUSED(module) ){;};
   virtual void AddEffect(movable_ptr<Effect> && WXUNUSED(effect) ){;};
};

#endif

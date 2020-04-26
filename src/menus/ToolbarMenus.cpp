#include "../Audacity.h"

#include "../Menus.h"
#include "../ProjectSettings.h"
#include "../commands/CommandContext.h"
#include "../commands/CommandManager.h"
#include "../toolbars/ToolManager.h"

/// Namespace for functions for View Toolbar menu
namespace ToolbarActions {

// exported helper functions
// none

// Menu handler functions

struct Handler : CommandHandlerObject {

void OnResetToolBars(const CommandContext &context)
{
   auto &project = context.project;
   auto &toolManager = ToolManager::Get( project );

   toolManager.Reset();
   MenuManager::Get(project).ModifyToolbarMenus(project);
}

}; // struct Handler

} // namespace

static CommandHandlerObject &findCommandHandler(AudacityProject &) {
   // Handler is not stateful.  Doesn't need a factory registered with
   // AudacityProject.
   static ToolbarActions::Handler instance;
   return instance;
};

// Menu definitions

#define FN(X) (& ToolbarActions::Handler :: X)

namespace{
using namespace MenuTable;

auto ToolbarCheckFn( int toolbarId ) -> CommandListEntry::CheckFn
{
   return [toolbarId](AudacityProject &project){
      auto &toolManager = ToolManager::Get( project );
      return toolManager.IsVisible(toolbarId);
   };
}

BaseItemSharedPtr ToolbarsMenu()
{
   using Options = CommandManager::Options;

   static BaseItemSharedPtr menu{
   ( FinderScope{ findCommandHandler },
   Section( wxT("Toolbars"),
      Menu( wxT("Toolbars"), XO("&Toolbars"),
         Section( "Reset",
            /* i18n-hint: (verb)*/
            Command( wxT("ResetToolbars"), XXO("Reset Toolb&ars"),
               FN(OnResetToolBars), AlwaysEnabledFlag )
         ),

         Section( "Other" )
      )
   ) ) };
   return menu;
}

AttachedItem sAttachment1{
   Placement{ wxT("View/Other"), { OrderingHint::Begin } },
   Shared( ToolbarsMenu() )
};
}

#undef FN

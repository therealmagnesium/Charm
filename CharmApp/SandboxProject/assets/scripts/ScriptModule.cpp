#include "TopDownController.h"
#include "CameraController.h"

extern "C"
{
    void RegisterScripts()
    {
        ScriptManager::BindScript<TopDownController>("Top Down Controller");
        ScriptManager::BindScript<CameraController>("Camera Controller");
    }
}

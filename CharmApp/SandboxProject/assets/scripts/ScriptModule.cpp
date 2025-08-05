#include "TopDownController.h"
#include "CameraController.h"
#include "Bird.h"

extern "C"
{
    void RegisterScripts()
    {
        ScriptManager::BindScript<TopDownController>("Top Down Controller");
        ScriptManager::BindScript<CameraController>("Camera Controller");
        ScriptManager::BindScript<Bird>("Bird");
    }
}

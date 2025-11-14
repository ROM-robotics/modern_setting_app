include("/home/mr_robot/Desktop/Git/modern_setting_app/build/Android_Qt_6_8_3_Clang_arm64_v8a-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modern_setting_app-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")
_qt_internal_show_skip_runtime_deploy_message("shared Qt libs, cross-compiled, non-bundle app"
    EXTRA_MESSAGE "Executable targets have to be app bundles to use this command on Apple platforms."
)
include("/home/mr_robot/Desktop/Git/modern_setting_app/build/Desktop_Qt_6_8_3-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/modern_setting_app-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE /home/mr_robot/Desktop/Git/modern_setting_app/build/Desktop_Qt_6_8_3-Debug/modern_setting_app
    GENERATE_QT_CONF
)
